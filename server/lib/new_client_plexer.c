/**
* karel on 2021-04-10.
*/

#include "new_client_plexer.h"
#include "handshaking.h"
#include "game_collection.h"
#include "../games/fsm_symbols.h"
#include "../games/ttt/ttt.h"
#include "../games/rps/rps.h"
#include "game_environment.h"
#include "../game_server.h"
#include <dc_utils/dlinked_list.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define NO_DICE -1

void init_game_players( dlinked_list * game_players[AVAILABLE_GAME_COUNT] ) {
    for ( int i = 0; i < AVAILABLE_GAME_COUNT; ++i ) {
        game_players[ i ] = dlinked_create_list();
    }
}

int plex_next_client( extended_thread_list_t * clients, dlinked_list * const * game_players) {
    sem_wait( &clients->items_sem );
    pthread_mutex_lock( &clients->list_mx );
    handshaked_client_t * client = dlinked_pop_head( clients->list );
    printf("Plexing %d\n", client->client_fd);
    dlinked_push( game_players[ client->game_choice ], ( void * ) ( unsigned long ) client->client_fd );
    pthread_mutex_unlock( &clients->list_mx );
    if ( game_players[ client->game_choice ]->size == 2 ) {
        return client->game_choice;
    }
    free(client);
    return NO_DICE;
}


void build_game( dlinked_list * clients, game_funcs game ) {
    int game_clients[2];
    for ( int i = 0; i < 2; ++i ) {
        game_clients[ i ] = ( int ) ( unsigned long ) dlinked_pop_head( clients );
    }

    game_collection.create(game_clients[PLAYER_1_INDEX], game_clients[PLAYER_2_INDEX], game);

}

_Noreturn void * new_client_plexer_thread( void * vargs ) {
    puts("Started multiplexing clients");
    new_client_plexer_args_t * args = ( new_client_plexer_args_t * ) vargs;
    dlinked_list * game_players[AVAILABLE_GAME_COUNT];
    init_game_players( game_players );

    while ( true ) {
        int game_to_build = plex_next_client( args->handshaked_clients, game_players );

        if ( game_to_build == NO_DICE ) {
            continue;
        }

        if (game_to_build == TTT) {
            puts("built TTT");
            build_game(game_players[game_to_build], ttt);
        } else {
            puts("built RPS");
            build_game(game_players[game_to_build], rps);
        }

        if (write(args->new_game_signal, "1", 1) == -1) {
            perror("why you!");
        }
    }
}
