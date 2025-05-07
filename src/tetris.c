#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <string.h>
#include <stdbool.h>
#include "tetris.h"
#include "print_utils.h"

const char piece_names[PIECE_TYPES] = {'I', 'T', 'O', 'J', 'L', 'S', 'Z'};

static const int64_t LANDING_HEIGHT = (int64_t) (WEIGHT_LANDING_HEIGHT * 10000);
static const int64_t HOLES = (int64_t) (WEIGHT_HOLES * 10000);
static const int64_t ROW_TRANSITIONS = (int64_t) (WEIGHT_ROW_TRANSITIONS * 10000);
static const int64_t COL_TRANSITIONS = (int64_t) (WEIGHT_COLUMN_TRANSITIONS * 10000);
static const int64_t WELL_SUMS = (int64_t) (WEIGHT_WELL_SUMS * 10000);
static const int64_t ROWS_ELIMINATED = (int64_t) (WEIGHT_ROWS_ELIMINATED * 10000);


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
// 棋盘有效状态范围是 0---19 行，COL_SHIFT---COL_SHIFT+COL 列
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

static inline int get_center_of_gravity(const struct rotation *rot, int landing_row) {
    int cg = 0;
    for (int i = 0; i < rot->height; i++) {
        cg += rot->hspan[i] * (landing_row + i);
    }
    cg >>= 2;  // 除以4
    return cg;
}

void init_tetris(struct tetris *t) {
    srand(time(NULL)); // 初始化随机数种子
    init_pieces();

    memset(t, 0, sizeof(struct tetris)); // 初始化棋盘
    for (int i = 0; i < ROW; i++) {
        t->board[i] = EMPTY_ROW; // 初始化棋盘为空
    }
    t->pad0 = FULL_ROW;    // 防止用-1作为下标访问board时越界
}

int get_piece_name(int piece) {
    if (piece < 0 || piece >= PIECE_TYPES) {
        return -1;
    }
    return piece_names[piece];
}

void place_piece(struct tetris *t, const struct piece *p, int rotation, int col) {
    t->rows_eliminated = 0;
    const struct rotation *rot = &p->rotations[rotation];
    t->landing_row = get_landing_row(t, rot, col);
    if (t->landing_row + rot->height > ROW) {
        t->landing_row = -1; // 超出棋盘范围
        return;
    }

    for(int i = 0; i < rot->height; i++) {
        int r = t->landing_row + i;
        int cs = col + rot->hstart[i];
        int ce = cs + rot->hspan[i];
        t->board[r] |= (rot->shape[i] << col);
        //   s1 s2 XXX s3 s4    
        int s1 = get_status(t, r, cs - 2);
        int s2 = get_status(t, r, cs - 1);
        int s3 = get_status(t, r, ce);
        int s4 = get_status(t, r, ce + 1);
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
        if (s) {  // 落点下方有方块
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
            t->rows_eliminated++;

            for (int j = COL_SHIFT; j < COL + COL_SHIFT; j++) {
                t->col_height[j]--;
                int up = get_status(t, r, j);
                if (up) { // 被消除的块上面有块，什么也不用做
                    continue;
                }
                int k = r - 1;
                int down = get_status(t, k, j);
                if (down == 0) {
                    t->col_transitions--;
                }
                if (r == t->col_height[j]) {  // 消除的是该列最顶上方块
                    while (get_status(t, k, j) == 0) { // 有洞
                        t->col_height[j]--;
                        t->holes--;
                        k--;
                    }
                }
            }
        }
    }
}

int64_t evaluate_board(const struct tetris *t) {
    if (t->landing_row == -1) {
        return INT64_MIN;
    }

    int64_t score = 0;
    if (t->rows_eliminated == 1 && t->max_height < 11) {
        score -= (int64_t) 12 * ROWS_ELIMINATED;
    }
    else {
        score += (int64_t) 2 * t->rows_eliminated * ROWS_ELIMINATED;
    }
    struct rotation *rot = &pieces[t->piece].rotations[t->rotation];
    score += (int64_t )get_center_of_gravity(rot, t->landing_row) * LANDING_HEIGHT;
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
            struct tetris temp_tetris = *t;
            place_piece(&temp_tetris, &pieces[piece_index], j, col);
            int64_t score = evaluate_board(&temp_tetris);
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
            struct tetris temp_tetris = *t;
            place_piece(&temp_tetris, &pieces[curr_piece_index], j, col);
            int64_t bonus = (int64_t) get_center_of_gravity(rot, t->landing_row) * LANDING_HEIGHT / 4;

            int64_t next_best = INT64_MIN;
            for (int nj = 0; nj < pieces[next_piece_index].count; nj++) {
                const struct rotation *nrot = &pieces[next_piece_index].rotations[nj];
                for (int ncol = COL_SHIFT; ncol <= COL_SHIFT + COL - nrot->width; ncol++) {
                    struct tetris next_tetris = temp_tetris;
                    place_piece(&next_tetris, &pieces[next_piece_index], nj, ncol);
                    int64_t score = evaluate_board(&next_tetris);
                    if (score > next_best) {
                        next_best = score;
                    }
                }
            }
            int64_t total_score = bonus + next_best;
            if (total_score > best_score) {
                best_score = total_score;
                *best_rotation = j;
                *best_col = col;
            }
        }
    }
}


