#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>
#include <SDL2/SDL.h>

#define MAX_SIZE 150

struct Point {
    int x;
    int y;
};

struct Door {
    int exist;
    struct Point pos;
    int is_opened;
    int is_old;
    int has_password;
    char password[MAX_SIZE];
};

struct Trap {
    int exist;
    int damage;
};

struct Gold {
    char type; // n: normal, b: balck
    int cnt; // value
};

struct Room {
    struct Point p;
    int height, width;
    char type; // r: regular, e: enchanted, t: treasure, n: nightmare
};

struct Bag {
    int number_of_food;
    char food[5];
    int number_of_weapon;
    char weapon[5];
    // potions
    int health_potion;
    int speed_potion;
    int damage_potion;
};


struct User {
    char username[MAX_SIZE];
    char password[MAX_SIZE];
    char email[MAX_SIZE];
    int number_of_games;
    time_t first_game;
    int number_of_floor;
    int level;
    char **map[5];
    int mask[5][MAX_SIZE][MAX_SIZE];
    char theme[5][MAX_SIZE][MAX_SIZE]; // r: regular, t: treasure, e: enchant, n: nightmare
    struct Point pos;
    struct Bag bag;
    int score;
    int golds;
    int health;
    int hunger;
    struct Gold gold[5][MAX_SIZE][MAX_SIZE];
    struct Door door[5][MAX_SIZE][MAX_SIZE];
    struct Trap trap[5][MAX_SIZE][MAX_SIZE];
};

struct miniUser {
    char username[MAX_SIZE];
    int number_of_games;
    time_t first_game;
    int gold;
    int score;
};

time_t LAST_REFRESH[3], LAST_EAT;
int USERS;
int DIFFICULTY = 0; // 0: Easy, 1: Medium, 2: HARD
int GAME_X = 30, GAME_Y = 120;
int ST_X = 3, ST_Y = 4;
// up, right, down, left, topleft, topright, bottomright, bottomleft
int D_X[] = {-1, 0, 1, 0, -1, -1, 1, 1};
int D_Y[] = {0, 1, 0, -1, -1, 1, 1, -1}; 
int number_of_rooms;
struct Point* corners;

struct Point create_point(int x, int y) {
    struct Point p;
    p.x = x;
    p.y = y;
    return p;
}

void change_color(int color) {
    attroff(COLOR_PAIR(1));
    attron(COLOR_PAIR(color));
}

void undo_color(int color) {
    attroff(COLOR_PAIR(color));
    attron(COLOR_PAIR(1));
}

void print_message_with_color(int x, int y, char message[], int color) {
    change_color(color);
    mvprintw(x, y, "%s", message);
    undo_color(color);
}

void print_number_with_color(int x, int y, int n, int color) {
    change_color(color);
    mvprintw(x, y, "%d", n);
    undo_color(color);
}

void print_character_with_color(int x, int y, char c, int color) {
    change_color(color);
    mvprintw(x, y, "%c", c);
    undo_color(color);
}

void init_user(struct User* user, int level) {
    user->map[level] = malloc(sizeof(char*) * GAME_X);
    for (int i = 0; i < GAME_X; i++)
        (user->map)[level][i] = malloc(sizeof(char) * GAME_Y);
    for (int i = 0; i < GAME_X; i++)
        for (int j = 0; j < GAME_Y; j++)
            (user->mask)[level][i][j] = 0;
    // user->golds = 0;
    user->bag.number_of_food = 0;
    user->bag.number_of_weapon = 0;
    user->bag.speed_potion = user->bag.health_potion = user->bag.damage_potion = 0;
    user->health = 10;
    user->hunger = 10;
    for (int i = 0; i < GAME_X; i++)
        for (int j = 0; j < GAME_Y; j++) {
            (user->gold)[level][i][j].type = 'N'; // Not available
            (user->door)[level][i][j].exist = (user->door)[level][i][j].has_password = 0;
            (user->trap)[level][i][j].exist = 0;
            (user->theme)[level][i][j] = '.';
        }
}

struct Point next_point(struct Point p, int dir) {
    p.x += D_X[dir];
    p.y += D_Y[dir];
    return p;
}

void trim(char s[]) {
    s[strlen(s) - 1] = '\0';
}

int get_num(char s[]) {
    int len = strlen(s), res = 0;
    for (int i = 0; i < len; i++)
        res = res * 10 + s[i] - '0';
    return res;
}

char* get_str(int n) {
    char* res = malloc(sizeof(char) * 20);
    int len = 0;
    while (n > 0) {
        res[len++] = '0' + n % 10;
        n /= 10;
    }
    for (int l = 0, r = len - 1; l < r; l++, r--) {
        char tmp = res[l];
        res[l] = res[r];
        res[r] = tmp;
    }
    if (len == 0)
        res[len++] = '0';
    res[len] = '\0';
    return res;
}

int is_in_map(struct Point p) {
    return p.x >= 0 && p.y >= 0 && p.x < GAME_X && p.y < GAME_Y;
}

int is_in_corridor(char ***map, struct Point p) {
    return (*map)[p.x][p.y] == '+' || (*map)[p.x][p.y] == '#';
}

int is_corner(struct Point p) {
    for (int i = 0; i < 4 * number_of_rooms; i++)
        if (p.x == corners[i].x && p.y == corners[i].y)
            return 1;
    return 0;
}

