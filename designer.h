void draw_horizontal_line(struct Point p, int width, char c) {
    move(p.x, p.y);
    for (int i = 0; i < width; i++)
        printw("%c", c);
}

void draw_vertical_line(struct Point p, int height, char c) {
    for (int i = 0; i < height; i++)
        mvprintw(p.x + i, p.y, "%c", c);
}

void draw_border(struct Point p, int height, int width, char c_hor, char c_ver) {
    draw_horizontal_line(p, width, c_hor);
    p.x++;
    draw_vertical_line(p, height - 2, c_ver);
    p.y += width - 1;
    draw_vertical_line(p, height - 2, c_ver);
    p = create_point(p.x + height - 2, p.y - width + 1);
    draw_horizontal_line(p, width, c_hor);
}

void draw_box(char title[], struct Point p, int height, int width, char c) {
    height += 2;
    width += 2;
    draw_border(p, height, width, c, c);
    if (title[0] != '\0') {
        attron(A_BOLD | A_UNDERLINE);
        mvprintw(p.x + 1, p.y + width / 2 - strlen(title) / 2, "%s", title);
        attroff(A_BOLD | A_UNDERLINE);
        draw_horizontal_line(create_point(p.x + 2, p.y), width, c);
    }
}

void clean_area(struct Point top_left, struct Point bottom_right) {
    for (int i = top_left.x; i <= bottom_right.x; i++)
        for (int j = top_left.y; j <= bottom_right.y; j++)
            mvprintw(i, j, " ");
}

int create_list(struct Point st, char *options[], int cnt, int color) {
    curs_set(FALSE);
    noecho();
    change_color(color);
    int choice = 0, key = -1;
    do {
        if (key == KEY_UP && choice > 0)
            choice--;
        if (key == KEY_DOWN && choice < cnt - 1)
            choice++;
        for (int i = 0; i < cnt; i++) {
            if (choice == i) {
                attron(A_REVERSE | A_BOLD);
                mvprintw(st.x + 2 * i, st.y, "%s", options[i]);
                attroff(A_REVERSE | A_BOLD);
            }
            else
                mvprintw(st.x + 2 * i, st.y, "%s", options[i]);
        }
        key = getch();
    } while (key != '\n');
    undo_color(color);
    return choice;
}

void update_theme(struct User* user, int level, struct Room* room) {
    for (int i = room->p.x; i < room->p.x + room->height + 2; i++)
        for (int j = room->p.y; j < room->p.y + room->width + 2; j++)
            user->theme[level][i][j] = room->type;
}

int create_room(struct User* user, char ***map, struct Point p, int height, int width, struct Room** rooms, int index, int level) {
    (*rooms)[index].p = create_point(p.x, p.y);
    (*rooms)[index].height = height;
    (*rooms)[index].width = width;
    // determine room type
    if ((rand() % 10) < 7)
        (*rooms)[index].type = 'r';
    else if (rand() % 2) // enchanted room
        (*rooms)[index].type = 'e';
    else // nightmare room
        (*rooms)[index].type = 'n';
    corners[4 * index] = create_point(p.x, p.y);
    corners[4 * index + 1] = create_point(p.x + height + 1, p.y);
    corners[4 * index + 2] = create_point(p.x, p.y + width + 1);
    corners[4 * index + 3] = create_point(p.x + height + 1, p.y + width + 1);
    return 1;
}

void draw_room(char ***map, struct Room* room) {
    for (int j = 0; j < room->width + 2; j++)
        (*map)[room->p.x][room->p.y + j] = '_';

    for (int j = 1; j < room->width + 1; j++)
        (*map)[room->p.x + room->height + 1][room->p.y + j] = '_';
    
    for (int i = 1; i < room->height + 2; i++) {
        (*map)[room->p.x + i][room->p.y] = '|';
        (*map)[room->p.x + i][room->p.y + room->width + 1] = '|';
    }
    for (int i = 0; i < room->height; i++)
        for (int j = 0; j < room->width; j++)
            (*map)[room->p.x + i + 1][room->p.y + j + 1] = '.';
}

int check_rooms_dist(struct Room** rooms, int index) {
    int X_1 = (*rooms)[index].p.x - 5, X_2 = (*rooms)[index].p.x + (*rooms)[index].height + 1 + 5;
    int Y_1 = (*rooms)[index].p.y - 5, Y_2 = (*rooms)[index].p.y + (*rooms)[index].width + 1 + 5;
    for (int i = 0; i < index; i++) {
        int x_1 = (*rooms)[i].p.x - 5, x_2 = (*rooms)[i].p.x + (*rooms)[i].height + 1 + 5;
        int y_1 = (*rooms)[i].p.y - 5, y_2 = (*rooms)[i].p.y + (*rooms)[i].width + 1 + 5;
        if (X_1 <= x_2 && Y_1 <= y_2 && x_1 <= X_2 && y_1 <= Y_2)
            return 0;
    }
    return 1;
}

