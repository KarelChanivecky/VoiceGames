/**
* karel on 2021-04-10.
*/

#ifndef GAME_SERVER_NEW_CLIENT_PLEXER_H
#define GAME_SERVER_NEW_CLIENT_PLEXER_H

#include "threaded_list.h"

typedef struct {
    extended_thread_list_t * handshaked_clients;
    int new_game_signal;
} new_client_plexer_args_t;


_Noreturn void * new_client_plexer_thread( void * );

#endif //GAME_SERVER_NEW_CLIENT_PLEXER_H
