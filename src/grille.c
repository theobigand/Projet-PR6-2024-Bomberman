#include "grille.h"

void setup_board(board* board) {
    board->h = LINES - 2 - 1; // 2 rows reserved for border, 1 row for chat
    board->w = COLUMNS - 2; // 2 columns reserved for border
    // Allouer la mémoire pour la grille
    board->grid = calloc(board->w * board->h, sizeof(char));

    // Vérifier que l'allocation de mémoire a réussi
    if (!board->grid) {
        perror("Erreur d'allocation de mémoire pour la grille");
        exit(EXIT_FAILURE);
    }

    // Remplir la grille avec des bords de '1' et l'intérieur de '0'
    for (int y = 0; y < board->h; y++) {
        for (int x = 0; x < board->w; x++) {
            if (y == 0 || y == board->h - 1 || x == 0 || x == board->w - 1) {
                board->grid[y * board->w + x] = '1'; // Bords
            } else {
                board->grid[y * board->w + x] = '0'; // Intérieur
            }
        }
    }
}

void free_board(board* board) {
    free(board->grid);
}

int get_grid(board* b, int x, int y) {
    return b->grid[y * b->w + x];
}

void set_grid(board* b, int x, int y, int v) {
    b->grid[y * b->w + x] = v;
}


void refresh_game(board* b, line* l) {

    // Update grid
    int x, y;
    for (y = 0; y < b->h; y++) {
        for (x = 0; x < b->w; x++) {
            char c;
            switch (get_grid(b, x, y)) {
            case '1': // Mur horizontal
                c = '1';
                break;
            case '5': // Mur vertical
                c = '5';
                break;
            case '6': // Coin en bas à gauche
                c = '6';
                break;
            case '7': // Coin en bas à droite
                c = '7';
                break;
            case '8': // Coin en haut à droite
                c = '8';
                break;
            case '0': // Espace vide
                c = ' ';
                break;
            default:
                c = ' '; // Par défaut, afficher un espace
                break;
            }
            mvaddch(y + 1, x + 1, c);
        }
    }

    // Draw borders
    for (x = 0; x < b->w + 2; x++) {
        mvaddch(0, x, '-');
        mvaddch(b->h + 1, x, '-');
    }
    for (y = 0; y < b->h + 2; y++) {
        mvaddch(y, 0, '|');
        mvaddch(y, b->w + 1, '|');
    }

    // Update chat text (commented out as per original function)
    // attron(COLOR_PAIR(1)); // Enable custom color 1
    // attron(A_BOLD); // Enable bold
    // for (x = 0; x < b->w + 2; x++) {
    //     if (x >= TEXT_SIZE || x >= l->cursor)
    //         mvaddch(b->h + 2, x, ' ');
    //     else
    //         mvaddch(b->h + 2, x, l->data[x]);
    // }
    // attroff(A_BOLD); // Disable bold
     //attroff(COLOR_PAIR(1)); // Disable custom color 1

    refresh(); // Apply the changes to the terminal
    //printf("epojgvpoejgvopegv\n");
}

ACTION control_chat(line* l) {
    int c;
    int prev_c = ERR;
    // We consume all similar consecutive key presses
    while ((c = getch()) != ERR) { // getch returns the first key press in the queue
        if (prev_c != ERR && prev_c != c) {
            ungetch(c); // put 'c' back in the queue
            break;
        }
        prev_c = c;
    }
    ACTION a = NONE;
    switch (prev_c) {
    case ERR:
        break;
    case 'q':
        a = QUIT;
    case 27: // pour le escape mais pas instantaner
        break;
    case 'z':
        a=CHAT;
        break;
    case KEY_BACKSPACE:
    case 127:
        if (l->cursor > 0)
            l->cursor--;
        break;
    default:
        if (prev_c >= ' ' && prev_c <= '~' && l->cursor < TEXT_SIZE)
                l->data[(l->cursor)++] = prev_c;
            break;
        break;
    }
    return a;
}


