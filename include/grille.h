#ifndef GRILLE_H
#define GRILLE_H
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define TEXT_SIZE 255
#define LINES 23
#define COLUMNS 22

typedef enum ACTION { NONE, UP, DOWN, LEFT, RIGHT, QUIT, CHAT } ACTION;

typedef struct board {
    char* grid;
    int w;
    int h;
} board;

typedef struct line {
    char data[TEXT_SIZE];
    int cursor;
} line;

typedef struct pos {
    int x;
    int y;
} pos;

#define MOVE_NORTH 0
#define MOVE_EAST 1
#define MOVE_SOUTH 2
#define MOVE_WEST 3
#define QUIT_GAME -2


typedef struct requete_info
{
    uint16_t codereq;
    uint16_t id;
    uint16_t eq;
} requete_info;

void setup_board(board* board);
void free_board(board* board);
int get_grid(board* b, int x, int y);
void set_grid(board* b, int x, int y, int v);
void refresh_game(board* b, line* l);
ACTION control(line* l);
ACTION control_chat(line* l);
bool perform_action(board* b, pos* p, ACTION a);
void init_print();
void end_print();
int manage_action(ACTION a);

#endif