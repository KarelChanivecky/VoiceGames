/**
* karel on 2021-04-09.
*/

#ifndef TIC_TAC_TOE_VOICE_PROT_H
#define TIC_TAC_TOE_VOICE_PROT_H

#include <stdint.h>
#include "arpa/inet.h"

#ifndef PLAYER_DISCONNECTED
#define PLAYER_DISCONNECTED -20
#endif

#ifndef INTERNAL_ERR
#define INTERNAL_ERR -25
#endif

#ifndef INCOMPLETE_WRITE
#define INCOMPLETE_WRITE -30
#endif

#ifndef MALLOC_ERR
#define MALLOC_ERR -35
#endif

#ifndef OK
#define OK true
#endif

#define NO_PORT 0

// The class actually agreed per protocol to use these sockets
#define UDP_INBOUND_PORT 2034
#define UDP_OUTBOUND_PORT 2035

#define DATAGRAM_LEN 5008

#define DATAGRAM_SAMPLE_C 2500

enum foo {
    ERR_RECV = -120,
    ERR_SEND,
    ERR_OPENING_PORT
};



typedef struct {
    uint32_t order;
    uint32_t uid;
//    uint16_t port;
    uint16_t samples[DATAGRAM_SAMPLE_C];
} datagram_t;


int recv_voice( int sock, datagram_t * datagram, struct sockaddr * client_addr, socklen_t * client_len );

int send_voice( int sock, datagram_t * datagram, struct sockaddr * client_addr );

int get_inbound_sock();

int get_outbound_sock();


#endif //TIC_TAC_TOE_VOICE_PROT_H
