/**
* karel on 2021-02-10.
*/

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <dcfsm/fsm.h>
#include <dc/pthread.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dc/stdlib.h>
#include <dc/unistd.h>

#include "utils.h"
#include "tic_tac_toe.h"

#define GAME_SEM_NAME "/game_sem"
#define QUEUE_SEM_NAME "/queue_sem"

static int initialize_env( Environment * env );

static int await_player( Environment * env );

static int setup_game( Environment * env );

static int await_move( Environment * env );

static int validate_move( Environment * env );

static int evaluate_state( Environment * env );

static int player_won( Environment * env );

static int player_tied( Environment * env );

static int player_disconnected( Environment * env );

static int player_quit( Environment * env );

static int internal_server_error( Environment * env );

static int notify( Environment * env );

static int notify_tie_or_win( Environment * env ); // not a state. A helper

int main( int argc, char * argv[] ) {


    server_config server_cfg;

    if ( argc != PORT_PARAM_GIVEN ) {
        server_cfg.port = DEFAULT_PORT;
    } else {
        int port_number = ( int ) strtol( argv[ 1 ], NULL, 10 );
        if ( port_number < MIN_PORT || MAX_PORT < port_number ) {
            fprintf( stderr,
                     "Invalid port number entered! This application only accepts: %d < port < %d\n",
                     MIN_PORT, MAX_PORT );
            exit( EXIT_FAILURE );
        }
        server_cfg.port = port_number;
    }
    server_cfg.sin_family = AF_INET;
    server_cfg.addr = htonl(INADDR_ANY);

    get_socket( &server_cfg );

    fd_set rfds;
    fd_game_set fd_game[MAX_CONN];

    init_fd_game_set(fd_game);

    pthread_mutex_t fd_mx = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t queue_mx = PTHREAD_MUTEX_INITIALIZER;
    sem_t * game_sem = open_sem(GAME_SEM_NAME);
    sem_t * queue_sem = open_sem(QUEUE_SEM_NAME);
    dlinked_list * players_queue = dlinked_create_list();

    queue_args_s * accept_queue = (queue_args_s * ) dc_malloc(sizeof(queue_args_s));
//    queue_args_s accept_queues = {queue_mx, queue_sem, players_queue};
    accept_queue->queue_mx = queue_mx;
    accept_queue->queue_sem = queue_sem;
    accept_queue->queue = players_queue;

    accept_threads_args_s * thread_args = (accept_threads_args_s * ) dc_malloc(sizeof(accept_threads_args_s));
    thread_args->server_fd = server_cfg.listen_sock;
    thread_args->game_sem = game_sem;
    thread_args->queue_arg = accept_queue;
    thread_args->fd_mx = fd_mx;
    thread_args->fds_games = fd_game;

//    make_game_args_s * game_args = (make_game_args_s * ) dc_malloc(sizeof(make_game_args_s));
//    game_args->server_fd = server_cfg.listen_sock;
//    game_args->fd_mx = fd_mx;
//    game_args->client_fd_set = &rfds;
//    game_args->fds_games = fd_game;
//    accept_thread_args->game_sem = game_sem;
//    accept_thread_args->queue_arg = accept_queue;

    pthread_t acceptor_thread;
    pthread_t listener_thread;

    dc_pthread_create(&acceptor_thread, NULL, accept_conns, thread_args);

    dc_pthread_create(&listener_thread, NULL, make_games, thread_args);

    srand( time(NULL));

//    game_env env;
//
//    init_env( &env );
//
//    env.listen_sock = server_cfg.listen_sock;


    StateTransition transitions[] = {
            { FSM_INIT,              INITIALIZE_ENV,        &initialize_env },
            { INITIALIZE_ENV,        AWAIT_PLAYER,          &await_player },
            { AWAIT_PLAYER,          AWAIT_PLAYER,          &await_player },
            { AWAIT_PLAYER,          SETUP_GAME,            &setup_game },
            { SETUP_GAME,            NOTIFY,                &notify },
            { NOTIFY,                SETUP_GAME,            &setup_game },
            { SETUP_GAME,            AWAIT_MOVE,            &await_move },
            { AWAIT_MOVE,            AWAIT_MOVE,            &await_move },
            { AWAIT_MOVE,            VALIDATE_MOVE,         &validate_move },
            { AWAIT_MOVE,            NOTIFY,                &notify },
            { AWAIT_MOVE,            PLAYER_DISCONNECTED,   &player_disconnected },
            { NOTIFY,                AWAIT_MOVE,            &await_move },
            { VALIDATE_MOVE,         PLAYER_DISCONNECTED,   &player_disconnected },
            { VALIDATE_MOVE,         NOTIFY,                &notify },
            { NOTIFY,                VALIDATE_MOVE,         &validate_move },
            { VALIDATE_MOVE,         AWAIT_MOVE,            &await_move },
            { VALIDATE_MOVE,         EVALUATE_STATE,        &evaluate_state },
            { EVALUATE_STATE,        AWAIT_MOVE,            &await_move },
            { EVALUATE_STATE,        PLAYER_WON,            &player_won },
            { EVALUATE_STATE,        PLAYER_TIED,           &player_tied },
            { EVALUATE_STATE,        PLAYER_QUIT,           &player_quit },
            { EVALUATE_STATE,        INTERNAL_SERVER_ERROR, &internal_server_error },
            { EVALUATE_STATE,        NOTIFY,                &notify },
            { NOTIFY,                EVALUATE_STATE,        &evaluate_state },
            { PLAYER_WON,            NOTIFY,                &notify },
            { PLAYER_WON,            INITIALIZE_ENV,        &initialize_env },
            { NOTIFY,                PLAYER_WON,            &player_won },
            { PLAYER_TIED,           NOTIFY,                &notify },
            { PLAYER_TIED,           INITIALIZE_ENV,        &initialize_env },
            { NOTIFY,                PLAYER_TIED,           &player_tied },
            { PLAYER_DISCONNECTED,   NOTIFY,                &notify },
            { PLAYER_DISCONNECTED,   INITIALIZE_ENV,        &initialize_env },
            { NOTIFY,                PLAYER_DISCONNECTED,   &player_disconnected },
            { PLAYER_QUIT,           NOTIFY,                &notify },
            { PLAYER_QUIT,           INITIALIZE_ENV,        &initialize_env },
            { NOTIFY,                PLAYER_QUIT,           &player_quit },
            { INTERNAL_SERVER_ERROR, NOTIFY,                &notify },
            { NOTIFY,                INTERNAL_SERVER_ERROR, &internal_server_error },
            { INTERNAL_SERVER_ERROR, FSM_EXIT,   NULL },
            { FSM_IGNORE,            FSM_IGNORE, NULL },
    };
//    int code;
//    int start_state;
//    int end_state;
//
//    start_state = FSM_INIT;
//    end_state = INITIALIZE_ENV;
//    code = fsm_run(( Environment * ) &env, &start_state, &end_state, transitions );
//
//    if ( code != 0 ) {
//        fprintf( stderr, "Cannot move from %d to %d\n", start_state, end_state );
//
//        return EXIT_FAILURE;
//    }

    while ( 1 ){
        dc_sem_wait(game_sem);
        init_fd_set(fd_game, &rfds, &fd_mx);
        int return_val = await_select(&rfds);
        serve_fds(fd_game, &rfds, return_val, transitions, thread_args);
    }

    return EXIT_SUCCESS;
}

