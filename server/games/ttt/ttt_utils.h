/**
* karel on 2021-04-12.
*/

#ifndef GAME_SERVER_TTT_UTILS_H
#define GAME_SERVER_TTT_UTILS_H

#include <stdbool.h>

#define CELLS_IN_GRID 9

/**
 * Contains the possible game_state codes which are sent to the client. <br>
 *
 * See the attached BNF <br>
 *
 * The first nine codes represent a cell in the tic-tac-toe grid,
 * where R represents row, C represents column.
 */
typedef enum {
    R0_C0,
    R0_C1,
    R0_C2,
    R1_C0,
    R1_C1,
    R1_C2,
    R2_C0,
    R2_C1,
    R2_C2,
} game_states;



enum board_ticks {
    FREE_CELL ,
    P1,
    P2
};

typedef struct {
    int cells[CELLS_IN_GRID];
    int cells_taken;
} game_board_s;


typedef struct {
    game_board_s game_board;
    bool player_1_is_X;
    bool X_turn;
} ttt_game_state;

void print_board( game_board_s game_board );

void init_game_board( game_board_s * game_board );

bool user_won( const int game_board[CELLS_IN_GRID] );

#endif //GAME_SERVER_TTT_UTILS_H
