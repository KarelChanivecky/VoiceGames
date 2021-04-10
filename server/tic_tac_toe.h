/**
* karel on 2021-02-12.
*/

#ifndef THE_ACTUAL_FSN_TIC_TAC_TOE_H
#define THE_ACTUAL_FSN_TIC_TAC_TOE_H

#include <stdbool.h>
#include <semaphore.h>
#include <dc/semaphore.h>

/*
 * PROTOCOL SPECIFIED DEFINITIONS
 */

/**
 * Contains the possible game_state codes which are sent to the client. <br>
 *
 * See the attached BNF <br>
 *
 * The first nine codes represent a cell in the tic-tac-toe grid,
 * where R represents row, C represents column.
 */
typedef enum {
    R0_C0,
    R0_C1,
    R0_C2,
    R1_C0,
    R1_C1,
    R1_C2,
    R2_C0,
    R2_C1,
    R2_C2,
    QUIT,

    WON = 64,
    LOST,
    TIE,
    ERR_ADVERSARY_DISCONNECTED,

    ERR_INTERNAL_SERVER = 128,
    ERR_OUT_OF_TURN,
    ERR_POSITION_TAKEN,
    ERR_INVAL_CHOICE,
    ERR_INVAL_ID,
    ERR_UNEXPECTED_MSG_LENGTH,
    COMM_PING,
    NO_CODE // for internal use only
} game_states;
#define X_SYM 0
#define Y_SYM 1
#define DEFAULT_PLAYER_ID 0

#define GAME_STATE_MSG_LENGTH 1
#define PLAYER_INFO_MSG_LENGTH 2
#define SERVER_BOUND_MSG_LENGTH 2

#define GAME_STATE_MSG_INDEX 0
#define PLAYER_INFO_MSG_SYM_INDEX 1
#define USER_MOVE_INDEX 1

#define GAME_EXIT 9
#define GAME_CONTINUE 10


/*
 * IMPLEMENTATION SPECIFIC DEFINITIONS
 */

#define PLAYER_1_INDEX 0
#define PLAYER_2_INDEX 1
#define PLAYERS_IN_GAME 2
#define CELLS_IN_GRID 9

enum board_ticks {
    FREE_CELL ,
    P1,
    P2
};

#define PORT_PARAM_GIVEN 2
#define MIN_PORT 1024
#define MAX_PORT 65535
#define NO_CONN -2
#define INCORRECT_MSG_LENGTH -3
#define MAX_MSG_LEN 2
#define AWAIT_EXIT 254
#define AWAIT_CONTINUE 255

#define GAME_END_NO_RESPONSE 0
#define GAME_END_ONE_RESPONSE 1

typedef enum {
    INITIALIZE_ENV = FSM_APP_STATE_START,
    AWAIT_PLAYER,
    SETUP_GAME,
    AWAIT_MOVE,
    VALIDATE_MOVE,
    EVALUATE_STATE,
    PLAYER_WON,
    PLAYER_TIED,
    PLAYER_DISCONNECTED,
    PLAYER_QUIT,
    NOTIFY,
    INTERNAL_SERVER_ERROR,
} fsn_states;

typedef struct {
    int cells[CELLS_IN_GRID];
    int cells_taken;
} game_board_s;

typedef struct {
    uint8_t content[MAX_MSG_LEN];
    int target;
    int len;
    bool status;
} message;


typedef struct {
    Environment common;
    game_board_s game_board;
    int conn_fds[PLAYERS_IN_GAME];
    bool player_1_is_X;
    uint8_t prev_state_code;
    bool X_turn;
    message msg;
    int listen_sock;
    int game_end_response_num;
} game_env;

typedef struct {
    int fd;
    game_env * game;
} fd_game_set;

typedef struct {
    pthread_mutex_t queue_mx;
    sem_t * queue_sem;
    dlinked_list * queue;
} queue_args_s;

typedef struct {
    int server_fd;
    pthread_mutex_t fd_mx;
    fd_set * client_fd_set;
    fd_game_set * fds_games;
    sem_t * game_sem;
} fd_set_args;

typedef struct {
    int server_fd;
    queue_args_s * queue_arg;
    sem_t * game_sem;
} make_game_args_s;

typedef struct {
    int server_fd;
    pthread_mutex_t fd_mx;
    queue_args_s * queue_arg;
    sem_t * game_sem;
    fd_game_set * fds_games;
} accept_threads_args_s;

#endif //THE_ACTUAL_FSN_TIC_TAC_TOE_H