void corridor_at_point(char ***map, struct Point p) {
    if ((*map)[p.x][p.y] == ' ')
        (*map)[p.x][p.y] = '#';
    else if ((*map)[p.x][p.y] == '_' || (*map)[p.x][p.y] == '|')
        (*map)[p.x][p.y] = '+';
}

void create_corridor(char ***map, struct Point st, struct Point en) {
    while (st.x != en.x || st.y != en.y) {
        int dir = rand() % 4; // 0: up, 1: right, 2: down, 3: left
        struct Point nxt = next_point(st, dir);
        if ((dir == 0 && st.x > en.x) || (dir == 1 && st.y < en.y) || (dir == 2 && st.x < en.x) || (dir == 3 && st.y > en.y)) {
            if (is_in_map(nxt)) {
                corridor_at_point(map, nxt);
                st.x = nxt.x, st.y = nxt.y;
            }
        }
    }
}

void destroy_door(char ***map, struct Point p) {
    if ((p.x > 0 && is_in_room(map, create_point(p.x - 1, p.y))) || (p.x + 1 < GAME_X && is_in_room(map, create_point(p.x + 1, p.y))))
        (*map)[p.x][p.y] = '_';
    else
        (*map)[p.x][p.y] = '|';
}

void trim_rooms(char ***map) {
    // remove multiple doors
    for (int i = 0; i < GAME_X; i++)
        for (int j = 0; j < GAME_Y; j++)
            if ((*map)[i][j] == '+') {
                int valid = 0;
                for (int dir = 0; dir < 4; dir++) {
                    struct Point nxt = next_point(create_point(i, j), dir);
                    if (is_in_map(nxt) && (*map)[nxt.x][nxt.y] == '#')
                        valid = 1;
                }
                if (vertical_neighbor(map, create_point(i, j)) || horizontal_neighbor(map, create_point(i, j)))
                    valid = 1;
                if (!valid)
                    destroy_door(map, create_point(i, j));
            }
}

void clear_map(char ***map) {
    for (int i = 0; i < GAME_X; i++)
        for (int j = 0; j < GAME_Y; j++)
            (*map)[i][j] = ' ';
}

void determine_initial_position(struct User* user, char ***map) {
    do {
        user->pos.x = rand() % GAME_X;
        user->pos.y = rand() % GAME_Y;
    } while ((*map)[user->pos.x][user->pos.y] != '.');
}

void generate_pillar(char ***map, struct Room* room) {
    int number_of_pillars = rand() % 2;
    struct Point pillar;
    if (number_of_pillars) {
        pillar.x = room->p.x + 2 + rand() % (room->height - 2);
        pillar.y = room->p.y + 2 + rand() % (room->width - 2);
        (*map)[pillar.x][pillar.y] = 'O';
    }
}

void generate_traps(struct User* user, struct Room* room, int level) {
    char ***map = &(user->map[level]);
    if (room->type == 'r' || room->type == 'e' || room->type == 'n') {
        int number_of_traps = DIFFICULTY + rand() % 2;
        while (number_of_traps) {
           struct Point pos = create_point(room->p.x + rand() % room->height, room->p.y + rand() % room->width);
            if ((*map)[pos.x][pos.y] == '.' && !user->trap[level][pos.x][pos.y].exist) {
                user->trap[level][pos.x][pos.y].exist = 1;
                user->trap[level][pos.x][pos.y].damage = 1 + rand() % 3;
                --number_of_traps;
            } 
        }
    }
    else { // treasure room
        int number_of_traps = DIFFICULTY + rand() % 7;
        while (number_of_traps) {
           struct Point pos = create_point(room->p.x + rand() % room->height, room->p.y + rand() % room->width);
            if ((*map)[pos.x][pos.y] == '.' && !user->trap[level][pos.x][pos.y].exist) {
                user->trap[level][pos.x][pos.y].exist = 1;
                user->trap[level][pos.x][pos.y].damage = 1 + rand() % 3;
                --number_of_traps;
            } 
        }
    }
}

void generate_staircase(char ***map, struct Point* p) {
    struct Point tmp;
    do {
        tmp.x = rand() % GAME_X;
        tmp.y = rand() % GAME_Y;
    } while ((*map)[tmp.x][tmp.y] != '.');
    (*map)[tmp.x][tmp.y] = '>';
    p->x = tmp.x, p->y = tmp.y;
}

