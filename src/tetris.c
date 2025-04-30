#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <string.h>
#include "tetris.h"

// 得分规则
const int SCORE_TABLE[] = {0, 100, 300, 500, 800};

// 俄罗斯方块形状定义
struct piece pieces[] = {
    // I
    {
        2,  // rotation count
        {
            {
                4,  // width
                1,  // height
                { 0b1111 }  // shape
            },
            {
                1,  // width
                4,  // height
                { 0b1,
                  0b1,
                  0b1,
                  0b1
                }
            }
        }
    },

    // T
    {
        4,  // rotation count
        {
            {
                3,  // width
                2,  // height
                { 0b111,
                  0b010
                }
            },
            {
                2,  // width
                3,  // height
                { 0b01,
                  0b11,
                  0b01
                }
            },
            {
                3,  // width
                2,  // height
                { 0b010,
                  0b111
                }
            },
            {
                2,   // width
                3,   // height
                { 0b10,
                  0b11,
                  0b10
                }
            }
        }
    },

    // O
    {
        1,  // rotation count
        {
            {
                2,  // width
                2,  // height
                { 0b11,
                  0b11
                }
            }
        }
    },

    // J
    {
        4,  // rotation count
        {
            {
                3,  // width
                2,  // height
                { 0b111,
                  0b001
                }
            },
            {
                2,   // width
                3,   // height
                { 0b01,
                  0b01,
                  0b11
                }
            },
            {
                3,   // width
                2,   // height
                { 0b100,
                  0b111
                }
            },
            {
                2,   // width
                3,   // height
                { 0b11,
                  0b10,
                  0b10
                }
            }
        }
    },

    // L
    {
        4,  // rotation count
        {
            {
                3,  // width
                2,  // height
                { 0b111,
                  0b100
                }
            },
            {
                2,   // width
                3,   // height
                { 0b11,
                  0b01,
                  0b01
                }
            },
            {
                3,   // width
                2,   // height
                { 0b001,
                  0b111
                }
            },
            {
                2,   // width
                3,   // height
                { 0b10,
                  0b10,
                  0b11
                }
            }
        }
    },

    // S
    {
        2,  // rotation count
        {
            {
                3,  // width
                2,  // height
                { 0b011,
                  0b110
                }
            },
            {
                2,   // width
                3,   // height
                { 0b10,
                  0b11,
                  0b01
                }
            }
        }
    },

    // Z
    {
        2,  // rotation count
        {
            {
                3,  // width
                2,  // height
                { 0b110,
                  0b011
                }
            },
            {
                2,   // width
                3,   // height
                { 0b01,
                  0b11,
                  0b10
                }
            }
        }
    }
};


struct tetris tetris;


static void init_pieces() {
    for (int i = 0; i < PIECE_TYPES; i++) {
        for (int j = 0; j < MAX_ROTATIONS; j++) {
            struct rotation *rot = &pieces[i].rotations[j];
            for (int k = 0; k < rot->height; k++) {
                uint16_t s = rot->shape[k];
                rot->hstart[k] = 0;
                while ((s & 1) == 0) {
                    rot->hstart[k]++;
                    s = s >> 1;
                }
                rot->hend[k] = rot->hstart[k];
                while ((s & 1) == 1) {
                    rot->hend[k]++;
                    s = s >> 1;
                }
                rot->hspan[k] = rot->hend[k] - rot->hstart[k];
            }
            
            for (int k = 0; k < rot->width; k++) {
                int v = 0;
                rot->vstart[k] = 0;
                while ((rot->shape[v] & (1 << k)) == 0) {
                    rot->vstart[k]++;
                    v++;
                }
                rot->vend[k] = v;
                while ((v < rot->height) && (((rot->shape[v] & (1 << k))) != 0)) {
                    rot->vend[k]++;
                    v++;
                }
                rot->vspan[k] = rot->vend[k] - rot->vstart[k];
            }
        }
    }
}

// 调用者负责检查行列是否越界
// col:  0---15
// row: -1---19
// 坐标范围看起来有点奇怪，主要是为了避免边界条件判断而在棋盘周围做了填充
// 棋盘有效状态范围是 0---19 行，3---12 列
static inline int get_status(struct tetris *t, int row, int col) {
    return (t->board[row] & (1 << col)); // 检查该位置是否有方块
}

static inline int calc_landing_row(struct tetris *t, struct rotation *rot, int col) {
    int row = 0;  // 棋盘的最底行
    for (int i = 0; i < rot->width; i++) {
        int r =  t->col_height[col + i] - rot->vstart[i];
        if (r > row) {
            row = r;
        }
    }

    return row;
}

void init_tetris(struct tetris *t) {
    srand(time(NULL)); // 初始化随机数种子
    init_pieces();

    memset(t, 0, sizeof(struct tetris)); // 初始化棋盘
    for (int i = 0; i < ROW; i++) {
        t->board[i] = EMPTY_ROW; // 初始化棋盘为空
    }
    t->pad = FULL_ROW;    // 防止用-1作为下标访问board时越界
}

int place_piece(struct tetris *t, struct piece *p, int rotation, int col) {
    struct rotation *rot = &p->rotations[rotation];
    t->landing_row = calc_landing_row(t, rot, col);

    for(int i = 0; i < rot->height; i++) {
        int r = t->landing_row + i;
        int c = col + rot->hstart[i];
        t->board[r] |= (rot->shape[i] << col);
        //   s1 s2 XXX s3 s4
        int s1 = get_status(t, r, c - 2);
        int s2 = get_status(t, r, c - 1);
        c += rot->hspan[i];
        int s3 = get_status(t, r, c);
        int s4 = get_status(t, r, c + 1);
        if (s2 && s3) {
            if (t->board[r] != FULL_ROW) {
                t->row_transitions--;
            }
            if (rot->hspan[i] == 1) {
                t->wells--;
            }    
        }
        if (!s2 && !s3) {
            t->row_transitions++;
        }
        if (s1 && !s2) {
            t->wells++;
        }
        if (!s3 && s4) {
            t->wells++;
        }
    }

    for (int i = 0; i < rot->width; i++) {
        t->col_height[col + i] = t->landing_row + rot->vend[i];
        if (t->col_height[col + i] > t->max_height) {
            t->max_height = t->col_height[col + i];
        }
        int r = t->landing_row + rot->vstart[i] - 1;
        printf("rot->vstart[%d]: %d, col: %d, r: %d\n", i, rot->vstart[i], col + i, r);
        int s = get_status(t, r, col + i);
        printf("col: %d, row: %d, s: %d\n", col + i, r, s);
        if (s) {
            continue;
        }

        t->col_transitions++;
        t->holes++;
        while (get_status(t, --r, col + i) == 0) {
            t->holes++;
        }
    }

    for (int i = rot->height - 1; i >= 0; i--) {
        int r = t->landing_row + i;
        if (t->board[r] == FULL_ROW) {
            for (int j = r; j < t->max_height - 1; j++) {
                t->board[j] = t->board[j + 1];
            }
            t->board[t->max_height - 1] = EMPTY_ROW;
            t->max_height--;

            for (int j = COL_SHIFT; j < COL + COL_SHIFT; j++) {
                t->col_height[j]--;
                int s1 = get_status(t, r, j);
                if (s1) {
                    continue;
                }
                int s2 = get_status(t, r - 1, j);
                if (s2) {
                    continue;
                }
                t->holes--;
                int k = r - 2;
                while (get_status(t, k, j) == 0) {
                    t->holes--;
                    k--;
                }
                t->col_transitions--;
            }
        }
    }

    return 0;
}