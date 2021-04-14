/**
* karel on 2021-04-10.
*/

#include "handshaking.h"
#include <dc_utils/dlinked_list.h>
#include <stdio.h>
#include "../gaming_prot/gaming_prot.h"
#include "../utils.h"
#include "common_symbols.h"

void initialize_selector( fd_set * selector, dlinked_list * clients ) {
    FD_ZERO( selector );

    unsigned long new_client = ( int ) ( unsigned long ) dlinked_pop_head( clients );
    while (( void * ) new_client != NULL) {
        FD_SET( new_client, selector );
        new_client = ( int ) ( unsigned long ) dlinked_pop_head( clients );
    }
}

void receive_new_clients( dlinked_list * clients, extended_thread_list_t * new_clients ) {
    sem_wait(&new_clients->items_sem);
    lock_mx( &new_clients->list_mx );
    void * new_client = dlinked_pop_head( new_clients->list );
    int waits = -1;
    while ( new_client && waits++ ) {
        printf( "handshaker welcomes %d\n", ( int ) ( unsigned long ) new_client );
        dlinked_push( clients, new_client );
        new_client = dlinked_pop_head( new_clients->list );
    }
    unlock_mx( &new_clients->list_mx );

    while (waits--) {
        sem_wait(&new_clients->items_sem);
    }
}

handshaked_client_t * handshake_client( int client, int * stat ) {
    Request req;
    *stat = read_game_choice( client, &req );
    if ( *stat < 0 ) {
        return NULL;
    }

    handshaked_client_t * handshaked_client = ( handshaked_client_t * ) malloc( sizeof( handshaked_client_t ));
    if ( !handshaked_client ) {
        perror( "Could not allocate memory" );
        exit( MALLOC_ERR );
    }

    respond_uid( client, client );
    if ( *stat < 0 ) {
        return NULL;
    }

    handshaked_client->client_fd = client;
    handshaked_client->game_choice = req.payload[ GPROT_PAYLOAD_GAME_INDEX ];

    printf( "handshaked %d\n", ( int ) ( unsigned long ) client );

    return handshaked_client;
}

dlinked_list * handshake_clients( dlinked_list * clients, fd_set * selector ) {
    dlinked_list * handshaked_clients = dlinked_create_list();

    int client = ( int ) ( unsigned long ) dlinked_pop_head( clients );
    while ( client ) {
        if ( !FD_ISSET( client, selector )) {
            dlinked_push( clients, ( void * ) ( unsigned long ) client );
            continue;
        }

        int handshake_stat = OK;
        handshaked_client_t * handshaked_client = handshake_client( client, &handshake_stat );
        if ( !handshaked_client && handshake_stat != PLAYER_DISCONNECTED ) {
            dlinked_push( clients, ( void * ) ( unsigned long ) client );
            continue;
        }

        dlinked_push( handshaked_clients, handshaked_client );
        client = ( int ) ( unsigned long ) dlinked_pop_head( clients );
    }

    return handshaked_clients;
}

void pass_handshaked_clients( dlinked_list * handshaked_clients, extended_thread_list_t * next_list ) {
    void * client = dlinked_pop_head( handshaked_clients );
    pthread_mutex_lock( &next_list->list_mx );
    while ( client != NULL) {
        dlinked_push( next_list->list, client );
        sem_post( &next_list->items_sem );
        client = dlinked_pop_head( handshaked_clients );
    }
    pthread_mutex_unlock( &next_list->list_mx );
}

_Noreturn void * handshaking_thread( void * vargs ) {
    puts( "Started handshaking" );
    handshaker_args_t * args = ( handshaker_args_t * ) vargs;
    dlinked_list * clients = dlinked_create_list();
    fd_set selector;

    while ( true ) {
        receive_new_clients( clients, args->new_clients );
        initialize_selector( &selector, dlinked_shallow_copy( clients ));
        puts("Entering handshaking select");
        await_select( &selector );
        puts("exited handshaking select");
        dlinked_list * handshaked_clients = handshake_clients( clients, &selector );
        pass_handshaked_clients( handshaked_clients, args->handshaked_clients );
    }
}
