/**
* karel on 2021-02-11.
*/

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <dc/sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <dc/semaphore.h>
#include <dcfsm/fsm.h>
#include <dc_utils/dlinked_list.h>
#include <dc/stdlib.h>
#include "utils.h"

#define ROW_DIFF 3
#define COL_DIFF 1
#define FD_DIFF 2
#define BD_DIFF 4

/**
 * Evaluate if there is a winning condition.
 *  <br>
 * 4 ways to win:
 * <br>
 * - all cells in row<br>
 * - all cells in col<br>
 * - forward-slash diagonal i.e. : / <br>
 * - back-slash diagonal i.e. : \
 *
 * @param game_board A 9-cell Tic-Tac-Toe board represented as an int array, where the top-left cell is index 0
 *                      and the bottom-right is index 8
 *
 * @param starting_cell The cell from which to start evaluating the winning condition.
 *                      Specifically provided to evaluate forward-slash winning condition. <br>
 *                      Pass:<br>
 *                      - R0_C2 for back-slash diagonal <br>
 *                      - R0_C0 otherwise
 *
 * @param winning_cell_diff Used to select the cells that would create a winning condition.
 *                  The number of cells to skip over in the linear array. <br>
 *                  - COL_DIFF for evaluating rows <br>
 *                  - ROW_DIFF for evaluating columns <br>
 *                  - FD_DIFF for evaluating forward-slash diagonal <br>
 *                  - BD_DIFF for evaluating backward-slash diagonal
 *
 * @param displacement_cell_diff The number of cells to displace in order to re-evaluate winning condition in case that
 *                                 a winning condition is not found at certain position.<br>
 *                  Pass: <br>
 *                  - COL_DIFF for evaluating columns<br>
 *                  - ROW_DIFF for evaluating rows<br>
 *                  - CELLS_IN_GRID for evaluating both diagonals
 * @return true if any of the winning conditions are met
 */
bool eval_win_condition( const int game_board[CELLS_IN_GRID],
                         int starting_cell,
                         int ending_cell,
                         int winning_cell_diff,
                         int displacement_cell_diff ) {
    for ( int cell = starting_cell; cell <= ending_cell; cell += displacement_cell_diff ) {

        int cell1 = game_board[ cell ];
        int cell2 = game_board[ cell + winning_cell_diff ];
        int cell3 = game_board[ cell + winning_cell_diff * 2 ];

        if (cell1 == FREE_CELL || cell2 == FREE_CELL) {
            continue;
        }

        if ( cell1 == cell2  && cell2 == cell3 ) {
            return true;
        }
    }
    return false;
}

bool won_row( const int game_board[CELLS_IN_GRID] ) {
    return eval_win_condition( game_board, R0_C0, R2_C0, COL_DIFF, ROW_DIFF );
}

bool won_col( const int game_board[CELLS_IN_GRID] ) {
    return eval_win_condition( game_board, R0_C0, R0_C2, ROW_DIFF, COL_DIFF );
}

bool won_f_diag( const int game_board[CELLS_IN_GRID] ) {
    return eval_win_condition( game_board, R0_C2, R1_C0, FD_DIFF, CELLS_IN_GRID );
}

bool won_b_diag( const int game_board[CELLS_IN_GRID] ) {
    return eval_win_condition( game_board, R0_C0, R0_C1, BD_DIFF, CELLS_IN_GRID );
}

bool user_won( const int game_board[CELLS_IN_GRID] ) {
    return won_row( game_board )
    || won_col( game_board )
    || won_f_diag( game_board )
    || won_b_diag( game_board );
}

static void init_game_board( game_board_s * game_board ) {
    game_board->cells_taken = 0;
    for ( int cell = R0_C0; cell <= R2_C2; cell++ ) {
        game_board->cells[ cell ] = FREE_CELL;
    }
}

void init_env(game_env * g_env) {
    // Upon returning to this state, we need to close the old connections
    // If close fails, assume the connections are closed
    // On the first time close will set errno to EBADF. Since this is expected behaviour
    // it is not necessary to check for errors
    g_env->conn_fds[ PLAYER_1_INDEX ] = NO_CONN;
    g_env->conn_fds[ PLAYER_2_INDEX ] = NO_CONN;
    g_env->X_turn = true;
    g_env->player_1_is_X = false;
    g_env->prev_state_code = AWAIT_EXIT;
    g_env->msg.len = 0;
    g_env->msg.target = 0;
    g_env->game_end_response_num = 0;
    init_game_board( &g_env->game_board );
}

