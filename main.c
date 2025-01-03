// Name: Rassa Mohammadi
// Student: 403106657

#include "essentials.h"
#include "designer.h"
#include "file_processor.h"

void set_colors();
void create_game_menu();
void create_login_page();
void password_recovery_page();
void create_register_page();
void pregame_menu();
void play_game();
void move_player(int);
void play_trap();
void quit_game();

struct User user;

int main() {
    setlocale(LC_ALL, "");
    initscr();
    keypad(stdscr, TRUE);
    srand(time(NULL));
    set_colors();
    create_game_menu();
    play_game();
    quit_game();
}

void set_colors() {
    start_color();
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_CYAN, COLOR_BLACK);
    attron(COLOR_PAIR(1));
}

void create_game_menu() {
    curs_set(FALSE);
    noecho();
    int x = LINES / 3, y = COLS / 3;
    int width = COLS / 3, height = LINES / 3;
    draw_box("Welcome to Rogue", create_point(x, y), height, width, '+');
    x += 3, y += 1;
    int choice = 0, key = -1;
    char *options[] = {"Login", "Register", "Guest", "Quit"};
    do {
        if (key == KEY_UP && choice > 0)
            choice--;
        if (key == KEY_DOWN && choice < 3)
            choice++;
        for (int i = 0; i < 4; i++) {
            if (i == choice) {
                attron(A_REVERSE | A_BOLD);
                mvprintw(x + i, y + width / 2 - 3, "%s", options[i]);
                attroff(A_REVERSE | A_BOLD);
            }
            else
                mvprintw(x + i, y + width / 2 - 3, "%s", options[i]);
        }
        key = getch();
    } while (key != '\n');
    if (choice == 0) {
        create_login_page();
        pregame_menu();
    }
    else if (choice == 1) {
        create_register_page();
        pregame_menu();
    }
    else if (choice == 2) {
        generate_map(&user);
    }
    else // quit
        quit_game();
}

void password_recovery_page() {
    clear();
    echo();
    int x = LINES / 3, y = COLS / 3;
    char email[MAX_SIZE];
    mvprintw(x, y, "Enter Your Email:");
    while (true) {
        move(x + 1, y);
        getstr(email);
        if (!strcmp(email, user.email)) {
            clean_area(create_point(x, y + 20), create_point(x, COLS - 1));
            break;
        }
        else {
            print_message(create_point(x, y + 20), "Email not correct!");
            clean_area(create_point(x + 1, y), create_point(x + 1, COLS - 1));
        }
    }
    x += 3;
    while (true) {
        mvprintw(x, y, "Enter New Password:");
        print_message(create_point(x + 2, y - 30), "Password must contain at least 7 characters, 1 number character, 1 capital letter and 1 small letter.");
        print_message(create_point(x + 3, y), "To generate password press space button");
        move(x + 1, y);
        noecho();
        int len = 0, key = getch();
        while (key != '\n') {
            if (key == ' ') {
                len = MAX_SIZE;
                do {
                    strcpy(user.password, generate_password());
                } while (!valid_password(user.password));
                print_message(create_point(x + 1, y), user.password);
                refresh();
                usleep(2000000);
                break;
            }
            if (key == KEY_BACKSPACE && len > 0) {
                move(x + 1, y + len - 1);
                printw(" ");
                len--;
                move(x + 1, y + len);
            }
            else if (key != KEY_BACKSPACE) {
                user.password[len++] = key;
                printw("*");
            }
            key = getch();
        }
        user.password[len] = '\0';
        if (valid_password(user.password)) {
            clean_area(create_point(x, y + 20), create_point(x, COLS - 1));
            break;
        }
        else {
            print_message(create_point(x, y + 20), "Invalid Password!");
            clean_area(create_point(x + 1, y), create_point(x + 1, COLS - 1));
        }
    }
    strcpy(user.email, email);
}