int is_food(struct User* user) {
    return user->map[user->level][user->pos.x][user->pos.y] == 'f';
}

int is_weapon(char c) {
    return c == 'm' || c == 'd' || c == 'M' || c == 'n' || c == 's';
}

int is_potion(char c) {
    return c == 'H' || c == 'S' || c == 'D'; 
}

int is_in_room(char ***map, struct Point p) {
    return (*map)[p.x][p.y] == '.' || (*map)[p.x][p.y] == 'O' || (*map)[p.x][p.y] == '<' || (*map)[p.x][p.y] == '>' || (*map)[p.x][p.y] == '^' || (*map)[p.x][p.y] == 'g' || (*map)[p.x][p.y] == 'f' || is_weapon((*map)[p.x][p.y]) || is_potion((*map)[p.x][p.y]);
}

int is_new_room(struct User* user) {
    for (int dir = 0; dir < 4; dir++) {
        struct Point nxt = next_point(user->pos, dir);
        if (is_in_map(nxt)  && is_in_room(&(user->map[user->level]), nxt))
            return !user->mask[user->level][nxt.x][nxt.y];
    }
    return 0;
}

int not_restricted(struct User* user, char ***map, struct Point p) {
    if (user->door[user->level][p.x][p.y].exist)
        return 1;
    return (*map)[p.x][p.y] != '_' && (*map)[p.x][p.y] != '|' && (*map)[p.x][p.y] != 'O' && (*map)[p.x][p.y] != ' ';
}

int get_dir(int key) {
    if (key == KEY_UP || key == 'j')
        return 0;
    else if (key == KEY_RIGHT || key == 'l')
        return 1;
    else if (key == KEY_DOWN || key == 'k')
        return 2;
    else if (key == KEY_LEFT || key == 'h')
        return 3;
    else if (key == 'y')
        return 4;
    else if (key == 'u')
        return 5;
    else if (key == 'n')
        return 6;
    else if (key == 'b')
        return 7;
    return -1;
}

int check_corners(char ***map) {
    for (int i = 0; i < GAME_X; i++)
        for (int j = 0; j < GAME_Y; j++)
            if ((*map)[i][j] == '+' && is_corner(create_point(i, j)))
                return 0;
    return 1;
}

int vertical_neighbor(char ***map, struct Point p) {
    if (p.x > 0 && (*map)[p.x - 1][p.y] == '.') {
        if (p.x + 2 < GAME_X && (*map)[p.x + 1][p.y] == '+' && (*map)[p.x + 2][p.y] == '.')
            return 1;        
    }
    if (p.x + 1 < GAME_X && (*map)[p.x + 1][p.y] == '.') {
        if (p.x - 2 >= 0 && (*map)[p.x - 1][p.y] == '+' && (*map)[p.x - 2][p.y] == '.')
            return 1;
    }
    return 0;
}

int horizontal_neighbor(char ***map, struct Point p) {
    if (p.y > 0 && (*map)[p.x][p.y - 1] == '.') {
        if (p.y + 2 < GAME_Y && (*map)[p.x][p.y + 1] == '+' && (*map)[p.x][p.y + 2] == '.')
            return 1;
    }
    if (p.y + 1 < GAME_Y && (*map)[p.x][p.y + 1] == '.') {
        if (p.y - 2 >= 0 && (*map)[p.x][p.y - 1] == '+' && (*map)[p.x][p.y - 2] == '.')
            return 1;
    }
    return 0;
}


void sort_miniusers(struct miniUser **user_list) {
    for (int i = 0; i < USERS; i++)
        for (int j = i + 1; j < USERS; j++)
            if (user_list[i]->gold < user_list[j]->gold) {
                struct miniUser* tmp = user_list[i];
                user_list[i] = user_list[j];
                user_list[j] = tmp;
            }
}

char* get_time(time_t t) {
    time_t now;
    time(&now);
    long long dif = (long long) difftime(now, t);
    int d = dif / 86400;
    dif %= 86400;
    int h = dif / 3600;
    dif %= 3600;
    int m = dif / 60;
    int s = dif % 60;
    char* res = malloc(sizeof(char) * MAX_SIZE);
    res[0] = '\0';
    strcat(res, get_str(d));
    strcat(res, " Days, ");
    strcat(res, get_str(h));
    strcat(res, " Hours, ");
    strcat(res, get_str(m));
    strcat(res, " Minutes, ");
    strcat(res, get_str(s));
    strcat(res, " Seconds.");
    return res;
}

void init_time(time_t t) {
    for (int line = 0; line < 3; line++)
        LAST_REFRESH[line] = t;
    LAST_EAT = t;
}

int refresh_message(time_t t, int line) {
    if (difftime(t, LAST_REFRESH[line]) > 2) {
        LAST_REFRESH[line] = t;
        return 1;
    }
    return 0;
}

void reduce_hunger(struct User* user) {
    if (user->hunger > 0)
        --user->hunger;
    else
        --user->health;
}

void check_food(struct User* user, time_t t) {
    if (difftime(t, LAST_EAT) > 5 + (2 - DIFFICULTY)) {
        reduce_hunger(user);
        LAST_EAT = t;
    }
}