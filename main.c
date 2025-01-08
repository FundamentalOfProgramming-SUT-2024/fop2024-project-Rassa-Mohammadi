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
void go_to_settings();
void difficulty_menu();
void hero_color_menu();
void music_menu();
void play_game();
void move_player(int, char***);
void play_trap(struct Point);
void appear_map(struct Point, int);
void appear_nightmare(struct Point, int);
void disappear_nightmare(struct Point, int);
void hunger_menu();
void consume_food(int);
int check_health();
void quit_game();

struct User user;
int is_guest, reveal;
int hero_color = 1;

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
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_CYAN, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_WHITE, COLOR_BLACK);
    init_pair(7, COLOR_YELLOW, COLOR_BLACK);
    int gold_color = 8;
    init_color(gold_color, 1000, 840, 0);
    init_pair(8, gold_color, COLOR_BLACK);
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
        is_guest = 1;
        pregame_menu();
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
            print_message_with_color(x, y + 20, "Email not correct!", 2);
            clean_area(create_point(x + 1, y), create_point(x + 1, COLS - 1));
        }
    }
    x += 3;
    while (true) {
        mvprintw(x, y, "Enter New Password:");
        print_message_with_color(x + 2, y - 30, "Password must contain at least 7 characters, 1 number character, 1 capital letter and 1 small letter.", 2);
        print_message_with_color(x + 3, y, "To generate password press space button", 2);
        move(x + 1, y);
        noecho();
        int len = 0, key = getch();
        while (key != '\n') {
            if (key == ' ') {
                len = MAX_SIZE;
                do {
                    strcpy(user.password, generate_password());
                } while (!valid_password(user.password));
                print_message_with_color(x + 1, y, user.password, 2);
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
            print_message_with_color(x, y + 20, "Invalid Password!", 2);
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
            print_message_with_color(x, y + 16, "Invalid Username!", 2);
            clean_area(create_point(x + 1, y), create_point(x + 1, COLS - 1));
        }
    }
    load_user(&user);
    char password[MAX_SIZE];
    x += 3;
    while (true) {
        mvprintw(x, y, "Enter Password:");
        print_message_with_color(x + 2, y - 15, "It you have forgotten your password press space button", 2);
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
            print_message_with_color(x, y + 16, "Incorrect Password!", 2);
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
            print_message_with_color(x, y + 16, "Username is already used!", 2);
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
        print_message_with_color(x + 2, y - 30, "Password must contain at least 7 characters, 1 number character, 1 capital letter and 1 small letter.", 2);
        print_message_with_color(x + 3, y, "To generate password press space button", 2);
        move(x + 1, y);
        noecho();
        int len = 0, key = getch();
        while (key != '\n') {
            if (key == ' ') {
                len = MAX_SIZE;
                do {
                    strcpy(user.password, generate_password());
                } while (!valid_password(user.password));
                print_message_with_color(x + 1, y, user.password, 2);
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
            print_message_with_color(x, y + 16, "Invalid Password!", 2);
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
            print_message_with_color(x, y + 16, "Invalid Email!", 2);
            clean_area(create_point(x + 1, y), create_point(x + 1, COLS - 1));
        }
    }
    create_user(&user);
}

void difficulty_menu() {
    clear();
    char *options[] = {"Easy (default)", "Medium", "Hard"};
    int choice = create_list(create_point(LINES / 4, COLS / 3), options, 3, 1);
    DIFFICULTY = choice;
    go_to_settings();
}

void hero_color_menu() {
    clear();
    char *options[] = {"Blue (default)", "Red", "Green"};
    int choice = create_list(create_point(LINES / 4, COLS / 3), options, 3, 1);
    hero_color = choice + 1;
    go_to_settings();
}

void music_menu() {
    clear();
    char *options[] = {"No Music (default)", "ann", "Angry_Birds"};
    int choice = create_list(create_point(LINES / 4, COLS / 3), options, 3, 1);
    if (choice == 0)
        terminate_music();
    else
        play_song(options[choice]);
    go_to_settings();
}

void go_to_settings() {
    clear();
    char *options[] = {"Set Difficulty", "Hero Color", "Music", "Back"};
    int choice = create_list(create_point(LINES / 4, COLS / 3), options, 4, 1);
    if (choice == 0)
        difficulty_menu();
    else if (choice == 1)
        hero_color_menu();
    else if (choice == 2)
        music_menu();
    else
        pregame_menu();
}

void scoreboard_menu() {
    clear();
    curs_set(FALSE);
    struct miniUser** user_list;
    get_users(&user_list);
    if (user_list == NULL) {
        print_message_with_color(2, COLS / 3, "There are no users!", 2);
        print_message_with_color(4, COLS / 3, "Press any key to return ...", 2);
        getch();
        return;
    }
    sort_miniusers(user_list);
    print_message_with_color(LINES - 1, 0, "Press (Q) to return ...", 2);
    // Titles
    // Rank, Username, Golds, Games, Time
    print_message_with_color(ST_X, ST_Y, "Rank", 4);
    print_message_with_color(ST_X, ST_Y + 10, "Username", 4);
    print_message_with_color(ST_X, ST_Y + 30, "Score", 4);
    print_message_with_color(ST_X, ST_Y + 40, "Golds", 4);
    print_message_with_color(ST_X, ST_Y + 50, "Games", 4);
    print_message_with_color(ST_X, ST_Y + 60, "Time", 4);
    // Users
    int user_pos;
    for (int i = 0; i < USERS; i++)
        if (!strcmp(user_list[i]->username, user.username))
            user_pos = i;
    int cur = 0, key = -1, sz = (USERS >= 4? 4: USERS), st = 0;
    timeout(0);
    do {
        if (key == KEY_UP) {
            if (cur > 0)
                --cur;
            else if (st > 0)
                --st;
        }
        if (key == KEY_DOWN) {
            if (cur + 1 < sz)
                ++cur;
            else if (st + sz < USERS)
                ++st;
        }
        print_user_list(user_list, st, sz, cur, user_pos);
        key = getch();
    } while (key != 'Q');
    timeout(-1);
    pregame_menu();
}

void pregame_menu() {
    clear();
    curs_set(FALSE);
    noecho();
    char *options[] = {"Load previous game", "Create new game", "Scoreboard", "Settings"};
    int choice = create_list(create_point(LINES / 4, COLS / 3), options, 4, 1);
    if (choice == 0) { // load game
        if (is_guest || !has_map(&user) || !check_health()) {
            clear();
            print_message_with_color(LINES / 3, COLS / 3, "There is no previous name for this account!", 2);
            print_message_with_color(LINES / 3 + 1, COLS / 3, "Press any key to return to the previous menu.", 2);
            getch();
            pregame_menu();
        }
    }
    else if (choice == 1) { // create new game
        generate_map(&user);
        update_user(&user);
    }
    else if (choice == 2) {
        scoreboard_menu();
    }
    else { // settings
        go_to_settings();
    }
}

void hunger_menu() {
    clear();
    timeout(-1);
    curs_set(FALSE);
    int x = LINES / 3 - 2, y = COLS / 3;
    print_hunger(x, y, &user);
    print_message_with_color(x + 1, y - 15, "The following foods are available in your bag. choose one to recover hunger:", 2);
    refresh();
    char *options[user.bag.number_of_food];
    for (int i = 0; i < user.bag.number_of_food; i++) {
        if (user.bag.food[i] == 'f') {
            options[i] = malloc(sizeof(char) * 12);
            strcpy(options[i], "Normal food");
        }
        // else
    }
    int choice = create_list(create_point(x + 3, y), options, user.bag.number_of_food, 3);
    switch (user.bag.food[choice]) {
        case 'f':
            consume_food(3);
            break;
        default:
            break;
    }
    --user.bag.number_of_food;
    for (int i = choice; i < user.bag.number_of_food; i++)
        user.bag.food[i] = user.bag.food[i + 1];
    timeout(0);
}

void consume_food(int x) {
    if (10 - user.hunger >= x)
        user.hunger += x;
    else {
        x -= 10 - user.hunger;
        user.hunger = 10;
        user.health += x;
        if (user.health > 10)
            user.health = 10;
    }
}

void move_player(int key, char ***map) {
    struct Point nxt;
    if (key == KEY_UP) {
        nxt = next_point(user.pos, 0);
        if (is_in_map(nxt) && not_restricted(&user, map, nxt))
            user.pos.x = nxt.x, user.pos.y = nxt.y;
    }
    else if (key == KEY_RIGHT) {
        nxt = next_point(user.pos, 1);
        if (is_in_map(nxt) && not_restricted(&user, map, nxt))
            user.pos.x = nxt.x, user.pos.y = nxt.y;
    }
    else if (key == KEY_DOWN) {
        nxt = next_point(user.pos, 2);
        if (is_in_map(nxt) && not_restricted(&user, map, nxt))
            user.pos.x = nxt.x, user.pos.y = nxt.y;
    }
    else if (key == KEY_LEFT) {
        nxt = next_point(user.pos, 3);
        if (is_in_map(nxt) && not_restricted(&user, map, nxt))
            user.pos.x = nxt.x, user.pos.y = nxt.y;
    }
}

void appear_map(struct Point p, int depth) {
    int level = user.level;
    (user.mask)[level][p.x][p.y] = 1;
    if (!depth)
        return;
    if (is_in_room(&(user.map[level]), p)) {
        for (int dir = 0; dir < 8; dir++) {
            struct Point nxt = next_point(p, dir);
            if (is_in_room(&(user.map[level]), nxt) && !(user.mask)[level][nxt.x][nxt.y])
                appear_map(nxt, 5);
            else
                (user.mask)[level][nxt.x][nxt.y] = 1;
        }
    }
    else { // +, #
        for (int dir = 0; dir < 4; dir++) {
            struct Point nxt = next_point(p, dir);
            if (is_in_map(nxt)) {
                if (is_in_corridor(&(user.map[level]), nxt))
                    appear_map(nxt, depth - 1);
                if (is_in_room(&(user.map[level]), nxt) && depth == 5)
                    appear_map(nxt, 5);
            }
        }
    }
}

void appear_nightmare(struct Point p, int depth) {
    user.mask[user.level][p.x][p.y] = 1;
    if (!depth)
        return;
    for (int dir = 0; dir < 8; dir++) {
        struct Point nxt = next_point(p, dir);
        if (is_in_map(nxt) && user.theme[user.level][nxt.x][nxt.y] == 'n')
            appear_nightmare(nxt, depth - 1);
    }
}

void disappear_nightmare(struct Point p, int depth) {
    user.mask[user.level][p.x][p.y] = 0;
    if (!depth)
        return;
    for (int dir = 0; dir < 8; dir++) {
        struct Point nxt = next_point(p, dir);
        if (is_in_map(nxt) && user.theme[user.level][nxt.x][nxt.y] == 'n')
            disappear_nightmare(nxt, depth - 1);
    }
}

void play_trap(struct Point p) {
    clear();
    print_message_with_color(2, COLS / 3, "You fell into a trap!", 2);
    int level = user.level;
    user.health -= user.trap[level][p.x][p.y].damage;
    user.map[level][p.x][p.y] = '^';
    print_status(&user);
    print_message_with_color(4, COLS / 3, "Press any key to return ...", 2);
    timeout(-1);
    getch();
    clear();
}

int check_health() {
    if (user.health <= 0) {
        clear();
        print_message_with_color(LINES / 3, COLS / 3, "There is no health left for you!", 2);
        refresh();
        usleep(500000);
        update_user(&user);
        quit_game();
        return 0;
    }
    return 1;
}

void reached_treasure_room() {
    clear();
    print_message_with_color(LINES / 3, COLS / 3, "You have found the treasure room. Nice job!", 7);
    refresh();
    usleep(1500000);
    update_user(&user);
    quit_game();
}

void play_game() {
    clear();
    int key;
    time_t now, st_enchant;
    int is_in_enchant = 0, in_room = 1;
    time(&now);
    init_time(now);
    do {
        time(&now);
        for (int line = 0; line < 3; line++)
            if (refresh_message(now, line))
                clean_area(create_point(line, 0), create_point(line, COLS - 1));
        check_food(&user, now);
        timeout(0);
        // reduce health --> enchanted room
        if (user.theme[user.level][user.pos.x][user.pos.y] == 'e') {
            if (!is_in_enchant) {
                is_in_enchant = 1;
                st_enchant = now;
            }
            else if (difftime(now, st_enchant) > (2 - DIFFICULTY) + 5) {
                st_enchant = now;
                --user.health;
            }
        }
        else
            is_in_enchant = 0;
        check_health();
        if (user.theme[user.level][user.pos.x][user.pos.y] == 't')
            reached_treasure_room();
        // trap
        if (user.trap[user.level][user.pos.x][user.pos.y].exist)
            play_trap(create_point(user.pos.x, user.pos.y));
        check_health();
        // gold
        if (user.map[user.level][user.pos.x][user.pos.y] == 'g') {
            // int cnt = get_gold(user.map[user.level][user.pos.x][user.pos.y]);
            if (user.theme[user.level][user.pos.x][user.pos.y] != 'n') {
                user.gold++;
                user.score++;
                print_message_with_color(2, 0, "You gained ", 8);
                print_number_with_color(2, 11, 1, 8);
                print_message_with_color(2, 13, "golds!", 8);
                LAST_REFRESH[2] = now;
            }
            user.map[user.level][user.pos.x][user.pos.y] = '.';
        }
        // food
        if (is_food(&user)) {
            if (user.bag.number_of_food == 5) {
                print_message_with_color(1, 0, "Bag is full! Can not pick food!", 2);
                LAST_REFRESH[1] = now;
            }
            else {
                if (user.theme[user.level][user.pos.x][user.pos.y] != 'n') {
                    print_message_with_color(1, 0, "Food has been added to your bag!", 3);
                    LAST_REFRESH[1] = now;
                    user.bag.food[user.bag.number_of_food] = user.map[user.level][user.pos.x][user.pos.y];
                    ++user.bag.number_of_food;
                }
                user.map[user.level][user.pos.x][user.pos.y] = '.';
            }
        }
        // enter new room
        if (user.map[user.level][user.pos.x][user.pos.y] == '+' || user.map[user.level][user.pos.x][user.pos.y] == '_' || user.map[user.level][user.pos.x][user.pos.y] == '|') {
            if (is_new_room(&user)) {
                print_message_with_color(0, 0, "You have entered a new room", 2);
                LAST_REFRESH[0] = now;
            }
        }
        // change level
        if (user.map[user.level][user.pos.x][user.pos.y] == '<') {
            --user.level;
            timeout(-1);
        }
        else if (user.map[user.level][user.pos.x][user.pos.y] == '>') {
            ++user.level;
            if (!user.mask[user.level][user.pos.x][user.pos.y]) {
                print_message_with_color(0, 0, "You have entered a new floor. Floor level is ", 2);
                print_number_with_color(0, 45, user.level, 2);
                LAST_REFRESH[0] = now;
            }
            timeout(-1);
        }
        if (user.theme[user.level][user.pos.x][user.pos.y] == 'n')
            appear_nightmare(user.pos, 2);
        else
            appear_map(user.pos, 5);
        print_map(&user, reveal);
        print_message_with_color(user.pos.x + ST_X, user.pos.y + ST_Y, "$", hero_color);
        print_status(&user);
        refresh();
        if (user.theme[user.level][user.pos.x][user.pos.y] == 'n')
            disappear_nightmare(user.pos, 2);
        key = getch();
        move_player(key, &(user.map[user.level]));
        if (key == 'M')
            reveal = 1 - reveal;
        if (key == 'E')
            hunger_menu();
    } while (key != 'Q');
    timeout(-1);
    update_user(&user);
}

void quit_game() {
    usleep(500000);
    refresh();
    clear();
    curs_set(FALSE);
    int x = LINES / 3, y = COLS / 3;
    print_message_with_color(x, y, "Game over!", 2);
    mvprintw(x + 1, y, "Press any key to exit ...");
    refresh();
    getch();
    endwin();
    exit(0);
}
