/**
* karel on 2021-02-11.
*/

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <dc/sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <dc_utils/dlinked_list.h>
#include "utils.h"
#include "lib/game_collection.h"
#include "game_server.h"


void lock_mx( pthread_mutex_t * mutex ) {
    int lock_status = pthread_mutex_lock( mutex );
    if ( lock_status != 0 ) {
        perror( "ERROR: mutex lock" );
        exit( EXIT_FAILURE );
    }
}

void unlock_mx( pthread_mutex_t * mutex ) {
    int unlock_status = pthread_mutex_unlock( mutex );

    if ( unlock_status != 0 ) {
        perror( "ERROR: mutex unlock" );
        exit( EXIT_FAILURE );
    }
}

void get_socket( server_config * server_cfg ) {
    int listen_socket_fd;
    server_cfg->sin_family = AF_INET;
    server_cfg->addr = htonl(INADDR_ANY);

    struct sockaddr_in addr;
    listen_socket_fd = dc_socket( server_cfg->sin_family, SOCK_STREAM, 0 );

    int option = 1;
    int sock_opt_status = setsockopt( listen_socket_fd, SOL_SOCKET, SO_REUSEADDR,
                                      ( char * ) &option, sizeof( option ));
    if ( sock_opt_status == -1 ) {
        fprintf( stderr, "Error setting socket options!: %s", strerror(errno));
        exit( EXIT_FAILURE );
    }

    memset( &addr, 0, sizeof( struct sockaddr_in ));
    addr.sin_family = server_cfg->sin_family;
    addr.sin_port = htons( server_cfg->port );
    addr.sin_addr.s_addr = htonl( server_cfg->addr );
    dc_bind( listen_socket_fd, ( struct sockaddr * ) &addr, sizeof( struct sockaddr_in ));
    dc_listen( listen_socket_fd, MAX_CONN );
    server_cfg->listen_sock = listen_socket_fd;
}


void init_fd_set( fd_set * client_fd_set ) {
    game_collection.lock();
    FD_ZERO( client_fd_set );
    for ( int i = MIN_FD; i < MAX_CONN; ++i ) {
        if ( game_collection.get( i ) != NULL) {
            FD_SET( i, client_fd_set );
        }
    }
    game_collection.unlock();
}

int await_select( fd_set * client_fd_set ) {
    int stat = select( MAX_CONN + 1, client_fd_set, NULL, NULL, NULL);
    puts( "selected conn" );

    if ( stat == -1 ) {
        perror( "select" );
        exit( EXIT_FAILURE );
    }

    return stat;
}

int get_other(game_environment * env) {
    if (env->mover == env->game_sockets[0]) {
        return env->game_sockets[1];
    }
    return env->game_sockets[0];
}