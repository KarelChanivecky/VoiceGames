/**
* karel on 2021-04-12.
*/

#include <stdio.h>
#include <stdlib.h>
#include "ttt_utils.h"
#include "ttt.h"
#include "../../gaming_prot/gaming_prot.h"

#define ROW_DIFF 3
#define COL_DIFF 1
#define FD_DIFF 2
#define BD_DIFF 4


/**
 * Evaluate if there is a winning condition.
 *  <br>
 * 4 ways to win:
 * <br>
 * - all cells in row<br>
 * - all cells in col<br>
 * - forward-slash diagonal i.e. : / <br>
 * - back-slash diagonal i.e. : \
 *
 * @param game_board A 9-cell Tic-Tac-Toe board represented as an int array, where the top-left cell is index 0
 *                      and the bottom-right is index 8
 *
 * @param starting_cell The cell from which to start evaluating the winning condition.
 *                      Specifically provided to evaluate forward-slash winning condition. <br>
 *                      Pass:<br>
 *                      - R0_C2 for back-slash diagonal <br>
 *                      - R0_C0 otherwise
 *
 * @param winning_cell_diff Used to select the cells that would create a winning condition.
 *                  The number of cells to skip over in the linear array. <br>
 *                  - COL_DIFF for evaluating rows <br>
 *                  - ROW_DIFF for evaluating columns <br>
 *                  - FD_DIFF for evaluating forward-slash diagonal <br>
 *                  - BD_DIFF for evaluating backward-slash diagonal
 *
 * @param displacement_cell_diff The number of cells to displace in order to re-evaluate winning condition in case that
 *                                 a winning condition is not found at certain position.<br>
 *                  Pass: <br>
 *                  - COL_DIFF for evaluating columns<br>
 *                  - ROW_DIFF for evaluating rows<br>
 *                  - CELLS_IN_GRID for evaluating both diagonals
 * @return true if any of the winning conditions are met
 */
bool eval_win_condition( const int game_board[CELLS_IN_GRID],
                         int starting_cell,
                         int ending_cell,
                         int winning_cell_diff,
                         int displacement_cell_diff ) {
    for ( int cell = starting_cell; cell <= ending_cell; cell += displacement_cell_diff ) {

        int cell1 = game_board[ cell ];
        int cell2 = game_board[ cell + winning_cell_diff ];
        int cell3 = game_board[ cell + winning_cell_diff * 2 ];

        if (cell1 == FREE_CELL || cell2 == FREE_CELL) {
            continue;
        }

        if ( cell1 == cell2  && cell2 == cell3 ) {
            return true;
        }
    }
    return false;
}

bool won_row( const int game_board[CELLS_IN_GRID] ) {
    return eval_win_condition( game_board, R0_C0, R2_C0, COL_DIFF, ROW_DIFF );
}

bool won_col( const int game_board[CELLS_IN_GRID] ) {
    return eval_win_condition( game_board, R0_C0, R0_C2, ROW_DIFF, COL_DIFF );
}

bool won_f_diag( const int game_board[CELLS_IN_GRID] ) {
    return eval_win_condition( game_board, R0_C2, R1_C0, FD_DIFF, CELLS_IN_GRID );
}

bool won_b_diag( const int game_board[CELLS_IN_GRID] ) {
    return eval_win_condition( game_board, R0_C0, R0_C1, BD_DIFF, CELLS_IN_GRID );
}

bool user_won( const int game_board[CELLS_IN_GRID] ) {
    return won_row( game_board )
           || won_col( game_board )
           || won_f_diag( game_board )
           || won_b_diag( game_board );
}

void init_game_board( game_board_s * game_board ) {
    game_board->cells_taken = 0;
    for ( int cell = R0_C0; cell <= R2_C2; cell++ ) {
        game_board->cells[ cell ] = FREE_CELL;
    }
}

void print_board( game_board_s game_board ) {
    printf( "   Board >\n"
            "       cells taken: %d\n"
            "       cell state >\n"
            "           %d | %d | %d\n"
            "           ---------\n"
            "           %d | %d | %d\n"
            "           ---------\n"
            "           %d | %d | %d\n",
            game_board.cells_taken,
            game_board.cells[ R0_C0 ], game_board.cells[ R0_C1 ], game_board.cells[ R0_C2 ],
            game_board.cells[ R1_C0 ], game_board.cells[ R1_C1 ], game_board.cells[ R1_C2 ],
            game_board.cells[ R2_C0 ], game_board.cells[ R2_C1 ], game_board.cells[ R2_C2 ] );
}





