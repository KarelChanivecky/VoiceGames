/**
* karel on 2021-04-10.
*/

#ifndef TIC_TAC_TOE_GAME_SERVER_H
#define TIC_TAC_TOE_GAME_SERVER_H

#include "lib_voice_server/voice_server.h"
#include "lib/threaded_list.h"
#include <stdbool.h>

#define AVAILABLE_GAME_COUNT 2
#define MAX_CONN 255

#define PORT_PARAM_GIVEN 3
#define MIN_PORT 1024
#define MAX_PORT 65535
#define DEFAULT_TCP_PORT 3000
#define DEFAULT_UDP_PORT 2034

#define AWAIT_EXIT 254
#define AWAIT_CONTINUE 255

#define ERR_PIPE -6
#define CONTINUE -16


enum game_available {
    TTT = 1,
    RPS
};


#endif //TIC_TAC_TOE_GAME_SERVER_H