void create_login_page() {
    clear();
    curs_set(TRUE);
    int x = LINES / 3, y = COLS / 3;
    while (true) {
        mvprintw(x, y, "Enter Username:");
        move(x + 1, y);
        echo();
        getstr(user.username);
        if (exist_username(user.username)) {
            clean_area(create_point(x, y + 16), create_point(x, COLS - 1));
            break;
        }
        else {
            print_message(create_point(x, y + 16), "Invalid Username!");
            clean_area(create_point(x + 1, y), create_point(x + 1, COLS - 1));
        }
    }
    load_user(&user);
    char password[MAX_SIZE];
    x += 3;
    while (true) {
        mvprintw(x, y, "Enter Password:");
        print_message(create_point(x + 2, y - 15), "It you have forgotten your password press space button");
        move(x + 1, y);
        noecho();
        int len = 0, key = getch();
        int recovery = 0;
        while (key != '\n') {
            if (key == ' ') {
                password_recovery_page();
                recovery = 1;
                break;
            }
            else if (key == KEY_BACKSPACE && len > 0) {
                move(x + 1, y + len - 1);
                printw(" ");
                len--;
                move(x + 1, y + len);
            }
            else if (key != KEY_BACKSPACE) {
                password[len++] = key;
                printw("*");
            }
            key = getch();
        }
        password[len] = '\0';
        if (recovery)
            break;
        if (!strcmp(password, user.password)) {
            clean_area(create_point(x, y + 16), create_point(x, COLS - 1));
            break;
        }
        else {
            print_message(create_point(x, y + 16), "Incorrect Password!");
            clean_area(create_point(x + 1, y), create_point(x + 1, COLS - 1));
        }
    }
}

void create_register_page() {
    clear();
    curs_set(TRUE);
    int x = LINES / 3, y = COLS / 3;
    // get username
    while (true) {
        mvprintw(x, y, "Enter Username:");
        move(x + 1, y);
        echo();
        getstr(user.username);
        if (exist_username(user.username)) {
            print_message(create_point(x, y + 16), "Username is already used!");
            clean_area(create_point(x + 1, y), create_point(x + 1, COLS - 1));
        }
        else {
            clean_area(create_point(x, y + 16), create_point(x, COLS - 1));
            break;
        }
    }
    // get password
    x += 3;
    while (true) {
        mvprintw(x, y, "Enter Password:");
        print_message(create_point(x + 2, y - 30), "Password must contain at least 7 characters, 1 number character, 1 capital letter and 1 small letter.");
        print_message(create_point(x + 3, y), "To generate password press space button");
        move(x + 1, y);
        noecho();
        int len = 0, key = getch();
        while (key != '\n') {
            if (key == ' ') {
                len = MAX_SIZE;
                do {
                    strcpy(user.password, generate_password());
                } while (!valid_password(user.password));
                print_message(create_point(x + 1, y), user.password);
                break;
            }
            if (key == KEY_BACKSPACE && len > 0) {
                move(x + 1, y + len - 1);
                printw(" ");
                len--;
                move(x + 1, y + len);
            }
            else if (key != KEY_BACKSPACE) {
                user.password[len++] = key;
                printw("*");
            }
            key = getch();
        }
        user.password[len] = '\0';
        if (valid_password(user.password)) {
            clean_area(create_point(x, y + 16), create_point(x, COLS - 1));
            break;
        }
        else {
            print_message(create_point(x, y + 16), "Invalid Password!");
            clean_area(create_point(x + 1, y), create_point(x + 1, COLS - 1));
        }
    }
    // get email
    x += 5;
    while (true) {
        mvprintw(x, y, "Enter Email:");
        move(x + 1, y);
        echo();
        getstr(user.email);
        if (valid_email(user.email)) {
            clean_area(create_point(x, y + 16), create_point(x, COLS - 1));
            break;
        }
        else {
            print_message(create_point(x, y + 16), "Invalid Email!");
            clean_area(create_point(x + 1, y), create_point(x + 1, COLS - 1));
        }
    }
    create_user(user.username, user.password, user.email);
}

