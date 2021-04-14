

#ifndef GAME_PROTOCOL_LIBRARY_H
#define GAME_PROTOCOL_LIBRARY_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef PLAYER_DISCONNECTED
#define PLAYER_DISCONNECTED -20
#endif

#ifndef INTERNAL_ERR
#define INTERNAL_ERR -25
#endif

#ifndef INSUFFICIENT_DATA
#define INSUFFICIENT_DATA -30
#endif

#ifndef INCOMPLETE_WRITE
#define INCOMPLETE_WRITE -35
#endif

#ifndef MALLOC_ERR
#define MALLOC_ERR -40
#endif

#ifndef INVALID_REQ
#define INVALID_REQ -45
#endif

#ifndef GPROT_OK
#define GPROT_OK true
#endif

#define GPROT_PAYLOAD_GAME_INDEX 1

typedef struct {
    uint32_t uid;
    uint8_t type;
    uint8_t context;
    uint8_t payload_length;
    uint8_t * payload;
} Request;

typedef struct {
    uint8_t status;
    uint8_t type;
    uint8_t payload_length;
    uint8_t * payload;
} Response;

typedef struct {
    uint8_t status;
    uint8_t context;
    uint8_t payload_length;
    uint8_t * payload;
} Update;

static struct {
    const int TYPE;
    const int CONTEXT;
    const int PAYLOAD_LENGTH;
} const ReqIndexes = {
        4, 5, 6
};

static struct {
    const int GAME_CHOICE;
} PayloadLengths = {
        2
};

typedef struct {
    const int STATUS;
    const int REQ_TYPE;
    const int PAYLOAD_LENGTH;
} s_res_index;

static s_res_index ResIndexes = {
        0, 1, 2
};

static const struct {
    const int STATUS;
    const int CONTEXT;
    const int PAYLOAD_LENGTH;
} UpdIndexes = { 0, 1, 2 };


enum req_types {
    REQ_CONFIRMATION = 1,
    REQ_INFORMATION,
    REQ_META,
    REQ_ACTION
};

enum statuses {
    ST_SUCCESS = 10,
    ST_UPDATE = 20,
    ST_INVALID_REQ = 30,
    ST_INVALID_UID,
    ST_INVALID_TYPE,
    ST_INVALID_CONTEXT,
    ST_INVALID_PAYLOAD,
    ST_SERVER_ERROR = 40,
    ST_INVALID_ACTION = 50,
    ST_OUT_OF_TURN = 51
};

enum req_conf_contexts {
    CONF_RULESET = 1,
};

enum req_meta_contexts {
    META_QUIT = 1
};

enum req_action_contexts {
    ACT_MAKE_MOVE = 1
};

enum upd_contexts {
    UPD_START_GAME = 1,
    UPD_MOVE_MADE,
    UPD_END_OF_GAME,
    UPD_OPP_DISCONNECTED
};

bool set_games_supported( uint8_t * game_ids );

int respond_success( int client, uint8_t req_type );

int respond_success_payload( int client, uint8_t req_type, uint8_t * payload, size_t payload_size );

int respond_invalid_type( int client, uint8_t req_type );

int respond_invalid_context( int client, uint8_t req_type );

int respond_invalid_payload( int client, uint8_t req_type );

int respond_server_error( int client, uint8_t req_type );

int respond_invalid_uid( int client, uint8_t req_type );

int respond_uid( int client, uint32_t uid );

int respond_invalid_action( int client, uint8_t req_type );

int respond_out_of_turn( int client, uint8_t req_type );

int update_start_game( int client );

int update_start_game_with_payload( int client, size_t payload_length, uint8_t * payload );

int update_move( int client, uint8_t * move, size_t move_size );

int update_end_game( int client, uint8_t * payload, size_t payload_len );

int update_opp_disconn( int client );

int read_game_choice( int client, Request * req );

int read_move( int client, Request * req );

#endif //GAME_PROTOCOL_LIBRARY_H
