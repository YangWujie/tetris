#ifndef TETRIS_H
#define TETRIS_H

#include <stdint.h>

#define ROW 20
#define COL 10
#define PIECE_TYPES 7
#define MAX_ROTATIONS   4
#define MAX_BRICK_WIDTH 4
#define EMPTY_CHAR      '.'
#define FULL_CHAR       'X'

#define BEAM_WIDTH 6

// Pierre Dellacherie 算法评分权重
#define WEIGHT_LANDING_HEIGHT     (-4.500158825082766)
#define WEIGHT_ROWS_ELIMINATED    (3.4181268101392694)
#define WEIGHT_ROW_TRANSITIONS    (-3.2178882868487753)
#define WEIGHT_COLUMN_TRANSITIONS (-9.348695305445199)
#define WEIGHT_HOLES              (-7.899265427351652)
#define WEIGHT_WELL_SUMS          (-3.3855972247263626)

#define EMPTY_ROW   0B1111100000000001
#define FULL_ROW    0B1111111111111111
#define COL_SHIFT   1

// 俄罗斯方块棋盘定义
// 为了提高程序运行速度，用一个二进制位表示一个方格状态：0 表示空，1 表示有方块
// 这里使用 uint16_t 的 COL_SHIFT 至 COL_SHIFT+COL 位表示一行的状态，
// 其余空闲位用 1 填充
// 0 是底部行， 19 是顶部行
struct tetris {
    uint16_t pad0;
    uint16_t board[ROW];
    int8_t  col_height[COL + 2*COL_SHIFT];    // 每列的高度
    int8_t  max_height;        // 最高行
    int8_t  holes;             // 当前空洞数
    int8_t  row_transitions;   // 行转换数
    int8_t  col_transitions;   // 列转换数
    int8_t  wells;             // 井深度
    int8_t  piece;             // 当前方块类型
    int8_t  landing_row;       // 当前方块落点
    int8_t  rotation;          // 当前方块旋转角度
    int8_t  rows_eliminated;   // 当前方块消除的行数
    int8_t  reserved;
};

struct rotation {
    int width;
    int height;
    uint16_t shape[MAX_BRICK_WIDTH];
    int hstart[MAX_BRICK_WIDTH];
    int hend[MAX_BRICK_WIDTH];
    int hspan[MAX_BRICK_WIDTH];
    int vstart[MAX_BRICK_WIDTH];
    int vend[MAX_BRICK_WIDTH];
    int vspan[MAX_BRICK_WIDTH];
};

struct piece {
    int count;
    struct rotation rotations[MAX_ROTATIONS];
};

struct BeamNode {
    struct tetris t;
    int rotation;
    int col;
    int64_t score;
    int landing_row;
    int lines_cleared;
};

void init_tetris(struct tetris *t);
void select_best_move(struct tetris *t, int piece_index, int *best_rotation, int *best_col);
void select_best_move_with_next_beam(
    struct tetris *t,
    int curr_piece_index,
    int next_piece_index,
    int *best_rotation,
    int *best_col
);

int  place_piece(struct tetris *t, const struct piece *p, int rotation, int col, int *landing_row);

extern struct piece pieces[];

#endif // TETRIS_H