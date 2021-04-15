/**
* karel on 2021-04-13.
*/

#include "game_collection.h"
#include <stdio.h>
#include <pthread.h>
#include <bits/socket.h>
#include <unistd.h>
#include "../games/fsm_symbols.h"
#include "../utils.h"
#include "common_symbols.h"
#include "../voice_chat_prot/voice_prot.h"

// I know you may hate this, but I am experimenting with design patterns and better now than in the job :P
// Besides, you know if I did this, it was very well thought out
static game_environment ** gcol_games;
static pthread_mutex_t gcol_mx;

void gcol_init( size_t size ) {
    gcol_games = ( game_environment ** ) malloc( sizeof( game_environment * ) * size );
    if ( !gcol_games ) {
        perror( "Could not alloc memory" );
        exit( MALLOC_ERR );
    }

    for ( size_t i = 0; i < size; i++ ) {
        gcol_games[ i ] = NULL;
    }
}

game_environment * make_env( int p1, int p2, game_funcs game ) {
    game_environment * new_game = ( game_environment * ) malloc( sizeof( game_environment ));
    if ( !new_game ) {
        perror( "Could not alloc memory" );
        exit( MALLOC_ERR );
    }

    new_game->game_sockets[ PLAYER_1_INDEX ] = p1;
    new_game->game_sockets[ PLAYER_2_INDEX ] = p2;

    int status;
    new_game->game_state = game.construct_game( &status );

    if ( status != OK ) {
        free( new_game );
        return NULL;
    }
    new_game->move = NO_CHOICE;
    new_game->mover = NO_FD;

    new_game->voice_sockets[ PLAYER_1_INDEX ].sin_family = AF_INET;
    new_game->voice_sockets[ PLAYER_1_INDEX ].sin_port = NO_PORT;
    new_game->voice_sockets[ PLAYER_2_INDEX ].sin_family = AF_INET;
    new_game->voice_sockets[ PLAYER_2_INDEX ].sin_port = NO_PORT;

    new_game->game = game;

    return new_game;
}

void gcol_add( game_environment * new_game ) {
    int p1 = new_game->game_sockets[ PLAYER_1_INDEX ];
    int p2 = new_game->game_sockets[ PLAYER_2_INDEX ];
    pthread_mutex_lock( &gcol_mx );
    gcol_games[ p1 ] = new_game;
    gcol_games[ p2 ] = new_game;
    pthread_mutex_unlock( &gcol_mx );
}


void gcol_create( int p1, int p2, game_funcs game ) {
    game_environment * new_game_env = make_env( p1, p2, game );
    if ( !new_game_env ) {
        return;
    }
    if ( game.start_game( new_game_env ) != OK ) {
        game.destruct_game( new_game_env );
        free( new_game_env ); // I can't understand why this is screaming at me
    }
    gcol_add( new_game_env );
}

int get_other_uid( game_environment * env, int uid ) {
    if ( env->game_sockets[ PLAYER_1_INDEX ] == uid ) {
        return env->game_sockets[ PLAYER_2_INDEX ];
    }
    return env->game_sockets[ PLAYER_1_INDEX ];
}

void gcol_remove( int uid ) {

    pthread_mutex_lock( &gcol_mx );
    game_environment * game_to_remove = gcol_games[ uid ];
    int other = get_other_uid( game_to_remove, uid );
    gcol_games[ uid ] = NULL;
    gcol_games[ other ] = NULL;
    struct timespec timeout = {
            0, 200000
    };
    nanosleep(&timeout, NULL);
    close(game_to_remove->game_sockets[PLAYER_1_INDEX]);
    close(game_to_remove->game_sockets[PLAYER_2_INDEX]);
    game_to_remove->game.destruct_game( game_to_remove );
    game_to_remove->voice_sockets[ PLAYER_1_INDEX ].sin_port = NO_PORT;
    game_to_remove->voice_sockets[ PLAYER_2_INDEX ].sin_port = NO_PORT;
    free( game_to_remove );
    pthread_mutex_unlock( &gcol_mx );

}

game_environment * gcol_get( int uid ) {
    return gcol_games[ uid ];
}

void gcol_lock() {
    pthread_mutex_lock( &gcol_mx );
}

void gcol_unlock() {
    pthread_mutex_unlock( &gcol_mx );
}

struct gcol game_collection = {
        gcol_init,
        gcol_create,
        gcol_remove,
        gcol_get,
        gcol_lock,
        gcol_unlock
};