void generate_gold(struct User* user, char ***map, struct Room* room, int level) {
    if (room->type == 'r' || room->type == 'n') {
        int has_gold = rand() % (2 + DIFFICULTY);
        while (!has_gold) {
            int x = room->p.x + 1 + rand() % room->height;
            int y = room->p.y + 1 + rand() % room->width;
            if ((*map)[x][y] == '.') {
                (*map)[x][y] = 'g';
                --has_gold;
                int num = 1 + rand() % 4;
                int p = rand() % 5; // 0: black gold, else: normal
                if (p == 0) {
                    (user->gold)[level][x][y].type = 'b';
                    (user->gold)[level][x][y].cnt = 3 * num;
                }
                else {
                    (user->gold)[level][x][y].type = 'n';
                    (user->gold)[level][x][y].cnt = num;
                }
            }
        }
    }
    // else
}

int get_doors(struct Point **p, char ***map, struct Room* room) {
    int num = 0;
    for (int j = 0; j < room->width + 2; j++) {
        if ((*map)[room->p.x][room->p.y + j] == '+') {
            (*p)[num].x = room->p.x;
            (*p)[num].y = room->p.y + j;
            ++num;
        }
        if ((*map)[room->p.x + room->height + 1][room->p.y + j] == '+') {
            (*p)[num].x = room->p.x + room->height + 1;
            (*p)[num].y = room->p.y + j;
            ++num;
        }
    }
    for (int i = 0; i < room->height + 2; i++) {
        if ((*map)[room->p.x + i][room->p.y] == '+') {
            (*p)[num].x = room->p.x + i;
            (*p)[num].y = room->p.y;
            ++num;
        }
        if ((*map)[room->p.x + i][room->p.y + room->width + 1] == '+') {
            (*p)[num].x = room->p.x + i;
            (*p)[num].y = room->p.y + room->width + 1;
            ++num;
        }
    }
    return num;
}

void create_secret_doors(struct User* user, char ***map, struct Room* room, int level) {
    struct Point* p = malloc(sizeof(struct Point) * 10);
    int num = get_doors(&p, map, room);
    if (room->type == 'e') { // enchanted room
        for (int i = 0; i < num; i++) {
            user->door[level][p[i].x][p[i].y].exist = 1;
            destroy_door(map, p[i]);
        }
    }
    else if (num == 1) { // (Bon Bast)
        user->door[level][p[0].x][p[0].y].exist = 1;
        destroy_door(map, p[0]);
    }
}

void generate_food(char ***map, struct Room* room) {
    if (room->type != 't' && room->type != 'e') {
        int number_of_food = 1 + rand() % (3 - DIFFICULTY);
        while (number_of_food) {
            int x = room->p.x + 1 + rand() % room->height;
            int y = room->p.y + 1 + rand() % room->width;
            if ((*map)[x][y] == '.') {
                (*map)[x][y] = 'f';
                --number_of_food;
            }
        }
    }
}

void generate_weapon(char ***map, struct Room* room) {
    if (room->type == 't' || room->type == 'e')
        return;
    char tmp[] = {'m', 'd', 'M', 'n', 's'}; // mace, dagger, magic wand, normal arrow, sword
    for (int i = 0; i < 5; i++) {
        if (rand() % 5 == 0) {
            while (true) {
                int x = room->p.x + 1 + rand() % room->height;
                int y = room->p.y + 1 + rand() % room->width;
                if ((*map)[x][y] == '.') {
                    (*map)[x][y] = tmp[i];
                    break;
                }
            }
        }
    }
}

void generate_potion(char ***map, struct Room* room) {
    if (room->type == 'e') { // enchant
        int number_of_potion = 4 + rand() % 3;
        while (number_of_potion) {
            int x = room->p.x + 1 + rand() % room->height;
            int y = room->p.y + 1 + rand() % room->width;
            if ((*map)[x][y] == '.') {
                int type = rand() % 3;
                switch(type) {
                    case 0: // health
                        (*map)[x][y] = 'H';
                        break;
                    case 1: // speed
                        (*map)[x][y] = 'S';
                        break;
                    case 2: // damage
                        (*map)[x][y] = 'D';
                        break;
                }
                --number_of_potion;
            }
        }
    }
    else if (room->type != 't') { // normal and nightmare
        if (rand() % 3 == 0) {
            while (true) {
                int x = room->p.x + 1 + rand() % room->height;
                int y = room->p.y + 1 + rand() % room->width;
                if ((*map)[x][y] == '.') {
                    int type = rand() % 3;
                    switch(type) {
                        case 0: // health
                            (*map)[x][y] = 'H';
                            break;
                        case 1: // speed
                            (*map)[x][y] = 'S';
                            break;
                        case 2: // damage
                            (*map)[x][y] = 'D';
                            break;
                    }
                }
                break;
            }
        }
    }
}

