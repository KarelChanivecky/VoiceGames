/**
* karel on 2021-02-11.
*/

#ifndef THE_ACTUAL_FSN_UTILS_H
#define THE_ACTUAL_FSN_UTILS_H

#include <dc_utils/dlinked_list.h>
#include <stdbool.h>
#include <sys/select.h>
#include <pthread.h>
#include "lib/game_environment.h"

typedef struct {
    int tcp_port;
    int udp_port;
    int sin_family;
    int addr;
    int listen_sock;
} server_config;

#define MIN_FD 4

#define NO_FD -1


#define COMM_OK 1

void init_fd_set( fd_set * client_fd_set );

int await_select( fd_set * client_fd_set );

void get_tcp_socket( server_config * server_cfg );

void lock_mx( pthread_mutex_t * mutex );

void unlock_mx( pthread_mutex_t * mutex );

int get_other( game_environment * env );

void print_client_addr( const struct sockaddr_in * client_addr );

#endif //THE_ACTUAL_FSN_UTILS_H
