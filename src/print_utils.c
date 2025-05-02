#include <stdio.h>
#include "print_utils.h"

void print_board(const struct tetris *t) {
    for (int i = ROW - 1; i >= 0; i--) {
        for (int j = COL_SHIFT; j < COL + COL_SHIFT; j++) {
            if (t->board[i] & (1 << j)) {
                printf("%c", FULL_CHAR);
            } else {
                printf("%c", EMPTY_CHAR);
            }
        }
        printf("\n");
    }
    printf("\n");
}

void print_piece(const struct piece *p, int rotation) {
    const struct rotation *rot = &p->rotations[rotation];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i < rot->height && j < rot->width && (rot->shape[i] & (1 << j))) {
                putchar('*');
            } else {
                putchar('.');
            }
        }
        putchar('\n');
    }
}

void print_pieces_side_by_side(int col, const struct piece *p1, int rot1, const struct piece *p2, int rot2) {
    const struct rotation *rot1_ptr = &p1->rotations[rot1];
    const struct rotation *rot2_ptr = &p2->rotations[rot2];

    for (int i = MAX_BRICK_WIDTH - 1; i >= 0; i--) {
        for (int j = 0; j < col; j++) {
            putchar(' ');
        }
        for (int j = 0; j < MAX_BRICK_WIDTH; j++) {
            if (i < rot1_ptr->height && j < rot1_ptr->width && (rot1_ptr->shape[i] & (1 << j))) {
                putchar(FULL_CHAR);
            } else {
                putchar(' ');
            }
        }
        printf("    "); // Space between pieces
        for (int j = 0; j < MAX_BRICK_WIDTH; j++) {
            if (i < rot2_ptr->height && j < rot2_ptr->width && (rot2_ptr->shape[i] & (1 << j))) {
                putchar(FULL_CHAR);
            } else {
                putchar(' ');
            }
        }
        putchar('\n');
    }
}