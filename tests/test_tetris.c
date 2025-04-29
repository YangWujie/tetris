#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "../src/tetris.h"

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


void test_init() {
    struct tetris tetris;
    init_tetris(&tetris);

    for (int i = 0; i < ROW; i++) {
        CU_ASSERT_EQUAL(tetris.board[i], EMPTY_ROW);
    }

    print_piece(&pieces[0]);
    print_piece(&pieces[1]);
    print_piece(&pieces[2]);
    print_piece(&pieces[3]);
    print_piece(&pieces[4]);
    print_piece(&pieces[5]);
    print_piece(&pieces[6]);

    for (int i = 0; i < PIECE_TYPES; i++) {
        for (int j = 0; j < MAX_ROTATIONS; j++) {
            for (int k = 0; k < MAX_BRICK_WIDTH; k++) {
                CU_ASSERT_EQUAL(pieces[i].rotations[j].solid[k], 
                                pieces[i].rotations[j].elevation[k] - pieces[i].rotations[j].concave[k]);
            }
        }
    }
}

int main() { 
    CU_initialize_registry();
    CU_pSuite suite = CU_add_suite("Tetris Test Suite", NULL, NULL);
    CU_add_test(suite, "test_init", test_init);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return 0;
}
