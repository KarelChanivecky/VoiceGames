/**
* karel on 2021-04-10.
*/

#ifndef LIB_VOICE_CHAT_H
#define LIB_VOICE_CHAT_H

#include "../voice_chat_prot/voice_prot.h"
#include <pthread.h>

typedef struct {
    struct sockaddr_in sockets[2];
    pthread_mutex_t socket_mx;
} voice_client_sockets_t;

void initialize_voice_server();

#endif //LIB_VOICE_CHAT_H