ACTION control(line* l) {
    int c;
    int prev_c = ERR;
    // We consume all similar consecutive key presses
    while ((c = getch()) != ERR) { // getch returns the first key press in the queue
        if (prev_c != ERR && prev_c != c) {
            ungetch(c); // put 'c' back in the queue
            break;
        }
        prev_c = c;
    }
    ACTION a = NONE;
    switch (prev_c) {
    case ERR:
        break;
    case KEY_LEFT:
        a = LEFT;
        break;
    case KEY_RIGHT:
        a = RIGHT;
        break;
    case KEY_UP:
        a = UP;
        break;
    case KEY_DOWN:
        a = DOWN;
        break;
    case 'q':
        a = QUIT;
    case 27: // pour le escape mais pas instantaner
        break;
    case KEY_BACKSPACE:
    case 127:
        if (l->cursor > 0)
            l->cursor--;
        break;
    default:
        if (prev_c >= ' ' && prev_c <= '~' && l->cursor < TEXT_SIZE)
            l->data[(l->cursor)++] = prev_c;
        break;
    }
    return a;
}

bool perform_action(board* b, pos* p, ACTION a) {
    int xd = 0;
    int yd = 0;
    switch (a) {
    case LEFT:
        xd = -1;
        yd = 0;
        break;
    case RIGHT:
        xd = 1;
        yd = 0;
        break;
    case UP:
        xd = 0;
        yd = -1;
        break;
    case DOWN:
        xd = 0;
        yd = 1;
        break;
    case QUIT:
        end_print();
        return true;
    default:
        break;
    }
    set_grid(b, p->x, p->y, 0); // je remplace par du vide sur l'ancienne pos
    p->x += xd;
    p->y += yd;
    p->x = (p->x + b->w) % b->w;
    p->y = (p->y + b->h) % b->h;
    set_grid(b, p->x, p->y, 1); // je remplace par 1 la nouvelle pos
    return false;
}

int manage_action(ACTION a) {
    //printf("Control action: %d\n", a); // Debug statement

    switch (a) {
    case UP:
        return MOVE_NORTH;
    case LEFT:
        return MOVE_WEST;
    case DOWN:
        return MOVE_SOUTH;
    case RIGHT:
        return MOVE_EAST;
    case QUIT:
        return QUIT_GAME;
    default:
        // fprintf(stderr, "Unknown action\n");
        return -1;
    }
    // This line will never be reached due to the return statements within the switch.
    return 1;
}

void init_print() {
    initscr(); /* Start curses mode */
    raw(); /* Disable line buffering */
    intrflush(stdscr, FALSE); /* No need to flush when intr key is pressed */
    keypad(stdscr, TRUE); /* Required in order to get events from keyboard */
    nodelay(stdscr, TRUE); /* Make getch non-blocking */
    noecho(); /* Don't echo() while we do getch (we will manually print characters when relevant) */
    curs_set(0); // Set the cursor to invisible
    start_color(); // Enable colors
    init_pair(1, COLOR_RED, COLOR_BLACK); // Define a new color style (text is yellow, background is black)
}

void end_print() {
    curs_set(1); // Set the cursor to visible again
    endwin(); /* End curses mode */
}

// int main() {
//     board* b = malloc(sizeof(board));
//     line* l = malloc(sizeof(line));
//     l->cursor = 0;
//     pos* p = malloc(sizeof(pos));
//     p->x = 0; p->y = 0;

    // NOTE: All ncurses operations (getch, mvaddch, refresh, etc.) must be done on the same thread.
//     initscr(); /* Start curses mode */
//     raw(); /* Disable line buffering */
//     intrflush(stdscr, FALSE); /* No need to flush when intr key is pressed */
//     keypad(stdscr, TRUE); /* Required in order to get events from keyboard */
//     nodelay(stdscr, TRUE); /* Make getch non-blocking */
//     noecho(); /* Don't echo() while we do getch (we will manually print characters when relevant) */
//     curs_set(0); // Set the cursor to invisible
//     start_color(); // Enable colors
//     init_pair(1, COLOR_RED, COLOR_BLACK); // Define a new color style (text is yellow, background is black)

//     setup_board(b);
//     while (true) {
//         ACTION a = control(l);
//         if (perform_action(b, p, a)) break;
//         refresh_game(b, l);
//         usleep(30 * 1000);
//     }
//     free_board(b);

//     curs_set(1); // Set the cursor to visible again
//     endwin(); /* End curses mode */

//     free(p); free(l); free(b);

//     return 0;
// }