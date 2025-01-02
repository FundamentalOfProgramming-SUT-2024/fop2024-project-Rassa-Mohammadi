#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>

#define MAX_SIZE 150

struct Point {
    int x;
    int y;
};

struct User {
    char username[MAX_SIZE];
    char password[MAX_SIZE];
    char email[MAX_SIZE];
    int score;
    int gold;
    char **map;
    int mask[MAX_SIZE][MAX_SIZE];
    struct Point pos;
};

int GAME_X, GAME_Y;
int ST_X = 2, ST_Y = 4;
int D_X[] = {-1, 0, 1, 0}, D_Y[] = {0, 1, 0, -1}; // up, right, down, left
int number_of_rooms;
struct Point* corners;

struct Point create_point(int x, int y) {
    struct Point p;
    p.x = x;
    p.y = y;
    return p;
}

void print_message(struct Point st, char s[]) {
    move(st.x, st.y);
    attroff(COLOR_PAIR(1));
    attron(COLOR_PAIR(2));
    printw("%s", s);
    attroff(COLOR_PAIR(2));
    attron(COLOR_PAIR(1));
}

void init_user(struct User* user, int height, int width) {
    GAME_X = height;
    GAME_Y = width;
    user->map = malloc(sizeof(char *) * GAME_X);
    for (int i = 0; i < GAME_Y; i++)
        user->map[i] = malloc(sizeof(char) * GAME_Y);
    for (int i = 0; i < GAME_X; i++)
        for (int j = 0; j < GAME_Y; j++)
            (user->mask)[i][j] = 0;
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

int is_in_map(struct Point p) {
    return p.x >= 0 && p.y >= 0 && p.x < GAME_X && p.y < GAME_Y;
}

int is_corner(struct Point p) {
    for (int i = 0; i < 4 * number_of_rooms; i++)
        if (p.x == corners[i].x && p.y == corners[i].y)
            return 1;
    return 0;
}

int not_restricted(char ***map, struct Point p) {
    return (*map)[p.x][p.y] != '_' && (*map)[p.x][p.y] != '|' && (*map)[p.x][p.y] != 'O';
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