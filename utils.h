/**
* karel on 2021-02-11.
*/

#ifndef THE_ACTUAL_FSN_UTILS_H
#define THE_ACTUAL_FSN_UTILS_H

#include <dc_utils/dlinked_list.h>
#include "tic_tac_toe.h"
#include "stdbool.h"


typedef struct {
    int port;
    int sin_family;
    int addr;
    int listen_sock;
} server_config;

#define DEFAULT_PORT 3000


#define MIN_FD 4
#define MAX_CONN 100
#define NO_FD (-1)

bool user_won( const int game_board[CELLS_IN_GRID] );

void init_env( game_env * g_env );

#define COMM_OK 1

int await_player_connection( int listening_sock );

int await_move_msg( int player_conn, uint8_t msg_buffer[SERVER_BOUND_MSG_LENGTH] );

#define NOTIF_PLAYER_1 1 // 0b00000001
#define NOTIF_PLAYER_2 2 //0b00000010
#define NOTIF_ALL      3 //0b00000011
#define NOTIF_OK 0
#define NOTIF_DISCONN -1
#define NOTIF_SERVER_ERR -2


void get_socket( server_config * server_cfg );

/**
 * Send msg to players.
 *
 * @return NOTIF_OK | NOTIF_DISCONN | NOTIF_SERVER_ERR
 */
int send_game_state( message * msg, int conn_fds[PLAYERS_IN_GAME] );

void set_game_state_msg( message * msg, int target, const uint8_t content[MAX_MSG_LEN] );

void set_player_info_msg( message * msg, int target, const uint8_t content[MAX_MSG_LEN] );

void print_board( game_board_s game_board );

void print_message( message msg );

void print_env_common( Environment * env );

void print_game_env_not_common( game_env * env );

void print_game_env( Environment * env );

sem_t * open_sem(char* name);

void lock_mx(pthread_mutex_t * mutex);

void unlock_mx(pthread_mutex_t * mutex);

void init_fd_game_set(fd_game_set fd_game_container[MAX_CONN]);

void init_fd_set( const fd_game_set * fd_game, fd_set * client_fd_set, pthread_mutex_t * mx );

int await_select( fd_set * client_fd_set );

void serve_fds( fd_game_set * fd_game, fd_set * client_fd_set, int select_return, const StateTransition transitions[], accept_threads_args_s * args);

int setup_game_player(game_env *env, uint8_t player_num);

_Noreturn void * make_games( void * params );

void add_game_to_set(accept_threads_args_s * accept_thread_args, game_env* env);

_Noreturn void * accept_conns( void * params );

#endif //THE_ACTUAL_FSN_UTILS_H
