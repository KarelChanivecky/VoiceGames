/**
* karel on 2021-04-10.
*/

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "voice_server.h"
#include "../lib/game_collection.h"
#include "../games/fsm_symbols.h"
#include "dc_utils/dc_threaded_queue.h"
#include "../game_server.h"
#include "../utils.h"

typedef struct {
    int port;
} udp_args_t;

bool set_client_addr( datagram_t * datagram, struct sockaddr_in * client_addr) {
    int uid = (int) datagram->uid;

//    printf("listener uid :%d\n", uid);
    game_environment * game_env = game_collection.get(uid);

    game_collection.lock();

    if (!game_env) {
        game_collection.unlock();
        return false;
    }


    int index = game_env->game_sockets[PLAYER_1_INDEX] == uid
            ? PLAYER_1_INDEX : PLAYER_2_INDEX;
    struct sockaddr_in * stored_addr = &game_env->voice_sockets[index];

    if ( stored_addr->sin_port != NO_PORT) {
        game_collection.unlock();
        return true;
    }

    memcpy( stored_addr, client_addr, sizeof (struct sockaddr_in));

    game_collection.unlock();

    return true;
}


void send_datagram(int socket, datagram_t * datagram ) {
    int uid = (int) datagram->uid;

//    printf("talker uid :%d\n", uid);

    game_environment * game_env = game_collection.get(uid);
    game_collection.lock();

    if (!game_env) {
        game_collection.unlock();
        return;
    }
    int index = game_env->game_sockets[PLAYER_1_INDEX] == uid
                ? PLAYER_2_INDEX : PLAYER_1_INDEX;

    struct sockaddr_in * recipient_addr = &game_env->voice_sockets[index];

    if (recipient_addr->sin_port == NO_PORT) {
        game_collection.unlock();
        return;
    }

//        print_client_addr( client_addr );

    send_voice(socket, datagram, (struct sockaddr*)recipient_addr);

    free(datagram);

    game_collection.unlock();
}

void * listener( void * v_datagram_queue ) {

    udp_args_t * args = (udp_args_t *) v_datagram_queue;

    int port = args->port;

    int socket = get_udp_sock(port);

    while ( 1 ) {
        struct sockaddr_in client_addr;
        datagram_t * datagram = (datagram_t * ) malloc( sizeof(datagram_t));

        socklen_t socklen = sizeof(client_addr);
        recv_voice(socket, datagram, (struct sockaddr *) &client_addr, &socklen);
        if (MAX_CONN < datagram->uid ) {
            free(datagram);
            continue;
        }
        if (!set_client_addr(datagram, &client_addr)) {
            continue;
        }

        send_datagram(socket, datagram);
    }
}



void initialize_voice_server(int udp_port_num) {
    udp_args_t * args = (udp_args_t *) malloc( sizeof(udp_args_t));
    if (!args) {
        perror("Could not alloc");
        exit(MALLOC_ERR);
    }

    args->port = udp_port_num;
    pthread_t udp;

    pthread_create( &udp, NULL, listener, args);

}