void get_socket( server_config * server_cfg ) {
    int listen_socket_fd;
    struct sockaddr_in addr;
    listen_socket_fd = dc_socket( server_cfg->sin_family, SOCK_STREAM, 0 );

    int option = 1;
    int sock_opt_status = setsockopt( listen_socket_fd, SOL_SOCKET, SO_REUSEADDR,
                                      ( char * ) &option, sizeof( option ));
    if ( sock_opt_status == -1 ) {
        fprintf( stderr, "Error setting socket options!: %s", strerror(errno));
        exit( EXIT_FAILURE );
    }

    memset( &addr, 0, sizeof( struct sockaddr_in ));
    addr.sin_family = server_cfg->sin_family;
    addr.sin_port = htons( server_cfg->port );
    addr.sin_addr.s_addr = htonl( server_cfg->addr );
    dc_bind( listen_socket_fd, ( struct sockaddr * ) &addr, sizeof( struct sockaddr_in ));
    dc_listen( listen_socket_fd, MAX_CONN );
    server_cfg->listen_sock = listen_socket_fd;
}


int await_player_connection( int listening_sock ) {
    int new_conn_fd = accept( listening_sock, NULL, NULL);
    if ( new_conn_fd == -1 ) {
        perror( "COMM ERROR! @await connection. Problem with listening socket" );
    }
    return new_conn_fd;
}

static void print_move_msg(uint8_t * msg) {
    printf("Player sent: %d %d\n", msg[0], msg[USER_MOVE_INDEX]);
}

int await_move_msg( const int player_conn, uint8_t msg_buffer[SERVER_BOUND_MSG_LENGTH] ) {
    int bytes_read = read( player_conn, msg_buffer, SERVER_BOUND_MSG_LENGTH );
    if ( bytes_read <= 0 ) {
        if (errno == EBADF) {
            return ERR_ADVERSARY_DISCONNECTED;
        }
        return ERR_INTERNAL_SERVER;
    }
    print_move_msg(msg_buffer);
    return COMM_OK;
}


int evaluate_status( int status ) {
    printf("Notif status: %d\n", status);
    if ( status == 0 ) {
        printf("errno: %s\n", strerror(errno));
        return ERR_ADVERSARY_DISCONNECTED;
    }

    if ( status == -1 ) {
        printf("errno: %s", strerror(errno));
        return ERR_INTERNAL_SERVER;
    }
    return NOTIF_OK;
}


int send_game_state( message * msg, int conn_fds[PLAYERS_IN_GAME] ) {
    int status = 0;
    if ( msg->target & NOTIF_PLAYER_1 ) {
        puts( "Notifying player 1" );
        status = write( conn_fds[ PLAYER_1_INDEX ], msg->content, msg->len );

        if ( status <= 0 ) {
            return evaluate_status( status );
        }
    }

    if ( (msg->target & NOTIF_PLAYER_2) && !( msg->target & NOTIF_PLAYER_1 && status <= 0) ) {

        puts( "Notifying player 2" );
        status = write( conn_fds[ PLAYER_2_INDEX ], msg->content, msg->len );

    }

    return evaluate_status( status );
}


void set_msg( message * msg, const int target, const uint8_t content[MAX_MSG_LEN], int len ) {
    msg->len = len;
    msg->target = target;
    msg->content[ len - 1 ] = content[ len - 1 ];
}

void set_game_state_msg( message * msg, const int target, const uint8_t content[MAX_MSG_LEN] ) {
    set_msg( msg, target, content, GAME_STATE_MSG_LENGTH );
}

void set_player_info_msg( message * msg, const int target, const uint8_t content[MAX_MSG_LEN] ) {
    set_msg( msg, target, content, PLAYER_INFO_MSG_LENGTH );
}

void print_board( game_board_s game_board ) {
    printf( "   Board >\n"
            "       cells taken: %d\n"
            "       cell state >\n"
            "           %d | %d | %d\n"
            "           ---------\n"
            "           %d | %d | %d\n"
            "           ---------\n"
            "           %d | %d | %d\n",
            game_board.cells_taken,
            game_board.cells[ R0_C0 ], game_board.cells[ R0_C1 ], game_board.cells[ R0_C2 ],
            game_board.cells[ R1_C0 ], game_board.cells[ R1_C1 ], game_board.cells[ R1_C2 ],
            game_board.cells[ R2_C0 ], game_board.cells[ R2_C1 ], game_board.cells[ R2_C2 ] );
}

void print_message( message msg ) {
    printf( "message >\n"
            "   Content: %d | Target: %d | len: %d\n",
            msg.content[ msg.len ],
            msg.target, msg.len );
}

void print_env_common( Environment * env ) {
    printf( "Game Env >\n"
            "   From state: %d | Current State: %d\n",
            env->from_state_id, env->current_state_id );
}

