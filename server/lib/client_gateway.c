/**
* karel on 2021-04-10.
*/

#include "client_gateway.h"
#include <stdio.h>
#include "../utils.h"
#include "dc/sys/socket.h"

void add_player_to_queue( extended_thread_list_t  * queue_args, int fd){
    lock_mx(&queue_args->list_mx);
    dlinked_push( queue_args->list, (void *)((long)fd));
    sem_post(&queue_args->items_sem);
    unlock_mx(&queue_args->list_mx);
}

_Noreturn void * accept_clients( void * params ) {
    puts("accepting clients");
    accept_clients_args *args = (accept_clients_args *) params;
    while(true){

        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int new_conn = dc_accept( args->server_fd, ( struct sockaddr * ) &client_addr, &client_len);
        printf("received new client %d\n", new_conn);
        printf("new conn: %d\n", new_conn);
        print_client_addr(&client_addr);
        if ( new_conn == -1 ) {
            perror( "COMM ERROR! @await connection. Problem with listening socket" );
        }
        add_player_to_queue(args->queue_arg, new_conn);
    }
}