void add_items(struct User* user, char ***map, struct Room* room, int level) { // add items to room
    generate_pillar(map, room);
    generate_traps(user, room, level);
    generate_gold(user, map, room, level);
    generate_food(map, room);
    generate_weapon(map, room);
    generate_potion(map, room);
    create_secret_doors(user, map, room, level);
}

void generate_map(struct User* user) {
    clear();
    print_message_with_color(LINES / 3, COLS / 3 - 10, "It may take up to 60 seconds. Thank you for your patience.", 3);
    refresh();
    time_t st;
    time(&st);
    user->number_of_floor = 4 + rand() % 2;
    ++user->number_of_games;
    user->level = 0;
    struct Point staircaise;
    for (int level = 0; level < user->number_of_floor; level++) {
        init_user(user, level);
        char ***map = &(user->map[level]);
        int valid_map = 1;
        number_of_rooms = 6 + rand() % 2;
        struct Room* rooms = malloc(sizeof(struct Room) * number_of_rooms);
        corners = malloc(sizeof(struct Point) * number_of_rooms * 4);
        do {
            time_t now;
            time(&now);
            if (difftime(now, st) > 60) {
                clear();
                print_message_with_color(LINES / 3, COLS / 3, "Failed to generate map!", 2);
                print_message_with_color(LINES / 3 + 2, COLS / 3, "Press any key to regenerate ...", 3);
                refresh();
                getch();
                clear();
                refresh();
                generate_map(user);
                return;
            }
            valid_map = 1;
            clear_map(map);
            for (int i = 0; i < number_of_rooms && valid_map; i++) {
                int height = 4 + rand() % 3, width = 4 + rand() % 6;
                int x = rand() % (GAME_X - height - 1), y = rand() % (GAME_Y - width - 1);
                create_room(user, map, create_point(x, y), height, width, &rooms, i, level);
                valid_map &= check_rooms_dist(&rooms, i);
            }
            if (valid_map) {
                for (int i = 0; i < number_of_rooms; i++)
                    draw_room(map, &rooms[i]);
            }
            for (int i = 0; i < number_of_rooms - 1 && valid_map; i++) {
                int delta_x = rand() % (rooms[i].height - 1), delta_y = rand() % (rooms[i].width - 1);
                struct Point p1 = create_point(rooms[i].p.x + 1 + delta_x, rooms[i].p.y + 1 + delta_y);
                delta_x = rand() % (rooms[i + 1].height - 1), delta_y = rand() % (rooms[i + 1].width - 1);
                struct Point p2 = create_point(rooms[i + 1].p.x + 1 + delta_x, rooms[i + 1].p.y + 1 + delta_y);
                create_corridor(map, p1, p2);
            }
            if (level > 0) {
                valid_map &= (*map)[staircaise.x][staircaise.y] == '.';
                (*map)[staircaise.x][staircaise.y] = '<';
            }
            valid_map &= check_corners(map);
        } while (!valid_map);
        trim_rooms(map);
        if (level == user->number_of_floor - 1) { // add treasure room
            int index = rand() % number_of_rooms;
            rooms[index].type = 't';
        }
        for (int i = 0; i < number_of_rooms; i++) {
            update_theme(user, level, &rooms[i]);
            add_items(user, map, &rooms[i], level);
        }
        if (level != user->number_of_floor - 1)
            generate_staircase(map, &staircaise);
        if (level == 0)
            determine_initial_position(user, map);
        // determine_doors_type(user, map);
    }
}

