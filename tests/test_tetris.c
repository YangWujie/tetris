#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "../src/tetris.h"

void print_piece(struct piece *p) {
    for (int i = 0; i < p->count; i++) {
        printf("Rotation %d:\n", i);
        printf("Width: %d, Height: %d\n", p->rotations[i].width, p->rotations[i].height);
        for (int j = 0; j < p->rotations[i].width; j++) {
            printf("Col: %d, vstart: %d, vend: %d, vspan: %d\n",
                   j, 
                   p->rotations[i].vstart[j], 
                   p->rotations[i].vend[j], 
                   p->rotations[i].vspan[j]);
        }
        for (int j = 0; j < p->rotations[i].height; j++) {
            printf("Row: %d, hstart: %d, hend: %d, hspan: %d\n",
                   j, 
                   p->rotations[i].hstart[j], 
                   p->rotations[i].hend[j], 
                   p->rotations[i].hspan[j]);
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


void print_board(struct tetris *t) {
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

void test_init() {
    struct tetris tetris;
    init_tetris(&tetris);

    for (int i = 0; i < ROW; i++) {
        CU_ASSERT_EQUAL(tetris.board[i], EMPTY_ROW);
    }
    CU_ASSERT_EQUAL(tetris.board[-1], FULL_ROW);

    for(int i = 0; i < 16; i++) {
        CU_ASSERT_EQUAL(tetris.col_height[i], 0);
    }

    print_piece(&pieces[0]);
    print_piece(&pieces[1]);
    print_piece(&pieces[2]);
    print_piece(&pieces[3]);
    print_piece(&pieces[4]);
    print_piece(&pieces[5]);
    print_piece(&pieces[6]);
}

void test_place_piece1() {
    struct tetris tetris;
    init_tetris(&tetris);

    int col = 0;
    print_piece(&pieces[1]);
    place_piece(&tetris, &pieces[0], 0, 3);
    CU_ASSERT_EQUAL(tetris.landing_row, 0);
    CU_ASSERT_EQUAL(tetris.col_height[3], 1);
    CU_ASSERT_EQUAL(tetris.col_height[4], 1);
    CU_ASSERT_EQUAL(tetris.col_height[5], 1);
    CU_ASSERT_EQUAL(tetris.col_height[6], 1);

    print_piece(&pieces[1]);

    place_piece(&tetris, &pieces[1], 1, 4);
    CU_ASSERT_EQUAL(tetris.landing_row, 1);
    CU_ASSERT_EQUAL(tetris.max_height, 4);
    CU_ASSERT_EQUAL(tetris.wells, 3);
    CU_ASSERT_EQUAL(tetris.holes, 1);
    CU_ASSERT_EQUAL(tetris.row_transitions, 3);
    CU_ASSERT_EQUAL(tetris.col_transitions, 1);
    print_board(&tetris);
    printf("wells: %d\n", tetris.wells);
    printf("holes: %d\n", tetris.holes);
    printf("col_transitions: %d\n", tetris.col_transitions);
}

int main() {
    CU_initialize_registry();
    CU_pSuite suite = CU_add_suite("Tetris Test Suite", NULL, NULL);
    CU_add_test(suite, "test_init", test_init);
    CU_add_test(suite, "test_place_piece", test_place_piece1);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return 0;
}
