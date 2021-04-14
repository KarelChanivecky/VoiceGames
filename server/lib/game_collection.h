/**
* karel on 2021-04-13.
*/

#ifndef TIC_TAC_TOE_GAME_COLLECTION_H

#define TIC_TAC_TOE_GAME_COLLECTION_H

#include <stdlib.h>
#include "game_environment.h"

#ifndef EXTERN
#define EXTERN extern
#endif

EXTERN struct gcol {
    void (*init)(size_t size);
    void (*create)(int p1, int p2, game_funcs game);
    void (*remove)(int uid);
    game_environment * (*get)(int uid);
    void (*lock)();
    void (*unlock)();
} game_collection;

#endif //TIC_TAC_TOE_GAME_COLLECTION_H
