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

static void calc_shape_mask(struct piece *p, int rot) {
    for (int i = 0; i < p->rotations[rot].height; i++) {
        uint16_t m = p->rotations[rot].shape[i] << 3;
        for (int j = 0; j < COL; j++) {
            p->rotations[rot].mask[j][i] = m;
            m = m << 1;
        }
    }
}

static void init_pieces() {
    for (int i = 0; i < PIECE_TYPES; i++) {
        for (int j = 0; j < MAX_ROTATIONS; j++) {
            calc_shape_mask(&pieces[i], j);
            for (int k = 0; k < MAX_BRICK_WIDTH; k++) {
                pieces[i].rotations[j].solid[k] = 
                    pieces[i].rotations[j].elevation[k] - pieces[i].rotations[j].concave[k];
            }
        }
    }
}


static inline int calc_landing_row(struct tetris *t, struct piece *p, int rot, int col) {
    int row = 0;  // 棋盘的最底行
    for (int i = 0; i < p->rotations[rot].width; i++) {
        int r =  t->col_height[col + i] - p->rotations[rot].concave[i];
        if (r > row) {
            row = r;
        }
    }

    return row;
}


void init_tetris(struct tetris *t) {
    srand(time(NULL)); // 初始化随机数种子
    init_pieces();

    for (int i = 0; i < ROW; i++) {
        t->board[i] = EMPTY_ROW; // 初始化棋盘为空
    }
}

int place_piece(struct tetris *t, struct piece *p, int rotation, int col) {
    t->landing_row = calc_landing_row(t, p, rotation, col);

    return 0;
}