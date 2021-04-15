/**
* karel on 2021-04-14.
*/

#ifndef TIC_TAC_TOE_GAME_ENVIRONMENT_H
#define TIC_TAC_TOE_GAME_ENVIRONMENT_H

#include <stdint.h>
#include <netinet/in.h>

struct game_environment;

typedef int(*game_func)(struct game_environment * env);
typedef void * (*constructor)(int * status);

typedef struct {
    game_func validate_move;
    game_func evaluate_move;
    game_func won;
    game_func tied;
    game_func lost;
    game_func start_game;
    game_func destruct_game;
    constructor construct_game;
    int game_name;
} game_funcs;

typedef struct game_environment{
    void * game_state;
    uint8_t move;
    uint8_t mover;
    int game_sockets[2];
    game_funcs game;
    struct sockaddr_in voice_sockets[2];
} game_environment;

#endif //TIC_TAC_TOE_GAME_ENVIRONMENT_H