void print_game_env_not_common( game_env * env ) {
    printf( "   Player 1 is X: %d | X turn: %d\n"
            "   Prev state code: %d\n", env->player_1_is_X, env->X_turn,
            env->prev_state_code );
}

void print_game_env( Environment * env ) {
    game_env * g_env = ( game_env * ) env;
    print_env_common( env );
    print_game_env_not_common( g_env );
    print_board( g_env->game_board );
    print_message( g_env->msg );
}

sem_t * open_sem(char* name) {
    int status = sem_unlink(name);
    if (status == -1 && errno != ENOENT) {
        perror("ERROR cannot unlink semaphore");
        exit(EXIT_FAILURE);
    }
    return sem_open(name, O_CREAT, 0640, 0);
}

void lock_mx(pthread_mutex_t * mutex) {
    int lock_status = pthread_mutex_lock( mutex );
    if ( lock_status != 0 ) {
        perror( "ERROR: mutex lock" );
        exit( EXIT_FAILURE );
    }
}

void unlock_mx(pthread_mutex_t * mutex) {
    int unlock_status = pthread_mutex_unlock( mutex );

    if ( unlock_status != 0 ) {
        perror( "ERROR: mutex unlock" );
        exit( EXIT_FAILURE );
    }
}

void add_player_to_queue(accept_threads_args_s *args, int fd){
    queue_args_s * queue_args = args->queue_arg;
    lock_mx(&queue_args->queue_mx);
    dlinked_push(queue_args->queue, (void *)((long)fd));
    int size = queue_args->queue->size;
    unlock_mx(&queue_args->queue_mx);
    if (size != 0 && size % 2 == 0){
        printf("2 in queue \n");
        dc_sem_post(queue_args->queue_sem);
    }
}

_Noreturn void * accept_conns( void * params ) {
    accept_threads_args_s *args = (accept_threads_args_s *) params;
    while(true){
        int new_conn = dc_accept( args->server_fd, NULL, NULL);
        printf("new conn: %d\n", new_conn);
        if ( new_conn == -1 ) {
            perror( "COMM ERROR! @await connection. Problem with listening socket" );
        }
        add_player_to_queue(args, new_conn);
    }

}

void init_fd_game_set(fd_game_set fd_game_container[MAX_CONN]) {
    for ( int i = 0; i < MAX_CONN ; ++i ) {
        fd_game_container[i].fd = NO_FD;
        fd_game_container[i].game = NULL;
    }
}

int setup_game_player(game_env *env, uint8_t player_num) {
    uint8_t player_symbol = player_num - 1;
    uint8_t other_player_num = (player_num == 1) ? NOTIF_PLAYER_2 : NOTIF_PLAYER_1;
    uint8_t other_player_symbol = other_player_num - 1;
    set_player_info_msg(&env->msg,
                        player_num,
                        (uint8_t[PLAYER_INFO_MSG_LENGTH]) {0, env->player_1_is_X ? player_symbol : other_player_symbol});

    int status = send_game_state( &env->msg, env->conn_fds );

    if (status == ERR_ADVERSARY_DISCONNECTED || status == ERR_INTERNAL_SERVER){
        set_game_state_msg(&env->msg,
                           other_player_num,
                           (uint8_t[GAME_STATE_MSG_LENGTH]) {status});
        send_game_state(&env->msg, env->conn_fds);
        free(env);
        return -1;
    }
    return 1;
}

void add_game_to_set(accept_threads_args_s * accept_thread_args, game_env* env) {
    int player_1_fd = env->conn_fds[PLAYER_1_INDEX];
    int player_2_fd = env->conn_fds[PLAYER_2_INDEX];
    lock_mx(&accept_thread_args->fd_mx);
    accept_thread_args->fds_games[player_1_fd].fd = player_1_fd;
    accept_thread_args->fds_games[player_1_fd].game = env;
    accept_thread_args->fds_games[player_2_fd].fd = player_2_fd;
    accept_thread_args->fds_games[player_2_fd].game = env;
    unlock_mx(&accept_thread_args->fd_mx);
    dc_sem_post(accept_thread_args->game_sem);
}

