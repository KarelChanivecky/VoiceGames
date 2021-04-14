/**
* karel on 2021-04-10.
*/

#ifndef GAME_SERVER_CLIENT_GATEWAY_H
#define GAME_SERVER_CLIENT_GATEWAY_H

#include "threaded_list.h"


typedef struct {
    extended_thread_list_t * queue_arg;
    int server_fd;
} accept_clients_args;

void * accept_clients( void * params );

#endif //GAME_SERVER_CLIENT_GATEWAY_H
