char* get_address(char username[MAX_SIZE]) {
    char *res = malloc(MAX_SIZE * sizeof(char));
    strcpy(res, "users/");
    strcat(res, username);
    strcat(res, ".txt");
    return res;
}
char* get_password(char username[MAX_SIZE]) {
    char *path = get_address(username);
    FILE *fptr = fopen(path, "r");
    char *res = malloc(sizeof(char) * MAX_SIZE);
    fgets(res, MAX_SIZE, fptr);
    fgets(res, MAX_SIZE, fptr);
    return res;
}

int exist_username(char username[MAX_SIZE]) {
    char *path = get_address(username);
    FILE *fptr = fopen(path, "r");
    if (fptr == NULL)
        return 0;
    return 1;
}

int correct_password(char username[], char password[]) {
    char *path = get_address(username);
    FILE *fptr = fopen(path, "r");
    char pass[100];
    fgets(pass, 100, fptr);
    fgets(pass, 100, fptr);
    int n = strlen(pass) - 1;
    if (n != strlen(password))
        return 0;
    for (int i = 0; i < n; i++)
        if (password[i] != pass[i])
            return 0;
    return 1;
}

int valid_password(char password[]) {
    int has_number = 0, has_capital = 0, has_small = 0, len = strlen(password);
    if (len < 7)
        return 0;
    for (int i = 0; i < len; i++) {
        if (password[i] >= '0' && password[i] <= '9')
            has_number |= 1;
        if (password[i] >= 'A' && password[i] <= 'Z')
            has_capital |= 1;
        if (password[i] >= 'a' && password[i] <= 'z')
            has_small |= 1;
    }
    return has_number && has_capital && has_small;
}

char* generate_password() {
    char symbols[] = {'!', '@', '#', '$', '%', '^', '&', '*', '?'};
    char capital[26], small[26];
    for (int i = 0; i < 26; i++) {
        capital[i] = 'A' + i;
        small[i] = 'a' + i;
    }
    int len = 7 + rand() % 4, type = rand() % 4;
    char *res = malloc(sizeof(char) * (len + 1));
    for (int i = 0; i < len; i++) {
        if (type == 0) // number
            res[i] = '0' + rand() % 10; 
        else if (type == 1) { // symbol
            int idx = rand() % 9;
            res[i] = symbols[idx];
        }
        else if (type == 2) { // capital
            int idx = rand() % 26;
            res[i] = capital[idx];
        }
        else { // small
            int idx = rand() % 26;
            res[i] = small[idx];
        }
        type = rand() % 4;
    }
    res[len] = '\0';
    return res;
}

int valid_email(char email[]) {
    int len = strlen(email), at = -1;
    for (int i = 0; i < len; i++)
        if (email[i] == '@') {
            if (at != -1)
                return 0;
            at = i;
        }
    if (at == -1 || at == 0)
        return 0;
    int noghte = -1;
    for (int i = len - 1; i > at; i--)
        if (email[i] == '.') {
            if (noghte != -1)
                return 0;
            noghte = i;
        }
    if (noghte == -1 || noghte == len - 1 || noghte == at + 1)
        return 0;
    return 1;
}

void create_user(char username[], char password[], char email[]) {
    char *path = get_address(username);
    FILE *fptr = fopen(path, "w");
    fprintf(fptr, "%s\n", username);
    fprintf(fptr, "%s\n", password);
    fprintf(fptr, "%s\n", email);
    fprintf(fptr, "0\n"); // score
    fprintf(fptr, "0\n"); // gold
    fclose(fptr);
    // add to users.txt
    fptr = fopen("users/users.txt", "a");
    fprintf(fptr, "%s\n", username);
    fclose(fptr);
}

void load_user(struct User* user) {
    char *path = get_address(user->username);
    FILE *fptr = fopen(path, "r");
    char line[MAX_SIZE];
    fgets(line, MAX_SIZE, fptr); // username;
    fgets(line, MAX_SIZE, fptr); // password
    fgets(line, MAX_SIZE, fptr); // email
    trim(line);
    strcpy(user->email, line);
    fgets(line, MAX_SIZE, fptr); // score
    trim(line);
    user->score = get_num(line);
    fgets(line, MAX_SIZE, fptr); // gold
    trim(line);
    user->gold = get_num(line);
    for (int i = 0; i < GAME_X; i++) { // map
        fgets(line, MAX_SIZE, fptr);
        for (int j = 0; j < GAME_Y; j++)
            (user->map)[i][j] = line[j];
    }
    for (int i = 0; i < GAME_X; i++) { // mask
        fgets(line, MAX_SIZE, fptr);
        for (int j = 0; j < GAME_Y; j++)
            (user->mask)[i][j] = line[j] - '0';
    }
    fgets(line, MAX_SIZE, fptr); // user.pos
    trim(line);
    char *token = strtok(line, " ");
    user->pos.x = get_num(token);
    token = strtok(NULL, " ");
    user->pos.y = get_num(token);
}

int has_map(struct User* user) {
    char *path = get_address(user->username);
    FILE *fptr = fopen(path, "r");
    char line[MAX_SIZE];
    for (int i = 0; i < 6; i++)
        if (fgets(line, MAX_SIZE, fptr) == NULL) {
            fclose(fptr);
            return 0;
        }
    fclose(fptr);
    return 1;
}

void update_user(struct User* user) {
    char *path = get_address(user->username);
    FILE *fptr = fopen(path, "w");
    fprintf(fptr, "%s\n", user->username);
    fprintf(fptr, "%s\n", user->password);
    fprintf(fptr, "%s\n", user->email);
    fprintf(fptr, "%d\n", user->score);
    fprintf(fptr, "%d\n", user->gold);
    for (int i = 0; i < GAME_X; i++) {
        for (int j = 0; j < GAME_Y; j++)
            fprintf(fptr, "%c", (user->map)[i][j]);
        fprintf(fptr, "\n");
    }
    for (int i = 0; i < GAME_X; i++) {
        for (int j = 0; j < GAME_Y; j++)
            fprintf(fptr, "%d", (user->mask)[i][j]);
        fprintf(fptr, "\n");
    }
    fprintf(fptr, "%d %d\n", user->pos.x, user->pos.y);
    fclose(fptr);
}