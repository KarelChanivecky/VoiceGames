#include "../../lib/common_symbols.h"
#include <stdlib.h>
#include <stdbool.h>
#include "../../game_server.h"
#include "rps.h"
#include "../fsm_symbols.h"
#include "../../gaming_prot/gaming_prot.h"
#include "../../lib/game_driver.h"
#include "../../utils.h"

/**
* karel on 2021-04-12.
*/

int eval_game_end( uint8_t reference, uint8_t other ) {
    if ( reference == other ) {
        return TIE;
    }

    if ( reference - 1 == other ) {
        return WON;
    }

    if ( other - 1 == reference ) {
        return LOST;
    }

    if ( reference == ROCK && other == SCISSORS ) {
        return WON;
    }

    return LOST;

}

int get_index_of_mover( game_environment * env ) {
    if ( env->game_sockets[ 0 ] == env->mover ) {
        return 0;
    }
    return 1;
}

int get_index_of_other( game_environment * env ) {
    if ( env->game_sockets[ 0 ] != env->mover ) {
        return 0;
    }
    return 1;
}


int validate_move( game_environment * env ) {
    rps_game_state * rps_state = ( rps_game_state * ) env->game_state;

    int player = get_index_of_mover( env );
    if ( rps_state->choices[ player ] != NO_CHOICE ) {
        return ERR_OUT_OF_TURN;
    }

    int player_choice = env->move;
    if ( player_choice < ROCK || SCISSORS < player_choice ) {
        return ERR_INVAL_CHOICE;
    }

    return OK;
}

int evaluate_move( game_environment * env ) {
    rps_game_state * rps_state = ( rps_game_state * ) env->game_state;

    int other = get_index_of_other( env );
    int player = get_index_of_mover( env );

    rps_state->choices[ player ] = env->move;

    if ( rps_state->choices[ other ] != NO_CHOICE ) {
        return eval_game_end( rps_state->choices[ player ], rps_state->choices[ other ] );
    }

    return SILENT_MOVE;
}

void * make_rps_game_state( int * status ) {
    rps_game_state * state = ( rps_game_state * ) malloc( sizeof( rps_game_state ));
    if ( !state ) {
        return NULL;
    }

    *status = OK;
    state->choices[ 0 ] = NO_CHOICE;
    state->choices[ 1 ] = NO_CHOICE;
    return state;
}

int start_game( game_environment * game_env ) {

    int stat = update_start_game( game_env->game_sockets[ PLAYER_1_INDEX ] );

    if ( stat == PLAYER_DISCONNECTED || stat == INTERNAL_ERR ) {
        update_opp_disconn( game_env->game_sockets[ PLAYER_2_INDEX ] );
        return stat;
    }

    int other_stat = update_start_game( game_env->game_sockets[ PLAYER_2_INDEX ] );

    if ( other_stat == PLAYER_DISCONNECTED || other_stat == INTERNAL_ERR ) {
        update_opp_disconn( game_env->game_sockets[ PLAYER_1_INDEX ] );
        return stat;
    }
    return OK;
}

int destroy_rps( game_environment * env ) {
    free( env->game_state );
    return OK;
}

static int won( game_environment * env ) {
    rps_game_state * state = ( rps_game_state * ) env->game_state;
    uint8_t winner_payload[] = {
            WON,
            env->game_sockets[ PLAYER_1_INDEX ] == env->mover
            ? state->choices[ PLAYER_2_INDEX ]
            : state->choices[ PLAYER_1_INDEX ]
    };
    update_end_game( env->mover, winner_payload, 2 );
    uint8_t loser_payload[] = { LOST, env->move };
    update_end_game( get_other( env ), loser_payload, 2 );
    return OK;
}

static int tied( game_environment * env ) {
    uint8_t payload[] = { TIE, env->move };
    update_end_game( env->mover, payload, 2 );
    update_end_game( get_other( env ), payload, 2 );
    return OK;
}

static int lost( game_environment * env ) {
    rps_game_state * state = ( rps_game_state * ) env->game_state;
    uint8_t winner_payload[] = { WON, env->move };
    update_end_game( get_other( env ), winner_payload, 2 );
    uint8_t loser_payload[] = {
            LOST,
            env->game_sockets[ PLAYER_1_INDEX ] == env->mover
            ? state->choices[ PLAYER_2_INDEX ]
            : state->choices[ PLAYER_1_INDEX ]
    };
    update_end_game( env->mover, loser_payload, 2 );
    return OK;
}


game_funcs rps = {
        validate_move,
        evaluate_move,
        won,
        tied,
        lost,
        start_game,
        destroy_rps,
        make_rps_game_state,
        RPS
};
