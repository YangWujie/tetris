#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <string.h>
#include <stdbool.h>
#include "tetris.h"

// 得分规则
const int SCORE_TABLE[] = {0, 100, 300, 500, 800};
const char piece_names[PIECE_TYPES] = {'I', 'T', 'O', 'J', 'L', 'S', 'Z'};

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

static inline int get_landing_row(struct tetris *t, const struct rotation *rot, int col) {
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

int get_piece_name(int piece) {
    if (piece < 0 || piece >= PIECE_TYPES) {
        return -1;
    }
    return piece_names[piece];
}

int place_piece(struct tetris *t, const struct piece *p, int rotation, int col) {
    int rows_eliminated = 0;
    const struct rotation *rot = &p->rotations[rotation];
    t->landing_row = get_landing_row(t, rot, col);

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
        int s = get_status(t, r, col + i);
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
            rows_eliminated++;

            for (int j = COL_SHIFT; j < COL + COL_SHIFT; j++) {
                t->col_height[j]--;
                int s1 = get_status(t, r, j);
                if (s1) {
                    continue;
                }
                int k = r - 1;
                while (get_status(t, k, j) == 0) {
                    t->col_height[j]--;
                    t->holes--;
                    k--;
                }
                if (k != r - 1) {
                    t->col_transitions--;
                }
            }
        }
    }

    return rows_eliminated;
}

int64_t evaluate_board(const struct tetris *t, int rows_eliminated) {
    static const int64_t LANDING_HEIGHT = (int64_t) WEIGHT_LANDING_HEIGHT * 10000;
    static const int64_t HOLES = (int64_t) WEIGHT_HOLES * 10000;
    static const int64_t ROW_TRANSITIONS = (int64_t) WEIGHT_ROW_TRANSITIONS * 20000;
    static const int64_t COL_TRANSITIONS = (int64_t) WEIGHT_COLUMN_TRANSITIONS * 20000;
    static const int64_t WELL_SUMS = (int64_t) WEIGHT_WELL_SUMS * 10000;
    static const int64_t ROWS_ELIMINATED = (int64_t) WEIGHT_ROWS_ELIMINATED * 10000;

    int64_t score = 0;
    score += (int64_t) rows_eliminated * ROWS_ELIMINATED;
    score += (int64_t )t->landing_row * LANDING_HEIGHT;
    score += (int64_t) t->col_transitions * COL_TRANSITIONS;
    score += (int64_t) t->row_transitions * ROW_TRANSITIONS; 
    score += (int64_t) t->wells * WELL_SUMS;
    score += (int64_t) t->holes * HOLES;

    return score;
}

void select_best_move(struct tetris *t, int piece_index, int *best_rotation, int *best_col) {
    int64_t best_score = INT64_MIN;
    for (int j = 0; j < pieces[piece_index].count; j++) {
        const struct rotation *rot = &pieces[piece_index].rotations[j];
        for (int col = COL_SHIFT; col < COL_SHIFT + COL - rot->width + 1; col++) {
            struct tetris temp_tetris;
            memcpy(&temp_tetris, t, sizeof(struct tetris));
            int rows_eliminated = place_piece(&temp_tetris, &pieces[piece_index], j, col);
            int64_t score = evaluate_board(&temp_tetris, rows_eliminated);
            if (score > best_score) {
                best_score = score;
                *best_rotation = j;
                *best_col = col;
            }
        }
    }
}

void select_best_move_with_next(
    struct tetris *t,
    int curr_piece_index,
    int next_piece_index,
    int *best_rotation,
    int *best_col
) {
    int64_t best_score = INT64_MIN;
    for (int j = 0; j < pieces[curr_piece_index].count; j++) {
        const struct rotation *rot = &pieces[curr_piece_index].rotations[j];
        for (int col = COL_SHIFT; col <= COL_SHIFT + COL - rot->width; col++) {
            struct tetris temp_tetris;
            memcpy(&temp_tetris, t, sizeof(struct tetris));
            int lines_cleared = place_piece(&temp_tetris, &pieces[curr_piece_index], j, col);
            int64_t curr_score = evaluate_board(&temp_tetris, lines_cleared);

            int64_t next_best = INT64_MIN;
            for (int nj = 0; nj < pieces[next_piece_index].count; nj++) {
                const struct rotation *nrot = &pieces[next_piece_index].rotations[nj];
                for (int ncol = COL_SHIFT; ncol <= COL_SHIFT + COL - nrot->width; ncol++) {
                    struct tetris next_tetris;
                    memcpy(&next_tetris, &temp_tetris, sizeof(struct tetris));
                    int nlines_cleared = place_piece(&next_tetris, &pieces[next_piece_index], nj, ncol);
                    int64_t score = evaluate_board(&next_tetris, nlines_cleared);
                    if (score > next_best) {
                        next_best = score;
                    }
                }
            }
            int64_t total_score = curr_score + next_best;
            if (total_score > best_score) {
                best_score = total_score;
                *best_rotation = j;
                *best_col = col;
            }
        }
    }
}

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

void play_game_with_score() {
    struct tetris t;
    init_tetris(&t);
    int step = 0;
    int total_score = 0;
    int total_lines = 0;
    int curr_piece = rand() % PIECE_TYPES;
    int next_piece = rand() % PIECE_TYPES;
    while (1) {
        int best_rotation = 0, best_col = 0;
        select_best_move_with_next(&t, curr_piece, next_piece, &best_rotation, &best_col);
        int lines = place_piece(&t, &pieces[curr_piece], best_rotation, best_col);
        total_score += SCORE_TABLE[lines];
        total_lines += lines;
        step++;
        printf("Step %d: Piece %c\n", step, piece_names[curr_piece]);
        print_board(&t);
        printf("Current score: %d, Total lines: %d\n", total_score, total_lines);
        if (t.max_height > 18) {
            printf("Game over at step %d!\n", step);
            printf("Final score: %d, Total lines: %d\n", total_score, total_lines);
            break;
        }
        curr_piece = next_piece;
        next_piece = rand() % PIECE_TYPES;
    }
}