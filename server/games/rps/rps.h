/**
* karel on 2021-04-12.
*/

#ifndef GAME_SERVER_RPS_H
#define GAME_SERVER_RPS_H

#include <stdint.h>
#include "../../lib/game_environment.h"

enum {
    ROCK = 1,
    PAPER,
    SCISSORS
};

typedef struct {
    uint8_t choices[2];
} rps_game_state;

#ifndef EXTERN
#define EXTERN extern
#endif

EXTERN game_funcs rps;

#endif //GAME_SERVER_RPS_H
