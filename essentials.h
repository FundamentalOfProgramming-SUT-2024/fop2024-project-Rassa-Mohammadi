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
    int hidden;
    int is_opened;
    int is_old;
    int has_password;
    char password[MAX_SIZE];
};

struct Trap {
    int exist;
    int damage;
};

struct Room {
    struct Point p;
    int height, width;
    char type;
};

struct User {
    char username[MAX_SIZE];
    char password[MAX_SIZE];
    char email[MAX_SIZE];
    int number_of_games;
    time_t first_game;
    int level;
    char **map[4];
    int mask[4][MAX_SIZE][MAX_SIZE];
    char theme[4][MAX_SIZE][MAX_SIZE]; // r: regular, t: treasure, e: enchant
    struct Point pos;
    int gold;
    int health;
    struct Door door[4][MAX_SIZE][MAX_SIZE];
    struct Trap trap[4][MAX_SIZE][MAX_SIZE];
};

struct miniUser {
    char username[MAX_SIZE];
    int number_of_games;
    time_t first_game;
    int gold;
};

int USERS;
int GAME_X = 30, GAME_Y = 120;
int ST_X = 2, ST_Y = 4;
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

void print_message_with_color(int x, int y, char message[], int color) {
    attroff(COLOR_PAIR(1));
    attron(COLOR_PAIR(color));
    mvprintw(x, y, "%s", message);
    attroff(COLOR_PAIR(color));
    attron(COLOR_PAIR(1));
}

void print_number_with_color(int x, int y, int n, int color) {
    attroff(COLOR_PAIR(1));
    attron(COLOR_PAIR(color));
    mvprintw(x, y, "%d", n);
    attroff(COLOR_PAIR(color));
    attron(COLOR_PAIR(1));
}

void init_user(struct User* user, int level) {
    user->map[level] = malloc(sizeof(char*) * GAME_X);
    for (int i = 0; i < GAME_X; i++)
        (user->map)[level][i] = malloc(sizeof(char) * GAME_Y);
    for (int i = 0; i < GAME_X; i++)
        for (int j = 0; j < GAME_Y; j++)
            (user->mask)[level][i][j] = 0;
    // user->gold = 0;
    user->health = 10;
    for (int i = 0; i < GAME_X; i++)
        for (int j = 0; j < GAME_Y; j++) {
            (user->door)[level][i][j].exist = (user->door)[level][i][j].has_password = 0;
            (user->trap)[level][i][j].exist = 0;
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

int is_in_room(char ***map, struct Point p) {
    return (*map)[p.x][p.y] == '.' || (*map)[p.x][p.y] == 'O' || (*map)[p.x][p.y] == '<' || (*map)[p.x][p.y] == '>' || (*map)[p.x][p.y] == '^' || (*map)[p.x][p.y] == 'g';
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

int not_restricted(char ***map, struct Point p) {
    return (*map)[p.x][p.y] != '_' && (*map)[p.x][p.y] != '|' && (*map)[p.x][p.y] != 'O' && (*map)[p.x][p.y] != ' ';
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