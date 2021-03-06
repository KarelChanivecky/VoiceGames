/**
* karel on 2021-04-10.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>
#include "lib/game_driver.h"
#include "game_server.h"
#include "lib/client_gateway.h"
#include "lib/handshaking.h"
#include "lib/new_client_plexer.h"
#include "utils.h"
#include "lib/game_collection.h"
#include "lib/game_environment.h"
#include "gaming_prot/gaming_prot.h"
#include "games/fsm_symbols.h"


void initialize_client_gateway( extended_thread_list_t  * new_clients_list, int server_fd ) {
    accept_clients_args * args = ( accept_clients_args * ) malloc( sizeof( accept_clients_args ));

    if ( !args ) {
        perror( "Could not alloc mem" );
        exit( MALLOC_ERR );
    }

    args->server_fd = server_fd;

    args->queue_arg = new_clients_list;

    pthread_t t;
    pthread_create( &t, NULL, accept_clients, args );
}

void initialize_handshaking( extended_thread_list_t * new_client_list, extended_thread_list_t * handshaked_clients ) {
    handshaker_args_t * args = ( handshaker_args_t * ) malloc( sizeof( handshaker_args_t ));
    if ( !args ) {
        perror( "Could not alloc mem" );
        exit( MALLOC_ERR );
    }

    args->new_clients = new_client_list;
    args->handshaked_clients = handshaked_clients;

    pthread_t t;
    pthread_create( &t, NULL, handshaking_thread, args );
}

void initialize_new_client_plexer( extended_thread_list_t * handshaked_clients_list, int new_game_signal ) {
    new_client_plexer_args_t * args = (new_client_plexer_args_t * ) malloc( sizeof(new_client_plexer_args_t));
    args->handshaked_clients = handshaked_clients_list;
    args->new_game_signal = new_game_signal;

    pthread_t t;
    pthread_create( &t, NULL, new_client_plexer_thread, args );
}


void initialize_service_threads( int server_fd, int new_game_signal, int udp_port_num ) {
    extended_thread_list_t * new_client_list = get_extended_threaded_list();
    initialize_client_gateway( new_client_list, server_fd );

    extended_thread_list_t * handshaked_clients = get_extended_threaded_list();
    initialize_handshaking( new_client_list, handshaked_clients );

    initialize_new_client_plexer( handshaked_clients, new_game_signal );

    initialize_voice_server(udp_port_num);
}

bool asses_read_move_state( int client, game_environment * game_env, int status) {
    switch ( status ) {
        case INTERNAL_ERR: {
            perror("Internal error");
            exit(INTERNAL_ERR);
        }
        case PLAYER_DISCONNECTED: {
            if (game_env->game_sockets[PLAYER_1_INDEX] == client) {
                update_opp_disconn(game_env->game_sockets[PLAYER_2_INDEX]);
            } else {
                update_opp_disconn(game_env->game_sockets[PLAYER_1_INDEX]);
            }
            game_collection.remove(client);
            return false;
        }
        case INSUFFICIENT_DATA : {
            return false;
        }
        case INVALID_REQ: {
            return false;
        }
        default:
            return true;
    }
}

bool asses_quit( int client, const game_environment * game_env, Request * req ) {
    if ((*req).type == REQ_META && (*req).context == META_QUIT) {
        if (game_env->game_sockets[PLAYER_1_INDEX] == client) {
            update_opp_disconn(game_env->game_sockets[PLAYER_2_INDEX]);
        } else {
            update_opp_disconn(game_env->game_sockets[PLAYER_1_INDEX]);
        }
        game_collection.remove(client);
        return true;
    }
    return false;
}


void serve_clients( fd_set * client_fd_set ) {
    puts("Serving clients");
//    game_collection.lock();
    for ( int client = MIN_FD; client < MAX_CONN; ++client ) {
        if ( !FD_ISSET( client, client_fd_set )) {
            continue;
        }

        game_environment * game_env = game_collection.get( client );
        printf("serving %d game env is %s\n", client, game_env ? "NULL" : "NOT NULL" );
        if ( game_env == NULL) {
            continue;
        }

        Request req;
        int stat = read_move( client, &req);
        printf("serving %d Read state %d\n", client, stat);

        if (!asses_read_move_state( client, game_env, stat )) {
            printf("serving bad client: %d\n", client);
            continue;
        }

        if (asses_quit( client, game_env, &req )) {
            printf("player quit %d\n", client);
            continue;
        }

        game_env->mover = client;
        game_env->move = req.payload[0];
        free(req.payload);

        printf( "Playing %d\n", client);

        stat = play_game( game_env );

        printf("play status for %d: %d\n", client, stat);
        if (stat == DESTROY) {
            game_collection.remove( client);
        }
    }
//    game_collection.unlock();
}




void server( int server_fd, int udp_port_num ) {
    puts("INITIALIZING SERVER");
    game_collection.init(MAX_CONN);

    int new_game_signal[2];
    if ( pipe(new_game_signal) == -1) {
        perror("Could not init new game pipe");
        exit(ERR_PIPE);
    }

    initialize_service_threads( server_fd, new_game_signal[1], udp_port_num );

    puts("");
    fd_set client_fd_set;

    while ( true ) {
        init_fd_set( &client_fd_set );

        FD_SET(new_game_signal[0], &client_fd_set);

        puts("entering server select");
        int set_fd_count = await_select( &client_fd_set );
        puts("exiting server select");
        if ( set_fd_count == 0 ) {
            continue;
        }

        if ( FD_ISSET( new_game_signal[0], &client_fd_set)) {
            char _;
            if (read(new_game_signal[0], &_, 1) == -1) {
                perror("HOW COULD YOU!");
            }
            continue;
        }
        serve_clients( &client_fd_set );
    }

}


int get_port_from_arg( char ** argv, int index ) {
    int port_number = ( int ) strtol( argv[ index ], NULL, 10 );
    if ( port_number < MIN_PORT || MAX_PORT < port_number ) {
        fprintf( stderr,
                 "Invalid tcp_port number entered! This application only accepts: %d < tcp_port < %d\n",
                 MIN_PORT, MAX_PORT );
        exit( EXIT_FAILURE );
    }
    return port_number;
}

// argv1 = tcp, arg2 = udp
int main( int argc, char ** argv ) {
    server_config server_cfg;
    int udp_port_num = NO_PORT;
    if ( argc != PORT_PARAM_GIVEN ) {
        server_cfg.tcp_port = DEFAULT_TCP_PORT;
        udp_port_num = DEFAULT_UDP_PORT;
    } else {
        server_cfg.tcp_port = get_port_from_arg( argv, 1 );
        udp_port_num = get_port_from_arg( argv, 2 );
        puts("custom ports");
    }

    printf("tcp: %d\nudp: %d\n", server_cfg.tcp_port, udp_port_num);
    get_tcp_socket( &server_cfg );

    server( server_cfg.listen_sock, udp_port_num );

    return EXIT_SUCCESS;
}