void print_map(struct User* user, int reveal) {
    print_message_with_color(LINES - 1, 0, "Press (Q) to exit game.", 2);
    for (int i = 0; i < GAME_X; i++)
        for (int j = 0; j < GAME_Y; j++) {
            char c = user->theme[user->level][i][j];
            int color;
            switch (c) {
            case 'r':
                color = 1;
                break;
            case 'e':
                color = 5;
                break;
            case 'n':
                color = 6;
                break;
            case 't':
                color = 7;
                break;
            default:
                color = 1;
                break;
            }
            if ((user->mask)[user->level][i][j] == 1 || reveal) {
                if ((user->map)[user->level][i][j] == 'g') {
                    if ((user->gold)[user->level][i][j].type == 'n')
                        print_character_with_color(i + ST_X, j + ST_Y, 'G', 8);
                    else
                        print_character_with_color(i + ST_X, j + ST_Y, 'G', 5);
                }
                else if ((user->map)[user->level][i][j] == 'f') {
                    print_character_with_color(i + ST_X, j + ST_Y, 'f', 3);
                }
                else if (is_weapon((user->map)[user->level][i][j])) {
                    char c = (user->map)[user->level][i][j];
                    if (c == 'm') {
                        print_message_with_color(i + ST_X, j + ST_Y, "⚒", 6);
                    }
                    else if (c == 'd') {
                        print_message_with_color(i + ST_X, j + ST_Y, "🗡", 6);
                    }
                    else if (c ==  'M') {
                        print_message_with_color(i + ST_X, j + ST_Y, "M", 6);
                    }
                    else if (c == 'n') {
                        print_message_with_color(i + ST_X, j + ST_Y, "➳", 6);
                    }
                    else {
                        print_message_with_color(i + ST_X, j + ST_Y, "⚔", 6);
                    }
                }
                else if (is_potion((user->map)[user->level][i][j])) {
                    print_character_with_color(i + ST_X, j + ST_Y, (user->map)[user->level][i][j], 4);
                }
                else
                    print_character_with_color(i + ST_X, j + ST_Y, (user->map)[user->level][i][j], color);
            }
            else
                mvprintw(i + ST_X, j + ST_Y, " ");
        }
}

void print_hunger(int x, int y, struct User* user) {
    print_message_with_color(x, y, "Your Hunger:[", 2);
    attroff(COLOR_PAIR(1));
    attron(COLOR_PAIR(2));
    for (int i = 0; i < user->hunger; i++)
        mvprintw(x, y + 13 + i, "#");
    for (int i = user->hunger; i < 10; i++)
        mvprintw(x, y + 13 + i, ".");
    mvprintw(x, y + 13 + 10, "] (%d out of 10)", user->hunger);
    attroff(COLOR_PAIR(2));
    attron(COLOR_PAIR(1));
}

void print_status(struct User* user) {
    move(GAME_X + ST_X, GAME_Y / 3);
    change_color(4);
    printw("Score: %d \t Gold: %d \t Health: %d \t Games: %d", user->score, user->golds, user->health, user->number_of_games);
    undo_color(4);
    print_hunger(GAME_X + ST_X + 1, GAME_Y / 3 + 1, user);
}

void print_user_list(struct miniUser** user_list, int st, int sz, int cur, int user_pos) {
    clean_area(create_point(ST_X + 2, 0), create_point(GAME_X, GAME_Y));
    for (int i = 0; i < sz; i++) {
        if (i == cur)
            attron(A_REVERSE | A_BOLD);
        int color = 5;
        if (i + st < 3) {
            color = 3;
            attron(A_ITALIC);
        }
        if (user_pos == i + st)
            print_message_with_color(ST_X + 2 * i + 2, 0, "-->", color);
        char name[MAX_SIZE];
        strcpy(name, user_list[i + st]->username);
        switch (i + st) {
        case 0:
            strcat(name, " (legend)");
            mvprintw(ST_X + 2 * i + 2, ST_Y + 10 + strlen(name), "🥇");
            break;
        case 1:
            strcat(name, " (goat)");
            mvprintw(ST_X + 2 * i + 2, ST_Y + 10 + strlen(name), "🥈");
            break;
        case 2:
            strcat(name, " (khafan)");
            mvprintw(ST_X + 2 * i + 2, ST_Y + 10 + strlen(name), "🥉");
            break;
        default:
            break;
        }
        print_number_with_color(ST_X + 2 * i + 2, ST_Y, i + st + 1, color);
        print_message_with_color(ST_X + 2 * i + 2, ST_Y + 10, name, color);
        print_number_with_color(ST_X + 2 * i + 2, ST_Y + 30, user_list[i + st]->score, color);
        print_number_with_color(ST_X + 2 * i + 2, ST_Y + 40, user_list[i + st]->gold, color);
        print_number_with_color(ST_X + 2 * i + 2, ST_Y + 50, user_list[i + st]->number_of_games, color);
        print_message_with_color(ST_X + 2 * i + 2, ST_Y + 60, get_time(user_list[i + st]->first_game), color);
        if (i + st < 3)
            attroff(A_ITALIC);
        if (i == cur)
            attroff(A_REVERSE | A_BOLD);
    }
    refresh();
}