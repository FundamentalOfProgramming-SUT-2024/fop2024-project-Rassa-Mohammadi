// Name: Rassa Mohammadi
// Student: 403106657

#include "essentials.h"
#include "designer.h"
#include "file_processor.h"

void set_colors();
void create_game_menu();
void create_login_page();
void create_register_page();
void pregame_menu();
void play_game();
void quit_game();

struct User user;

int main() {
    initscr();
    init_user(&user, 34, 125);
    keypad(stdscr, TRUE);
    srand(time(NULL));
    set_colors();
    create_game_menu();
    pregame_menu();
    quit_game();
}

void set_colors() {
    start_color();
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
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
    if (choice == 0)
        create_login_page();
    else if (choice == 1)
        create_register_page();
    /*else if (choice == 2)
        play as guest;       
    */
    else // quit
        quit_game();
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
    x += 3;
    while (true) {
        mvprintw(x, y, "Enter Password:");
        print_message(create_point(x + 2, y - 15), "It you have forgotten your password press space button");
        move(x + 1, y);
        noecho();
        int len = 0, key = getch();
        while (key != '\n') {
            if (key == ' ') {
                strcpy(user.password, get_password(user.username));
                print_message(create_point(x + 3, y), "You password is: %s");
                print_message(create_point(x + 3, y + 17), user.password);
                move(x + 1, y + len);
            }
            else if (key == KEY_BACKSPACE && len > 0) {
                move(x + 1, y + len - 1);
                printw(" ");
                len--;
                move(x + 1, y + len);
            }
            else {
                user.password[len++] = key;
                printw("*");
            }
            key = getch();
        }
        user.password[len] = '\0';
        if (correct_password(user.username, user.password))
            break;
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
        print_message(create_point(x + 2, y - 30), "Password must contain at least 7 characters, 1 number character, 1 Capital letter and 1 small letter.");
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
            else {
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
        if (!load_map(user.username, &user.map)) {
            clear();
            print_message(create_point(LINES / 3, COLS / 3), "There is no previous name for this account!");
            print_message(create_point(LINES / 3 + 1, COLS / 3), "Press any key to return to the previous menu.");
            getch();
            pregame_menu();
        }
    }
    else { // create new game
        generate_map(&user.map);
        update_user(user);
    }
    play_game();
}

void play_game() {
    clear();
    print_map(&user.map);
    getch();
}

void quit_game() {
    usleep(500000);
    clear();
    curs_set(FALSE);
    int x = LINES / 3, y = COLS / 3;
    mvprintw(x, y, "Game over!");
    mvprintw(x + 1, y, "Press any key to exit ...");
    getch();
    endwin();
    exit(0);
}
