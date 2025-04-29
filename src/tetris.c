#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include "tetris.h"

// 得分规则
const int SCORE_TABLE[] = {0, 100, 300, 500, 800};

// 俄罗斯方块形状定义
struct piece pieces[] = {
    // I
    {
        2,
        {
            {
                4,  // 宽度
                1,  // 高度
                { 0b1111 }, // 形状
                { 1, 1, 1, 1 }, // elevation
                { 0, 0, 0, 0 }  // concave
            },
            {
                1,  // 宽度
                4,  // 高度
                { 0b1, 0b1, 0b1, 0b1 }, // 形状
                { 4 }, // elevation
                { 0 }  // concave
            }
        }
    },

    // T
    {
        4,
        {
            {
                3,  // 宽度
                2,  // 高度
                { 0b111, 0b010 }, // 形状
                { 1, 2, 1 }, // elevation
                { 0, 0, 0 }  // concave
            },
            {
                2,  // 宽度
                3,  // 高度
                { 0b01, 0b11, 0b01 }, // 形状
                { 3, 2 }, // elevation
                { 0, 1 }, // concave
            },
            {
                3,  // 宽度
                2,  // 高度
                { 0b010, 0b111 }, // 形状
                { 2, 2, 2 }, // elevation
                { 1, 0, 1 }  // concave
            },
            {
                2,   // 宽度
                3,   // 高度
                { 0b10, 0b11, 0b10 }, // shape
                { 2, 3 },             // elevation
                { 1, 0 }              // concave
            }
        }
    },

    // O
    {
        1,
        {
            {
                2,  // 宽度
                2,  // 高度
                { 0b11, 0b11 }, // 形状
                { 2, 2 }, // elevation
                { 0, 0 }  // concave
            }
        }
    },

    // J
    {
        4,
        {
            {
                3,  // 宽度
                2,  // 高度
                { 0b111, 0b001 }, // 形状
                { 2, 1, 1 }, // elevation
                { 0, 0, 0 }  // concave
            },
            {
                2,   // 宽度
                3,   // 高度
                { 0b01, 0b01, 0b11 }, // shape
                { 3, 3 },             // elevation
                { 0, 2 }              // concave
            },
            {
                3,   // 宽度
                2,   // 高度
                { 0b100, 0b111 }, // shape
                { 2, 2, 2 },      // elevation
                { 1, 1, 0 }       // concave
            },
            {
                2,   // 宽度
                3,   // 高度
                { 0b11, 0b10, 0b10 }, // shape
                { 1, 3 },             // elevation
                { 0, 0 }              // concave
            }
        }
    },

    // L
    {
        4,
        {
            {
                3,  // 宽度
                2,  // 高度
                { 0b111, 0b100 }, // 形状
                { 1, 1, 2 }, // elevation
                { 0, 0, 0 }  // concave
            },
            {
                2,   // 宽度
                3,   // 高度
                { 0b11, 0b01, 0b01 }, // shape
                { 3, 1 },             // elevation
                { 0, 0 }              // concave
            },
            {
                3,   // 宽度
                2,   // 高度
                { 0b001, 0b111 }, // shape
                { 2, 2, 2 },      // elevation
                { 0, 1, 1 }       // concave
            },
            {
                2,   // 宽度
                3,   // 高度
                { 0b10, 0b10, 0b11 }, // shape
                { 3, 3 },             // elevation
                { 2, 0 }              // concave
            }
        }
    },

    // S
    {
        2,
        {
            {
                3,  // 宽度
                2,  // 高度
                { 0b011, 0b110 }, // 形状
                { 1, 2, 2 }, // elevation
                { 0, 0, 1 }  // concave
            },
            {
                2,   // 宽度
                3,   // 高度
                { 0b10, 0b11, 0b01 }, // shape
                { 3, 2 },             // elevation
                { 1, 0 }              // concave
            }
        }
    },

    // Z
    {
        2,
        {
            {
                3,  // 宽度
                2,  // 高度
                { 0b110, 0b011 }, // 形状
                { 2, 2, 1 }, // elevation
                { 1, 0, 0 }  // concave
            },
            {
                2,   // 宽度
                3,   // 高度
                { 0b01, 0b11, 0b10 }, // shape
                { 2, 3 },             // elevation
                { 0, 1 }              // concave
            }
        }
    }
};

struct tetris tetris;

static void init_pieces(struct piece *p) {
    for (int i = 0; i < PIECE_TYPES; i++) {
        for (int j = 0; j < MAX_ROTATIONS; j++) {
            for (int k = 0; k < MAX_BRICK_WIDTH; k++) {
                p[i].rotations[j].solid[k] = 
                    p[i].rotations[j].elevation[k] - p[i].rotations[j].concave[k];
            }
        }
    }
}

static void init_board(struct tetris *t) {
    for (int i = 0; i < ROW; i++) {
        t->board[i] = EMPTY_ROW; // 初始化棋盘为空
    }
}

void init() {
    srand(time(NULL)); // 初始化随机数种子
    init_board(&tetris);
    init_pieces(pieces);
}

void print_piece(struct piece *p) {
    for (int i = 0; i < p->count; i++) {
        printf("Rotation %d:\n", i);
        printf("Width: %d, Height: %d\n", p->rotations[i].width, p->rotations[i].height);
        for (int j = 0; j < p->rotations[i].width; j++) {
            printf("Col: %d, Elevation: %d, Concave: %d, Solid: %d\n",
                   j, 
                   p->rotations[i].elevation[j], 
                   p->rotations[i].concave[j], 
                   p->rotations[i].solid[j]);
        }
        for (int j = p->rotations[i].height - 1; j >= 0; j--) {
            for (int k = 0; k < p->rotations[i].width; k++) {
                if (p->rotations[i].shape[j] & (1 << k)) {
                    printf("%c", FULL_CHAR);
                } else {
                    printf("%c", EMPTY_CHAR);
                }
            }
            printf("\n");
        }
        printf("\n");
    }
}


int get_holes(struct tetris *t) {
    int holes = 0;
    for (int i = 0; i < ROW; i++) {
        uint16_t row = t->board[i];
        for (int j = 0; j < COL; j++) {
            if ((row & (1 << j)) == 0) { // 如果该位置为空
                holes++;
            }
        }
    }
    return holes;
}