/**
 * Initialize variables.
 */
static int initialize_env( Environment * env ) {
    puts( "\nEntered: Initialize env" );
    print_game_env( env );
    init_env(( game_env * ) env );
    return AWAIT_PLAYER;
}

/**
 * Await players.
 */
static int await_player( Environment * env ) {

    puts( "\nEntering await player state" );

    game_env * g_env = ( game_env * ) env;

    int new_conn = await_player_connection( g_env->listen_sock );

    if ( g_env->conn_fds[ PLAYER_1_INDEX ] == NO_CONN ) {
        g_env->conn_fds[ PLAYER_1_INDEX ] = new_conn;
        return AWAIT_PLAYER;
    }

    g_env->conn_fds[ PLAYER_2_INDEX ] = new_conn;

    return SETUP_GAME;
}

/**
 * Setup game.
 *
 * Randomly assign symbols to each players, and communicate them.
 */
static int setup_game( Environment * env ) {
    puts( "\nEntering setup game" );
    game_env * g_env = ( game_env * ) env;

    if ( env->from_state_id == NOTIFY ) {
        if ( g_env->msg.target == NOTIF_PLAYER_2 ) {
            return AWAIT_MOVE;
        }
        set_player_info_msg( &g_env->msg,
                             NOTIF_PLAYER_2,
                             ( uint8_t[PLAYER_INFO_MSG_LENGTH] ) { 0, g_env->player_1_is_X ? Y_SYM : X_SYM } );
        return NOTIFY;
    }


    g_env->player_1_is_X = rand() % 1;


    set_player_info_msg( &g_env->msg,
                         NOTIF_PLAYER_1,
                         ( uint8_t[PLAYER_INFO_MSG_LENGTH] ) { 0, g_env->player_1_is_X ? X_SYM : Y_SYM } );

    return NOTIFY;
}

/**
 * Await to read player move.
 */
