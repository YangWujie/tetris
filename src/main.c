#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "tetris.h"
#include "print_utils.h"

// 得分规则
const int SCORE_TABLE[] = {0, 100, 300, 500, 800};

int show_help = 0;
int auto_mode = 0;
int interactive_mode = 0;
int step_mode = 0;
int twostep_mode = 0;
int beam_mode = 0;
int level = 0;

void print_help(const char *prog) {
    printf("用法: %s [选项] 激进等级\n", prog);
    printf("选项:\n");
    printf("  -h, --help           显示帮助信息\n");
    printf("  -a, --auto           自动模式\n");
    printf("  -i, --interactive    交互模式\n");
    printf("  -s, --step           单步模式\n");
    printf("  -t, --twostep        两步模式\n");
    printf("  -b, --beam           BEAM模式\n");
    printf("激进等级: 1-5 的整数\n");
}

void play_game() {
    struct tetris t;
    init_tetris(&t);
    int step = 0;
    int total_score = 0;
    int total_lines = 0;
    int curr_piece = rand() % PIECE_TYPES;
    int next_piece = rand() % PIECE_TYPES;
    clock_t start_time = clock();
    while (1) {
        int best_rotation = 0, best_col = 0, landing_row = 0;
        if (t.max_height < 7)
            select_best_move_with_next_beam(&t, curr_piece, next_piece, &best_rotation, &best_col);
        else
            select_best_move_with_next_beam_sampleSZ(&t, curr_piece, next_piece, &best_rotation, &best_col);
        if (interactive_mode) {
            print_pieces_side_by_side(best_col - 1, &pieces[curr_piece], best_rotation, &pieces[next_piece], 0);
            print_board(&t);
            printf("Current score: %d, Total lines: %d\n", total_score, total_lines);
            getchar();
        }
        int lines = place_piece(&t, &pieces[curr_piece], best_rotation, best_col, &landing_row);
        total_score += SCORE_TABLE[lines];
        total_lines += lines;
        step++;
        if (t.max_height > 16 || step >= 1000000) {
            printf("Game over at step %d!\n", step);
            printf("Final score: %d, Total lines: %d\n", total_score, total_lines);
            break;
            }
            curr_piece = next_piece;
            next_piece = rand() % PIECE_TYPES;
        }
    clock_t end_time = clock();
    double elapsed = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Total elapsed time: %.3f seconds\n", elapsed);
}

int main(int argc, char *argv[]) {
    int opt;
    int option_index = 0;
 
    static struct option long_options[] = {
        {"help",        no_argument, 0, 'h'},
        {"auto",        no_argument, 0, 'a'},
        {"interactive", no_argument, 0, 'i'},
        {"step",        no_argument, 0, 's'},
        {"twostep",     no_argument, 0, 't'},
        {"beam",        no_argument, 0, 'b'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "haistb", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h': show_help = 1; break;
            case 'a': auto_mode = 1; break;
            case 'i': interactive_mode = 1; break;
            case 's': step_mode = 1; break;
            case 't': twostep_mode = 1; break;
            case 'b': beam_mode = 1; break;
            default:
                print_help(argv[0]);
                return 1;
        }
    }

    // 互斥逻辑
    if (show_help) {
        if (auto_mode || interactive_mode || step_mode || twostep_mode || beam_mode || optind < argc) {
            fprintf(stderr, "--help 不能与其他选项或参数同时使用\n");
            return 1;
        }
        print_help(argv[0]);
        return 0;
    }
    if (auto_mode && interactive_mode) {
        fprintf(stderr, "自动模式和交互模式不能同时指定\n");
        return 1;
    }
    if ((step_mode + twostep_mode + beam_mode) > 1) {
        fprintf(stderr, "单步、两步、BEAM模式三者互斥\n");
        return 1;
    }

    // 检查激进等级参数（可选，默认1）
    if (optind < argc) {
        level = atoi(argv[optind]);
        if (level < 1 || level > 5) {
            fprintf(stderr, "激进等级必须为1-5之间的整数\n");
            return 1;
        }
    } else {
        level = 1;
    }

    // 如果自动模式和交互模式都未指定，默认交互模式
    if (!auto_mode && !interactive_mode) {
        interactive_mode = 1;
    }

    // 如果单步模式、两步模式以及BEAM模式均未指定，则默认BEAM模式
    if (!step_mode && !twostep_mode && !beam_mode) {
        beam_mode = 1;
    }

    // 这里可以根据模式和level调用不同的游戏逻辑
    play_game();
    return 0;
}