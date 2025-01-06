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

int create_list(struct Point st, char *options[], int cnt) {
    clear();
    curs_set(FALSE);
    noecho();
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
    return choice;
}

int create_room(char ***map, struct Point p, int height, int width, struct Room** rooms, int index) {
    // create border
    if ((*map)[p.x][p.y] != ' ' || (*map)[p.x][p.y + width + 1] != ' ')
        return 0;
    (*map)[p.x][p.y] = '_'; // top left
    (*map)[p.x][p.y + width + 1] = '_'; // top right
    for (int j = 0; j < width; j++) {
        if ((*map)[p.x][p.y + 1 + j] != ' ' || (*map)[p.x + height + 1][p.y + 1 + j] != ' ')
            return 0;
        (*map)[p.x][p.y + 1 + j] = '_';
        (*map)[p.x + height + 1][p.y + 1 + j] = '_';
    }
    if ((*map)[p.x + height + 1][p.y] != ' ' || (*map)[p.x + height + 1][p.y + width + 1] != ' ')
        return 0;
    (*map)[p.x + height + 1][p.y] = '|'; // bottom left
    (*map)[p.x + height + 1][p.y + width + 1] = '|'; // bottom right
    for (int i = 0; i < height; i++) {
        if ((*map)[p.x + 1 + i][p.y] != ' ' || (*map)[p.x + 1 + i][p.y + width + 1] != ' ')
            return 0;
        (*map)[p.x + 1 + i][p.y] = '|';
        (*map)[p.x + 1 + i][p.y + width + 1] = '|';
    }
    // fill room
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++) {
            if ((*map)[p.x + 1 + i][p.y + 1 + j] != ' ')
                return 0;
            (*map)[p.x + 1 + i][p.y + 1 + j] = '.';
        }
    // create pillar
    int number_of_pillars = rand() % 2;
    struct Point pillar;
    if (number_of_pillars) {
        pillar.x = p.x + 2 + rand() % (height - 2);
        pillar.y = p.y + 2 + rand() % (width - 2);
        (*map)[pillar.x][pillar.y] = 'O';
    }
    (*rooms)[index].p = create_point(p.x, p.y);
    (*rooms)[index].height = height;
    (*rooms)[index].width = width;
    (*rooms)[index].type = 'r'; // normal room
    corners[4 * index] = create_point(p.x, p.y);
    corners[4 * index + 1] = create_point(p.x + height + 1, p.y);
    corners[4 * index + 2] = create_point(p.x, p.y + width + 1);
    corners[4 * index + 3] = create_point(p.x + height + 1, p.y + width + 1);
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
    if ((p.x > 0 && (*map)[p.x - 1][p.y] == '.') || (p.x + 1 < GAME_X && (*map)[p.x + 1][p.y] == '.'))
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

void generate_traps(struct User* user, struct Room** rooms, int level) {
    char ***map = &(user->map[level]);
    for (int i = 0; i < number_of_rooms; i++) {
        if ((*rooms)[i].type == 'r') { // normal room
            int number_of_traps = rand() % 2;
            while (number_of_traps) {
                struct Point pos = create_point((*rooms)[i].p.x + rand() % (*rooms)[i].height, (*rooms)[i].p.y + rand() % (*rooms)[i].width);
                if ((*map)[pos.x][pos.y] == '.') {
                    user->trap[level][pos.x][pos.y].exist = 1;
                    user->trap[level][pos.x][pos.y].damage = 1 + rand() % 3;
                    --number_of_traps;
                }
            }
        }
        // else
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

void generate_gold(char ***map, struct Room* room) {
    if (room->type == 'r') {
        int number_of_golds = rand() % 2;
        while (number_of_golds) {
            int x = room->p.x + 1 + rand() % room->height;
            int y = room->p.y + 1 + rand() % room->width;
            if ((*map)[x][y] == '.') {
                (*map)[x][y] = 'g';
                --number_of_golds;
            }
        }
    }
    // else
}

void generate_map(struct User* user) {
    ++user->number_of_games;
    user->level = 0;
    struct Point staircaise;
    for (int level = 0; level < 4; level++) {
        init_user(user, level);
        char ***map = &(user->map[level]);
        int valid_map = 1;
        number_of_rooms = 6 + rand() % 3;
        struct Room* rooms = malloc(sizeof(struct Room) * number_of_rooms);
        corners = malloc(sizeof(struct Point) * number_of_rooms * 4);
        do {
            valid_map = 1;
            clear_map(map);
            for (int i = 0; i < number_of_rooms; i++) {
                int height = 4 + rand() % 3, width = 4 + rand() % 9;
                int x = rand() % (GAME_X - height - 1), y = rand() % (GAME_Y - width - 1);
                valid_map &= create_room(map, create_point(x, y), height, width, &rooms, i);
            }
            for (int i = 0; i < number_of_rooms - 1; i++) {
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
        if (level != 3)
            generate_staircase(map, &staircaise);
        generate_traps(user, &rooms, level);
        if (level == 0)
            determine_initial_position(user, map);
        for (int i = 0; i < number_of_rooms; i++)
            generate_gold(map, &rooms[i]);
        // determine_doors_type(user, map);
    }
}

void print_map(struct User* user, int reveal) {
    print_message_with_color(0, 0, "Floor level is ", 2);
    print_number_with_color(0, 15, user->level + 1, 2);
    print_message_with_color(LINES - 1, 0, "Press (Q) to exit game.", 2);
    for (int i = 0; i < GAME_X; i++) {
        move(ST_X + i, ST_Y);
        // move(i, 0);
        for (int j = 0; j < GAME_Y; j++) {
            if ((user->mask)[user->level][i][j] || reveal)
                printw("%c", (user->map)[user->level][i][j]);
            else
                printw(" ");
        }
    }
}

void print_status(struct User* user) {
    move(GAME_X + ST_X, GAME_Y / 3);
    attroff(COLOR_PAIR(1));
    attron(COLOR_PAIR(4));
    printw("Gold: %d \t Health: %d \t Games: %d", user->gold, user->health, user->number_of_games);
    attroff(COLOR_PAIR(4));
    attron(COLOR_PAIR(1));
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
        print_number_with_color(ST_X + 2 * i + 2, ST_Y + 30, user_list[i + st]->gold, color);
        print_number_with_color(ST_X + 2 * i + 2, ST_Y + 40, user_list[i + st]->number_of_games, color);
        print_message_with_color(ST_X + 2 * i + 2, ST_Y + 50, get_time(user_list[i + st]->first_game), color);
        if (i + st < 3)
            attroff(A_ITALIC);
        if (i == cur)
            attroff(A_REVERSE | A_BOLD);
    }
    refresh();
}