static int await_move( Environment * env ) {
    puts( "\nEntering await move state" );

    print_env_common( env );
    game_env * g_env = ( game_env * ) env;
    if(g_env->prev_state_code == AWAIT_EXIT){
        printf("1777\n");
        return FSM_EXIT;
    }
    puts( "\nEntering await move state" );

    print_env_common( env );

    uint8_t move[SERVER_BOUND_MSG_LENGTH];

    bool player_1_turn = !( g_env->X_turn ^ g_env->player_1_is_X );

    int player_index = player_1_turn ? PLAYER_1_INDEX : PLAYER_2_INDEX;
    int state;

    state = await_move_msg( g_env->conn_fds[ player_index ], move );

    if ( state == ERR_INTERNAL_SERVER ) {
        return INTERNAL_SERVER_ERROR;
    }

    if ( state == ERR_ADVERSARY_DISCONNECTED ) {
        return PLAYER_DISCONNECTED;
    }

    g_env->prev_state_code = move[ USER_MOVE_INDEX ];
    return VALIDATE_MOVE;

}

/**
 * Validate move.
 *
 * Reasons a move could be invalid:
 * - cell is already taken
 * - choice is out of range
 */
static int validate_move( Environment * env ) {

    puts( "\nEntering validate move" );

    game_env * g_env = ( game_env * ) env;

    print_game_env_not_common( g_env );

    if ( env->from_state_id == NOTIFY ) {
        // Only reason to notify user in this state is that user made a bad move
        g_env->prev_state_code = AWAIT_EXIT;
        return AWAIT_MOVE;
    }

    int player_choice = g_env->prev_state_code;

    bool player_1_turn = !( g_env->X_turn ^ g_env->player_1_is_X );


    if ( QUIT < player_choice ) {
        // out of range
        puts( "Received invalid move" );
        set_game_state_msg( &g_env->msg,
                            player_1_turn ? NOTIF_PLAYER_1 : NOTIF_PLAYER_2,
                            ( uint8_t[GAME_STATE_MSG_LENGTH] ) { ERR_INVAL_CHOICE } );
        return NOTIFY;
    }

    if ( QUIT == player_choice ){
        return EVALUATE_STATE;
    }

    if ( g_env->game_board.cells[ player_choice ] != FREE_CELL ) {
        // cell is taken
        puts( "Received move to take cell already taken" );
        print_board( g_env->game_board );
        print_game_env_not_common( g_env );
        set_game_state_msg( &g_env->msg,
                            player_1_turn ? NOTIF_PLAYER_1 : NOTIF_PLAYER_2,
                            ( uint8_t[GAME_STATE_MSG_LENGTH] ) { ERR_POSITION_TAKEN } );
        return NOTIFY;
    }

    return EVALUATE_STATE;
}

static int evaluate_state( Environment * env ) {
    puts( "\nEntering evaluate state" );
    print_game_env( env );

    game_env * g_env = ( game_env * ) env;

    if ( env->from_state_id == NOTIFY ) {
        g_env->X_turn = !g_env->X_turn;
        g_env->prev_state_code = AWAIT_EXIT;
        return AWAIT_MOVE;
    }


    uint8_t player_choice = g_env->prev_state_code;

    if ( player_choice == QUIT ) {
        return PLAYER_QUIT;
    }

    bool player_1_turn = !( g_env->X_turn ^ g_env->player_1_is_X );

    g_env->game_board.cells[ player_choice ] = player_1_turn ? P1 : P2;
    g_env->game_board.cells_taken++;

    if ( user_won( g_env->game_board.cells )) {
        puts( "Evaluated player won" );
        return PLAYER_WON;
    }

    if ( g_env->game_board.cells_taken == CELLS_IN_GRID ) {
        puts( "Evaluated player tie" );
        return PLAYER_TIED;
    }

    set_game_state_msg( &g_env->msg,
                        player_1_turn ? NOTIF_PLAYER_2 : NOTIF_PLAYER_1,
                        &player_choice );

    return NOTIFY;
}


static int player_won( Environment * env ) {
    puts( "\nEntering player won" );
    print_env_common( env );
    return notify_tie_or_win( env );
}

static int player_tied( Environment * env ) {
    puts( "\nEntering player tied" );
    print_env_common( env );
    return notify_tie_or_win( env );
}

static int player_disconnected( Environment * env ) {
    puts( "\nEntering player disconnected" );

    game_env * g_env = ( game_env * ) env;

    if ( g_env->msg.content[ GAME_STATE_MSG_INDEX ] == ERR_ADVERSARY_DISCONNECTED ) {
        printf("5\n");
        return FSM_EXIT;
    }

    bool player_1_moved = !( g_env->X_turn ^ g_env->player_1_is_X );

    set_game_state_msg( &g_env->msg,
                        player_1_moved ? NOTIF_PLAYER_2 : NOTIF_PLAYER_1,
                        ( uint8_t[GAME_STATE_MSG_LENGTH] ) { ERR_ADVERSARY_DISCONNECTED } );

    return NOTIFY;
}

