/**
* karel on 2021-04-10.
*/

#ifndef GAME_SERVER_GAME_SWITCH_H
#define GAME_SERVER_GAME_SWITCH_H

#include "threaded_list.h"
#include "stdint.h"


typedef struct {
    extended_thread_list_t  * new_clients;
    extended_thread_list_t * handshaked_clients;
    uint8_t * available_games;
} handshaker_args_t;

typedef struct {
    int client_fd;
    uint8_t game_choice;
} handshaked_client_t;

_Noreturn void * handshaking_thread( void * args);

#endif //GAME_SERVER_GAME_SWITCH_H