// 辅助函数：生成 beam 节点
static int generate_beam(struct tetris *t, int piece_index, struct BeamNode *beam, int beam_width) {
    int beam_size = 0;
    for (int i = 0; i < pieces[piece_index].count; i++) {
        const struct rotation *rot = &pieces[piece_index].rotations[i];
        for (int col = COL_SHIFT; col <= COL_SHIFT + COL - rot->width; col++) {
            struct tetris temp_tetris = *t;
            place_piece(&temp_tetris, &pieces[piece_index], i, col);
            int64_t curr_score = evaluate_board(&temp_tetris);

            // 插入 beam 数组，按分数从大到小排序，保留前 beam_width 个
            int insert_pos = beam_size;
            while (insert_pos > 0 && beam[insert_pos - 1].score < curr_score) {
                if (insert_pos < beam_width) beam[insert_pos] = beam[insert_pos - 1];
                insert_pos--;
            }
            if (insert_pos < beam_width) {
                beam[insert_pos].t = temp_tetris;
                beam[insert_pos].rotation = i;
                beam[insert_pos].col = col;
                beam[insert_pos].score = curr_score;
                if (beam_size < beam_width) beam_size++;
            }
        }
    }
    return beam_size;
}

void select_best_move_with_next_beam(
    struct tetris *t,
    int curr_piece_index,
    int next_piece_index,
    int *best_rotation,
    int *best_col
) {
    struct BeamNode beam[BEAM_WIDTH];
    int beam_size = 0;

    // 1. 枚举所有当前方块的落子方式，保留前BEAM_WIDTH个
    beam_size = generate_beam(t, curr_piece_index, beam, BEAM_WIDTH);

    // 2. 对每个beam节点，枚举下一个方块所有落子，取最优
    int64_t best_total_score = INT64_MIN;
    for (int i = 0; i < beam_size; i++) {
        int64_t next_best = INT64_MIN;
        for (int nj = 0; nj < pieces[next_piece_index].count; nj++) {
            const struct rotation *nrot = &pieces[next_piece_index].rotations[nj];
            for (int ncol = COL_SHIFT; ncol <= COL_SHIFT + COL - nrot->width; ncol++) {
                struct tetris next_tetris = beam[i].t;
                place_piece(&next_tetris, &pieces[next_piece_index], nj, ncol);
                int64_t score = evaluate_board(&next_tetris);
                if (score > next_best) {
                    next_best = score;
                }
            }
        }
        int64_t bonus = (int64_t) beam[i].t.landing_row * LANDING_HEIGHT;
        int64_t total_score = bonus + next_best;
        if (total_score > best_total_score) {
            best_total_score = total_score;
            *best_rotation = beam[i].rotation;
            *best_col = beam[i].col;
        }
    }
}

// 辅助函数：采样第三步的 S 和 Z 型方块
static int64_t sample_third_step(struct tetris *t, int *third_pieces, int piece_count) {
    int64_t worst_best_score = INT64_MAX;  // 记录最差情况的最佳分数
    
    // 对每种方块（S和Z）
    for (int tp = 0; tp < piece_count; tp++) {
        int64_t best_score = INT64_MIN;  // 当前方块的最佳分数
        int third_piece_index = third_pieces[tp];
        
        // 枚举该方块的所有可能落子位置
        for (int tj = 0; tj < pieces[third_piece_index].count; tj++) {
            const struct rotation *trot = &pieces[third_piece_index].rotations[tj];
            for (int tcol = COL_SHIFT; tcol <= COL_SHIFT + COL - trot->width; tcol++) {
                struct tetris third_tetris = *t;
                place_piece(&third_tetris, &pieces[third_piece_index], tj, tcol);
                int64_t score = evaluate_board(&third_tetris);
                
                // 更新当前方块的最佳分数
                if (score > best_score) {
                    best_score = score;
                }
            }
        }
        
        // 更新最差情况的最佳分数
        if (best_score < worst_best_score) {
            worst_best_score = best_score;
        }
    }
    
    return worst_best_score;
}

void select_best_move_with_next_beam_sampleSZ(
    struct tetris *t,
    int curr_piece_index,
    int next_piece_index,
    int *best_rotation,
    int *best_col
) {
    struct BeamNode beam[BEAM_WIDTH];
    int beam_size = 0;

    // 1. 枚举所有当前方块的落子方式，保留前 BEAM_WIDTH 个
    beam_size = generate_beam(t, curr_piece_index, beam, BEAM_WIDTH);

    // 2. 对每个 beam 节点，枚举下一个方块的所有落子方式，保留前 BEAM_WIDTH 个
    struct BeamNode next_beam[BEAM_WIDTH];
    int next_beam_size = 0;
    int64_t best_total_score = INT64_MIN;

    for (int i = 0; i < beam_size; i++) {
        next_beam_size = generate_beam(&beam[i].t, next_piece_index, next_beam, BEAM_WIDTH);

        // 3. 第三步只采样 S 和 Z 型 piece（piece_index = 5 和 6）
        int third_pieces[2] = {5, 6};
        for (int j = 0; j < next_beam_size; j++) {
            int64_t third = sample_third_step(&next_beam[j].t, third_pieces, 2);
            int64_t bonus = (int64_t) (beam[i].t.landing_row + next_beam[j].t.landing_row) * LANDING_HEIGHT;
            int64_t total_score = bonus + third;

            // 更新最佳分数和第一个方块的落子位置
            if (total_score > best_total_score) {
                best_total_score = total_score;
                *best_rotation = beam[i].rotation; // 返回第一个方块的 rotation
                *best_col = beam[i].col;           // 返回第一个方块的 col
            }
        }
    }
}