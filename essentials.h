#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SIZE 100

int GAME_X, GAME_Y;

struct Point {
    int x;
    int y;
};

struct User {
    char username[MAX_SIZE];
    char password[MAX_SIZE];
    char email[MAX_SIZE];
    char **map;
    int score;
    int gold;
};

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
}