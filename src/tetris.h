#include <stdint.h>

#define ROW 20
#define COL 10
#define PIECE_TYPES 7
#define MAX_ROTATIONS   4
#define MAX_BRICK_WIDTH 4
#define EMPTY_CHAR      '.'
#define FULL_CHAR       'X'


// Pierre Dellacherie 算法评分权重
#define WEIGHT_LANDING_HEIGHT   (-4.500158825082766)
#define WEIGHT_ROWS_ELIMINATED  (3.4181268101392694)
#define WEIGHT_ROW_TRANSITIONS  (-3.2178882868487753)
#define WEIGHT_COLUMN_TRANSITIONS (-9.348695305445199)
#define WEIGHT_HOLES            (-7.899265427351652)
#define WEIGHT_WELL_SUMS        (-3.3855972247263626)

#define EMPTY_ROW   0B1110000000000111
#define FULL_ROW    0B1111111111111111
#define COL_SHIFT   3

// 俄罗斯方块棋盘定义
// 为了提高程序运行速度，用一个二进制位表示一个方格状态：0 表示空，1 表示有方块
// 这里使用 uint16_t 的 3-12 位表示一行的状态，其余空闲位用 1 填充
// 0 行是底部， 19 行是顶部
struct tetris {
    uint16_t pad;
    uint16_t board[ROW];
    uint8_t  col_height[16];    // 每列的高度
    uint8_t  max_height;        // 最高行
    uint8_t  landing_row;       // 最新一块方块的落点
    uint8_t  holes;             // 当前空洞数
    uint8_t  row_transitions;   // 行转换数
    uint8_t  col_transitions;   // 列转换数
    uint8_t  wells;             // 井深度
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

void init_tetris(struct tetris *t);
void print_piece(struct piece *p);
int place_piece(struct tetris *t, struct piece *p, int rotation, int col);

extern struct tetris tetris;
extern struct piece pieces[];