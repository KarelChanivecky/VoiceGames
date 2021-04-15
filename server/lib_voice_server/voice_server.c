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

bool set_client_addr(datagram_t * datagram, struct sockaddr_in * client_addr) {
    int uid = (int) datagram->uid;

    printf("listener uid :%d\n", uid);
    game_environment * game_env = game_collection.get(uid);

    game_collection.lock();

    if (!game_env) {
        puts("not found");
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

void * listener( void * v_datagram_queue ) {

    struct dc_threaded_queue * queue = (struct dc_threaded_queue *) v_datagram_queue;

    int socket = get_inbound_sock();

    while ( 1 ) {
        struct sockaddr_in client_addr;
        datagram_t * datagram = (datagram_t * ) malloc( sizeof(datagram_t));

        socklen_t socklen = sizeof(client_addr);
        recv_voice(socket, datagram, (struct sockaddr *) &client_addr, &socklen);
        if (MAX_CONN < datagram->uid ) {
            free(datagram);
            continue;
        }
        if (set_client_addr(datagram, &client_addr)) {
            queue->add(queue, datagram);
        }

    }
}

void * talker( void * v_datagram_queue ) {
    struct dc_threaded_queue * queue = (struct dc_threaded_queue *) v_datagram_queue;
    int socket = get_outbound_sock();
    while ( 1 ) {
        datagram_t * datagram = queue->take(queue);
        int uid = (int) datagram->uid;

        printf("talker uid :%d\n", uid);

        game_environment * game_env = game_collection.get(uid);
        game_collection.lock();

        if (!game_env) {
            game_collection.unlock();
            continue;
        }
        int index = game_env->game_sockets[PLAYER_1_INDEX] == uid
                    ? PLAYER_2_INDEX : PLAYER_1_INDEX;

        struct sockaddr_in * client_addr = &game_env->voice_sockets[index];

        if (client_addr->sin_port == NO_PORT) {
            game_collection.unlock();
            continue;
        }

        char str[INET_ADDRSTRLEN + 1];
        inet_ntop(AF_INET, &client_addr->sin_addr, str, INET_ADDRSTRLEN);
        printf("%d\n", client_addr->sin_family);
        printf("%d\n", ntohs(client_addr->sin_port));
        printf("%s\n", str);

        send_voice(socket, datagram, (struct sockaddr*)client_addr);

        free(datagram);

        game_collection.unlock();
    }
}


void initialize_voice_server() {
    struct dc_threaded_queue * queue = dc_threaded_queue_init(256);
    pthread_t listener_t;
    pthread_t talker_t;

    pthread_create(&listener_t, NULL, listener, queue);
    pthread_create(&talker_t, NULL , talker, queue);

    int status = pthread_join(listener_t, NULL);

    perror("thread crashed");

    puts("Initialized UDP server");
}