void pregame_menu() {
    clear();
    curs_set(FALSE);
    noecho();
    char *options[] = {"Load previous game", "Create new game"};
    int choice = 0, key = -1;
    int x = LINES / 4, y = COLS / 4;
    do {
        if (key == KEY_UP && choice > 0)
            choice--;
        if (key == KEY_DOWN && choice < 1)
            choice++;
        for (int i = 0; i < 2; i++) {
            if (choice == i) {
                attron(A_REVERSE | A_BOLD);
                mvprintw(x + i, y, "%s", options[i]);
                attroff(A_REVERSE | A_BOLD);
            }
            else
                mvprintw(x + i, y, "%s", options[i]);
        }
        key = getch();
    } while (key != '\n');
    if (choice == 0) { // load game
        if (!has_map(&user)) {
            clear();
            print_message(create_point(LINES / 3, COLS / 3), "There is no previous name for this account!");
            print_message(create_point(LINES / 3 + 1, COLS / 3), "Press any key to return to the previous menu.");
            getch();
            pregame_menu();
        }
    }
    else { // create new game
        generate_map(&user);
        update_user(&user);
    }
}

void move_player(int key) {
    struct Point nxt;
    if (key == KEY_UP) {
        nxt = next_point(user.pos, 0);
        if (is_in_map(nxt) && not_restricted(&user.map, nxt))
            user.pos.x = nxt.x, user.pos.y = nxt.y;
    }
    else if (key == KEY_RIGHT) {
        nxt = next_point(user.pos, 1);
        if (is_in_map(nxt) && not_restricted(&user.map, nxt))
            user.pos.x = nxt.x, user.pos.y = nxt.y;
    }
    else if (key == KEY_DOWN) {
        nxt = next_point(user.pos, 2);
        if (is_in_map(nxt) && not_restricted(&user.map, nxt))
            user.pos.x = nxt.x, user.pos.y = nxt.y;
    }
    else if (key == KEY_LEFT) {
        nxt = next_point(user.pos, 3);
        if (is_in_map(nxt) && not_restricted(&user.map, nxt))
            user.pos.x = nxt.x, user.pos.y = nxt.y;
    }
}

void appear_map(struct Point p, int depth) {
    if (!depth)
        return;
    if (is_in_room(&user.map, p)) {
        (user.mask)[p.x][p.y] = 1;
        for (int dir = 0; dir < 8; dir++) {
            struct Point nxt = next_point(p, dir);
            if (is_in_room(&user.map, nxt) && !(user.mask)[nxt.x][nxt.y])
                appear_map(nxt, 5);
            else
                (user.mask)[nxt.x][nxt.y] = 1;
        }
    }
    else { // +, #
        (user.mask)[p.x][p.y] = 1;
        for (int dir = 0; dir < 4; dir++) {
            struct Point nxt = next_point(p, dir);
            if (is_in_map(nxt)) {
                if (is_in_corridor(&user.map, nxt))
                    appear_map(nxt, depth - 1);
                if (is_in_room(&user.map, nxt) && depth == 5)
                    appear_map(nxt, 5);
            }
        }
    }
}

void play_trap(struct Point p) {
    clear();
    print_message(create_point(0, COLS / 3), "You fell into a trap!");
    user.health -= user.trap[p.x][p.y].damage;
    user.map[p.x][p.y] = '^';
    print_status(&user);
    print_message(create_point(2, COLS / 3), "Press any key to return ...");
    timeout(-1);
    getch();
    clear();
}

void play_game() {
    clear();
    print_message(create_point(0, 0), "Press (E) to exit game.");
    refresh();
    int key;
    do {
        appear_map(user.pos, 5);
        if (user.trap[user.pos.x][user.pos.y].exist)
            play_trap(create_point(user.pos.x, user.pos.y));
        print_map(&user);
        mvprintw(user.pos.x + ST_X, user.pos.y + ST_Y, "$");
        print_status(&user);
        refresh();
        key = getch();
        move_player(key);
        timeout(0);
    } while (key != 'Q');
    update_user(&user);
    timeout(-1);
}

void quit_game() {
    usleep(500000);
    refresh();
    clear();
    curs_set(FALSE);
    int x = LINES / 3, y = COLS / 3;
    print_message(create_point(x, y), "Game over!");
    mvprintw(x + 1, y, "Press any key to exit ...");
    refresh();
    getch();
    endwin();
    exit(0);
}
