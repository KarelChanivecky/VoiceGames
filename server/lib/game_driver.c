/**
* karel on 2021-04-12.
*/

#include "game_driver.h"
#include <stdint.h>
#include "../games/fsm_symbols.h"
#include "common_symbols.h"
#include "../gaming_prot/gaming_prot.h"
#include "../utils.h"


int evaluate_move_state(game_environment * env, int move_state) {
    switch ( move_state ) {
        case WON: {
            env->game.won(env);
            return DESTROY;
        }
        case TIE: {
            env->game.tied(env);
            return DESTROY;
        }
        case LOST: {
            env->game.lost(env);
            return DESTROY;
        }
        case SILENT_MOVE: {
            return OK;
        }
        default: {
            uint8_t payload[] = { env->move };
            int stat = update_move( get_other(env), payload, 1);
            if (stat == PLAYER_DISCONNECTED) {
                update_opp_disconn( env->mover );
                return DESTROY;
            }
        }
    }
    return OK;
}

int play_game(game_environment * env) {
    game_funcs game = env->game;

    int validation_stat = game.validate_move(env);

    if (validation_stat != OK) {
        if (validation_stat == ERR_OUT_OF_TURN) {
            respond_out_of_turn(env->mover, 0);
        } else {
            respond_invalid_action(env->mover, 0);
        }
        return OK;
    }

    int move_state = game.evaluate_move(env);

    if (respond_success(env->mover, REQ_ACTION) != GPROT_OK) {
        update_opp_disconn(env->mover == env->game_sockets[PLAYER_1_INDEX]
                                            ? env->game_sockets[PLAYER_2_INDEX]
                                            : env->game_sockets[PLAYER_1_INDEX]);
        return DESTROY;
    }

    return evaluate_move_state(env, move_state);
}