_Noreturn void * make_games( void * params ) {
    accept_threads_args_s * accept_thread_args = ( accept_threads_args_s * ) params;
    queue_args_s * queue_args = accept_thread_args->queue_arg;
    while (1) {
        dc_sem_wait(queue_args->queue_sem);
        game_env *env = (game_env *) dc_malloc(sizeof(game_env));
        init_env(env);

        env->common.from_state_id = SETUP_GAME;
        env->listen_sock = accept_thread_args->server_fd;
        lock_mx(&queue_args->queue_mx);
        env->conn_fds[PLAYER_1_INDEX] = (int) (long) dlinked_pop_head((queue_args->queue));
        env->conn_fds[PLAYER_2_INDEX] = (int) (long) dlinked_pop_head((queue_args->queue));
        unlock_mx(&queue_args->queue_mx);

        // setup_game
        env->player_1_is_X = rand() % 1;
        int status = setup_game_player(env, NOTIF_PLAYER_1);
        if (status == -1) continue;

        status = setup_game_player(env, NOTIF_PLAYER_2);
        if (status == -1) continue;

        add_game_to_set(accept_thread_args, env);
    }
}

void init_fd_set( const fd_game_set fd_game[MAX_CONN], fd_set * client_fd_set, pthread_mutex_t * mx ) {
    lock_mx(mx);
    FD_ZERO(client_fd_set);
    for ( int i = MIN_FD; i < MAX_CONN; ++i ) {
        if ((fd_game[i]).fd != NO_FD) {
            FD_SET((fd_game[i]).fd, client_fd_set);
        }
    }
    unlock_mx(mx);
}

int await_select( fd_set * client_fd_set ) {
    int retval = select( MAX_CONN + 1, client_fd_set, NULL, NULL, NULL);
    puts("selected conn");

    if ( retval == -1 ) {
        perror( "select" );
        exit( EXIT_FAILURE );
    }

    return retval;
}

void serve_fds( fd_game_set * fd_game, fd_set * client_fd_set, int select_return, const StateTransition transitions[], accept_threads_args_s * args) {
    for ( int i = MIN_FD; i < MAX_CONN; ++i ) {
        if ((fd_game[i]).fd == NO_FD) {
            continue;
        }
        if (FD_ISSET( (fd_game[i]).fd, client_fd_set)) {
            game_env * game = fd_game[i].game;
            if (game->prev_state_code == AWAIT_EXIT) {
                bool player_1_turn = !( game->X_turn ^ game->player_1_is_X );
                if(player_1_turn){
                    if(i != game->conn_fds[PLAYER_1_INDEX]){
                        continue;
                    }
                }
                else {
                    if (i != game->conn_fds[PLAYER_2_INDEX]) {
                        continue;
                    }
                }
                game->prev_state_code = AWAIT_CONTINUE;
                int from_id = AWAIT_MOVE;
                fsm_run((Environment *)game, &(game->common.from_state_id), &from_id, transitions);
                dc_sem_post(args->game_sem);
            }
            else{
                printf("here %d\n", game->prev_state_code);
                if (game->game_end_response_num == GAME_END_NO_RESPONSE) { // no player submitted response yet
                    int player_fd = fd_game[i].fd;
                    int responded_player_num = (game->conn_fds[PLAYER_1_INDEX] == player_fd) ? PLAYER_1_INDEX
                                                                                             : PLAYER_2_INDEX;
                    int other_player_num = 1 - responded_player_num;
                    int other_player_fd = game->conn_fds[other_player_num];

                    uint8_t response[SERVER_BOUND_MSG_LENGTH];

                    int state = await_move_msg(player_fd, response);

                    if (state != ERR_INTERNAL_SERVER && state != ERR_ADVERSARY_DISCONNECTED &&
                        response[USER_MOVE_INDEX] == GAME_CONTINUE) {
                        add_player_to_queue(args, player_fd);
                    } else FD_CLR(player_fd, client_fd_set);


                    fd_game[player_fd].fd = NO_FD;
                    fd_game[player_fd].game = NULL;

                    const uint8_t ping[1] = {0};
                    int status = write(other_player_fd, ping, 0);
                    if (status == -1){
                        FD_CLR(other_player_fd, client_fd_set);
                        fd_game[other_player_fd].fd = NO_FD;
                        free(fd_game[player_fd].game);
                        fd_game[other_player_fd].game = NULL;
                        continue;
                    }

                    dc_sem_post(args->game_sem);
                    (game->game_end_response_num)++;

                } else {
                    int player_fd = fd_game[i].fd;
                    uint8_t response[SERVER_BOUND_MSG_LENGTH];
                    int state = await_move_msg(player_fd, response);
                    if (state != ERR_INTERNAL_SERVER && state != ERR_ADVERSARY_DISCONNECTED &&
                        response[USER_MOVE_INDEX] == GAME_CONTINUE) {
                        add_player_to_queue(args, player_fd);
                    } else FD_CLR(player_fd, client_fd_set);

                    fd_game[player_fd].fd = NO_FD;
                    free(fd_game[player_fd].game);
                    fd_game[player_fd].game = NULL;
                }
            }

        }
    }

}

