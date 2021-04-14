/**
* karel on 2021-04-12.
*/

#include "game_driver.h"
#include <stdint.h>
#include "../games/fsm_symbols.h"
#include "common_symbols.h"
#include "../gaming_prot/gaming_prot.h"

int get_other(game_environment * env) {
    if (env->mover == env->game_sockets[0]) {
        return env->game_sockets[1];
    }
    return env->game_sockets[0];
}

int evaluate_move_state(game_environment * env, int move_state) {
    switch ( move_state ) {
        case WON: {
            uint8_t winner_payload[] = { WON, env->move};
            update_end_game( env->mover, winner_payload, 2);
            uint8_t loser_payload[] = { LOST, env->move};
            update_end_game( get_other(env), loser_payload, 2);
            return DESTROY;
        }
        case TIE: {
            uint8_t payload[] = { TIE, env->move};
            update_end_game( env->mover, payload, 2);
            update_end_game( get_other(env), payload, 2);
            return DESTROY;
        }
        case LOST: {
            uint8_t winner_payload[] = { WON, env->move};
            update_end_game( get_other(env), winner_payload, 2);
            uint8_t loser_payload[] = { LOST, env->move};
            update_end_game( env->mover, loser_payload, 2);
            return DESTROY;
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
