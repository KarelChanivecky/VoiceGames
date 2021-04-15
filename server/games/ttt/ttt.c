/**
* karel on 2021-04-12.
*/

#include "ttt.h"
#include "../../lib/common_symbols.h"   `
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "../../gaming_prot/gaming_prot.h"
#include "ttt_utils.h"
#include "../fsm_symbols.h"
#include "../../lib/game_environment.h"
#include "../../game_server.h"
#include "../../utils.h"


/**
 * Validate move.
 *
 * Reasons a move could be invalid:
 * - cell is already taken
 * - choice is out of range
 */
static int validate_move( game_environment * env ) {


    ttt_game_state * ttt_state = ( ttt_game_state * ) env->game_state;


    bool player_1_turn = !( ttt_state->X_turn ^ ttt_state->player_1_is_X );

    int player_index = player_1_turn ? PLAYER_1_INDEX : PLAYER_2_INDEX;

    int player_choice =  env->move;

    if (env->game_sockets[player_index] != env->mover) {
        return ERR_OUT_OF_TURN;
    }

    if ( R2_C2 < player_choice ) {
        return ERR_INVAL_CHOICE;
    }

    if ( ttt_state->game_board.cells[ player_choice ] != FREE_CELL ) {
        return ERR_INVAL_CHOICE;
    }

    return OK;
}

static int evaluate_move( game_environment * env ) {
    puts( "\nEntering evaluate state" );

    ttt_game_state * ttt_state = ( ttt_game_state * ) env->game_state;

    bool player_1_turn = !( ttt_state->X_turn ^ ttt_state->player_1_is_X );

    ttt_state->game_board.cells[ env->move ] = player_1_turn ? P1 : P2;
    ttt_state->game_board.cells_taken++;
    print_board(ttt_state->game_board);

    if ( user_won( ttt_state->game_board.cells )) {
        puts( "Evaluated player won" );
        return WON;
    }

    if ( ttt_state->game_board.cells_taken == CELLS_IN_GRID ) {
        puts( "Evaluated player tie" );
        return TIE;
    }

    ttt_state->X_turn = !ttt_state->X_turn;
    return OK;
}

void * make_ttt_game_state(int * status) {
    ttt_game_state * state = (ttt_game_state * ) malloc( sizeof(ttt_game_state));
    if (!state) {
        return NULL;
    }
    init_game_board(&state->game_board);
    state->X_turn = true;
    int who_is_X = rand() % 2;

    state->player_1_is_X = who_is_X == PLAYER_1_INDEX;
    *status = OK;
    return state;
}

int destroy_ttt(game_environment * env) {
    free(env->game_state);
    return OK;
}

int game_start(game_environment * env) {
    ttt_game_state * ttt_state = ( ttt_game_state * ) env->game_state;
    uint8_t X[] = { 1 };
    uint8_t O[] = { 2 };
    int stat = update_start_game_with_payload(env->game_sockets[PLAYER_1_INDEX],
                                   1,
                                   ttt_state->player_1_is_X ? X : O);

    if (stat == PLAYER_DISCONNECTED) {
        update_opp_disconn(env->game_sockets[PLAYER_2_INDEX]);
        return stat;
    }

    stat = update_start_game_with_payload(env->game_sockets[PLAYER_2_INDEX],
                                   1,
                                   ttt_state->player_1_is_X ? O : X);

    if (stat == PLAYER_DISCONNECTED) {
        update_opp_disconn(env->game_sockets[PLAYER_1_INDEX]);
        return stat;
    }

    return OK;
}

int won(game_environment * env) {
    uint8_t winner_payload[] = { WON, env->move};
    update_end_game( env->mover, winner_payload, 2);
    uint8_t loser_payload[] = { LOST, env->move};
    update_end_game( get_other(env), loser_payload, 2);
    return OK;
}

int tied(game_environment * env) {
    uint8_t payload[] = { TIE, env->move};
    update_end_game( env->mover, payload, 2);
    update_end_game( get_other(env), payload, 2);
    return OK;
}

int lost(game_environment * _) {
    return OK; // not possible to fall here for ttt
}

game_funcs ttt = {
        validate_move,
        evaluate_move,
        won,
        tied,
        lost,
        game_start,
        destroy_ttt,
        make_ttt_game_state,
        TTT
};
