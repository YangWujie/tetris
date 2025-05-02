#ifndef PRINT_UTILS_H
#define PRINT_UTILS_H

#include "tetris.h"

void print_board(const struct tetris *t);
void print_piece(const struct piece *p, int rotation);
void print_pieces_side_by_side(int col, const struct piece *p1, int rot1, const struct piece *p2, int rot2);

#endif // PRINT_UTILS_H
