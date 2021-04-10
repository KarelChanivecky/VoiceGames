/**
* karel on 2021-04-09.
*
* This module sparingly uses code obtained from Fourzan's network book:
*
* Forouzan, Behrouz A. Data communications and networking / Behrouz A. Forouzan. — 5th ed.
* p. cm. ISBN 978-0-07-337622-6 (alk. paper)
*/

#include "voice_prot.h"
#include "arpa/inet.h"
#include <sys/socket.h>
#include <stdlib.h>
#include "string.h"
#include <stdio.h>

#define ORDER_INT32_PTR_INDEX 0
#define UID_INT32_PTR_INDEX 1
#define UID_INT8_PTR_INDEX 4
#define SAMPLES_INT8_PTR_INDEX 8

int recv_( int sock, uint8_t * buffer, struct sockaddr * client_addr, socklen_t * addr_len ) {

    int stat = recvfrom( sock, buffer, sizeof( buffer ), 0, client_addr, addr_len );

    if ( stat == -1 ) {
        exit( ERR_RECV );
    }

    return stat;
}

int send_( int sock, uint8_t * buffer, struct sockaddr * client_addr ) {
    int stat = sendto( sock, buffer, sizeof( buffer ), 0, client_addr, sizeof( *client_addr ));

    if ( stat == -1 ) {
        exit( ERR_SEND );
    }

    return stat;
}

int recv_voice( int sock, datagram_t * datagram, struct sockaddr * client_addr ) {
    uint8_t buffer[DATAGRAM_LEN] = { 0 };

    socklen_t client_len = 0;
    int stat = recv_( sock, buffer, client_addr, &client_len );
    uint32_t * buff_as_int32_ptr = ( uint32_t * ) buffer;
    datagram->order = ntohl( buff_as_int32_ptr[ ORDER_INT32_PTR_INDEX ] );
    datagram->uid = ntohl( buff_as_int32_ptr[ UID_INT32_PTR_INDEX ] );

    memcpy(( void * ) datagram->samples, &buffer[ SAMPLES_INT8_PTR_INDEX ], DATAGRAM_SAMPLE_C * 2 );

    return stat;
}

int send_voice( int sock, datagram_t * datagram, struct sockaddr * client_addr ) {
    uint8_t buffer[DATAGRAM_LEN] = { 0 };
    memcpy( buffer, &datagram->order, sizeof( uint32_t ));
    memset( &buffer[ UID_INT8_PTR_INDEX ], 0, sizeof( uint32_t ));
    memcpy( &buffer[ SAMPLES_INT8_PTR_INDEX ], &datagram->samples, DATAGRAM_SAMPLE_C * 2 );

    int stat = send_( sock, buffer, client_addr );

    return stat;
}

int get_voice_sock() {
    struct sockaddr_in servAddr;
    // Server (local) socket address

    // Build local (server) socket address
    // Allocate memory
    memset( &servAddr, 0, sizeof( servAddr ));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons( UDP_VOICE_PORT );
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Default IP address
    int s = socket( PF_INET, SOCK_DGRAM, 0 );
    // Create socket
    if ( s < 0 ) {
        perror( "Error: socket failed!" );
        exit( ERR_OPENING_PORT );
    }

    if (( bind( s, ( struct sockaddr * ) &servAddr, sizeof( servAddr )) < 0 )) {

        perror( "Error: bind failed!" );
        exit( ERR_OPENING_PORT );
    }

    return s;
}
