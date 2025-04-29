#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "../src/tetris.h"

void test_init() {
    init();

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
