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

int create_room(char ***map, struct Point p, int height, int width, struct Point** rooms, int index) {
    // create border
    (*map)[p.x][p.y] = '_'; // top left
    (*map)[p.x][p.y + width + 1] = '_'; // top right
    for (int j = 0; j < width; j++) {
        if ((*map)[p.x][p.y + 1 + j] != ' ' || (*map)[p.x + height + 1][p.y + 1 + j] != ' ')
            return 0;
        (*map)[p.x][p.y + 1 + j] = '_';
        (*map)[p.x + height + 1][p.y + 1 + j] = '_';
    }
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
    (*rooms)[index] = create_point(p.x + height / 2, p.y + width / 2);
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

void generate_map(struct User* user) {
    char ***map = &(user->map);
    int valid_map = 1;
    number_of_rooms = 6 + rand() % 5;
    struct Point* rooms = malloc(sizeof(struct Point) * number_of_rooms);
    corners = malloc(sizeof(struct Point) * number_of_rooms * 4);
    do {
        valid_map = 1;
        clear_map(map);
        for (int i = 0; i < number_of_rooms; i++) {
            int height = 4 + rand() % 3, width = 4 + rand() % 10;
            int x = rand() % (GAME_X - height - 1), y = rand() % (GAME_Y - width - 1);
            valid_map &= create_room(map, create_point(x, y), height, width, &rooms, i);
        }
        for (int i = 0; i < number_of_rooms - 1; i++) {
            create_corridor(map, rooms[i], rooms[i + 1]);
        }
        valid_map &= check_corners(map);
    } while (!valid_map);
    int initial_room = rand() % number_of_rooms;
    user->pos.x = rooms[initial_room].x, user->pos.y = rooms[initial_room].y;
    trim_rooms(map);
}

void print_map(char ***map) {
    for (int i = 0; i < GAME_X; i++) {
        move(ST_X + i, ST_Y);
        // move(i, 0);
        for (int j = 0; j < GAME_Y; j++)
            printw("%c", (*map)[i][j]);
    }
}