static int player_quit( Environment * env ) {
    puts( "\nEntering player quit" );
    if ( env->from_state_id == NOTIFY ) {
        printf("4\n");
        return FSM_EXIT;
    }

    game_env * g_env = ( game_env * ) env;
    bool player_1_moved = !( g_env->X_turn ^ g_env->player_1_is_X );

    set_game_state_msg( &g_env->msg,
                        player_1_moved ? NOTIF_PLAYER_2 : NOTIF_PLAYER_1,
                        ( uint8_t[GAME_STATE_MSG_LENGTH] ) { QUIT } );

    return NOTIFY;
}

static int internal_server_error( Environment * env ) {
    puts( "\nEntering internal server error" );
    game_env * g_env = ( game_env * ) env;

    if ( g_env->msg.content[ GAME_STATE_MSG_INDEX ] == INTERNAL_SERVER_ERROR ) {
        printf("3\n");
        return FSM_EXIT;
    }
    bool player_1_moved = !( g_env->X_turn ^ g_env->player_1_is_X );

    set_game_state_msg( &g_env->msg,
                        player_1_moved ? NOTIF_PLAYER_2 : NOTIF_PLAYER_1,
                        ( uint8_t[GAME_STATE_MSG_LENGTH] ) { ERR_INTERNAL_SERVER } );

    printf("2\n");
    return FSM_EXIT;
}

static int notify( Environment * env ) {
    puts( "\nEntering notify" );
    print_env_common( env );

    game_env * g_env = ( game_env * ) env;

    print_game_env_not_common( g_env );
    print_message( g_env->msg );

    int status = send_game_state( &g_env->msg, g_env->conn_fds );

    switch ( status ) {
        case NOTIF_DISCONN:
            return PLAYER_DISCONNECTED;
        case NOTIF_SERVER_ERR:
            return INTERNAL_SERVER_ERROR;
    }

    return g_env->common.from_state_id;
}

/*
 * Wrapper for common code
 *
 * Takes into account that it is not possible to win without moving
 *
 */
static int notify_tie_or_win( Environment * env ) {
    game_env * g_env = ( game_env * ) env;

    bool player_1_moved = !( g_env->X_turn ^ g_env->player_1_is_X );


    uint8_t mover_msg[GAME_STATE_MSG_LENGTH]; // the msg to the player who just moved
    uint8_t awaiter_msg[GAME_STATE_MSG_LENGTH]; // msg to the counterpart
    if ( env->current_state_id == PLAYER_TIED || env->from_state_id == PLAYER_TIED ) {
        mover_msg[ GAME_STATE_MSG_INDEX ] = TIE;
        awaiter_msg[ GAME_STATE_MSG_INDEX ] = TIE;
    } else {
        // someone has won
        mover_msg[ GAME_STATE_MSG_INDEX ] = WON;
        awaiter_msg[ GAME_STATE_MSG_INDEX ] = LOST;
    }


    if ( env->from_state_id == NOTIFY ) {
        // not the first time running this function state in this game
        puts( "Not first time notifying" );
        if (( player_1_moved ^ ( g_env->msg.target == NOTIF_PLAYER_1 ))) {
            // we have already notified mover

            if ( g_env->msg.content[ GAME_STATE_MSG_INDEX ] == TIE ||
                 g_env->msg.content[ GAME_STATE_MSG_INDEX ] == LOST ) {
                // We have just sent a notification to the awaiter that the game ended.
                // We must now forward the actual move
                puts( "Sending move to not mover" );
                set_game_state_msg( &g_env->msg, player_1_moved ? NOTIF_PLAYER_2 : NOTIF_PLAYER_1,
                                    ( uint8_t[GAME_STATE_MSG_LENGTH] ) { g_env->prev_state_code } );
                return NOTIFY;
            }

            // in this case we are done notifying
            printf("1\n");
            return FSM_EXIT;
        }

        puts( "Sending state to not mover" );
        set_game_state_msg( &g_env->msg, player_1_moved ? NOTIF_PLAYER_2 : NOTIF_PLAYER_1, awaiter_msg );

        return NOTIFY;
    }

    puts( "Notifying mover" );
    // send msg to mover
    set_game_state_msg( &g_env->msg, player_1_moved ? NOTIF_PLAYER_1 : NOTIF_PLAYER_2, mover_msg );

    return NOTIFY;
}
