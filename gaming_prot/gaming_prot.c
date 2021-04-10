#include "gaming_prot.h"
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define REQ_HEADER_SIZE 7
#define RES_HEADER_SIZE 3
#define UPD_HEADER_SIZE 3

#define UID_LEN 2

#define PAYLOAD_GAME_INDEX 1

#define PAYLOAD_MOVE_INDEX 0

static struct {
    const int TYPE;
    const int CONTEXT;
    const int PAYLOAD_LENGTH;
} const ReqIndexes = {
        4, 5, 6
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

static uint8_t * games_available = NULL;

int read_fd( int fd, uint8_t * buff, int bytes_c ) {
    int status = read( fd, buff, bytes_c );
    if ( status <= 0 ) {
        if ( errno == EBADF ) {
            return PLAYER_DISCONNECTED;
        }
        return INTERNAL_ERR;
    }
    if ( status < bytes_c ) {
        return INSUFFICIENT_DATA;
    }
    return status;
}

int write_fd( int fd, uint8_t * buff, int bytes_c ) {
    int stat = write( fd, buff, bytes_c );

    if ( stat == 0 ) {
        return PLAYER_DISCONNECTED;
    }

    if ( stat == -1 ) {
        return INTERNAL_ERR;
    }

    if ( stat < bytes_c ) {
        return INCOMPLETE_WRITE;
    }

    return stat;
}

int read_header( int client, Request * req, size_t header_size ) {
    uint8_t buff[header_size];
    int stat = read_fd( client, buff, header_size );
    if ( stat <= 0 ) {
        return stat;
    }

    req->uid = ntohs( *(( uint32_t * ) buff ));
    req->type = buff[ ReqIndexes.TYPE ];
    req->context = buff[ ReqIndexes.CONTEXT ];
    req->payload_length = buff[ ReqIndexes.PAYLOAD_LENGTH ];

    return stat;
}

int read_msg( int client, Request * req ) {
    int stat = read_header( client, req, REQ_HEADER_SIZE );

    if ( stat == -1 ) {
        return PLAYER_DISCONNECTED;
    }

    if ( req->payload_length == 0 ) {
        req->payload = NULL;
        return stat;
    }

    req->payload = ( uint8_t * ) malloc( sizeof( uint8_t ) * req->payload_length );

    if ( !req->payload ) {
        return INTERNAL_ERR;
    }

    return read_fd( client, req->payload, req->payload_length );
}

int respond( int client, Response * res ) {
    uint8_t buff[RES_HEADER_SIZE + res->payload_length];
    buff[ ResIndexes.STATUS ] = res->status;
    buff[ ResIndexes.REQ_TYPE ] = res->type;
    buff[ ResIndexes.PAYLOAD_LENGTH ] = res->payload_length;
    uint8_t * p = res->payload;
    for ( int i = 0; i < res->payload_length; i++ ) {
        buff[ RES_HEADER_SIZE + i ] = p[ i ];
    }

    return write_fd( client, buff, RES_HEADER_SIZE + res->payload_length );
}

int update( int client, Update * upd ) {
    uint8_t buff[RES_HEADER_SIZE + upd->payload_length];
    buff[ UpdIndexes.STATUS ] = upd->status;
    buff[ UpdIndexes.CONTEXT ] = upd->context;
    buff[ UpdIndexes.PAYLOAD_LENGTH ] = upd->payload_length;
    uint8_t * p = upd->payload;
    for ( int i = 0; i < upd->payload_length; i++ ) {
        buff[ UPD_HEADER_SIZE + i ] = p[ i ];
    }
    return write_fd( client, buff, RES_HEADER_SIZE + upd->payload_length );
}

bool set_games_supported( uint8_t * game_ids ) {
    if ( !game_ids ) {
        return false;
    }
    games_available = game_ids;
    return true;
}

int respond_status( int client, uint8_t req_type, uint8_t status ) {
    Response res;
    res.payload_length = 0;
    res.status = status;
    res.type = req_type;
    res.payload = NULL;
    return respond( client, &res );
}

int respond_success( int client, uint8_t req_type ) {
    return respond_status( client, req_type, ST_SUCCESS );
}

int respond_success_payload( int client, uint8_t req_type, uint8_t * payload, size_t payload_size ) {
    Response res;
    res.payload_length = payload_size;
    res.status = ST_SUCCESS;
    res.type = req_type;
    res.payload = payload;
    return respond( client, &res );
}

int respond_invalid_type( int client, uint8_t req_type ) {
    return respond_status( client, req_type, ST_INVALID_TYPE );
}


int respond_invalid_context( int client, uint8_t req_type ) {
    return respond_status( client, req_type, ST_INVALID_CONTEXT );
}


int respond_invalid_payload( int client, uint8_t req_type ) {
    return respond_status( client, req_type, ST_INVALID_PAYLOAD );
}


int respond_server_error( int client, uint8_t req_type ) {
    return respond_status( client, req_type, ST_SERVER_ERROR );
}

int respond_invalid_uid( int client, uint8_t req_type) {
    return respond_status(client, req_type, ST_INVALID_UID);
}

int respond_uid( int client, uint32_t uid ) {
    uint32_t netEndianUID = htonl( uid );
    return respond_success_payload( client, REQ_CONFIRMATION, ( uint8_t * ) &netEndianUID, UID_LEN );
}


int respond_invalid_action( int client, uint8_t req_type ) {
    return respond_status( client, req_type, ST_INVALID_ACTION );
}


int respond_out_of_turn( int client, uint8_t req_type ) {
    return respond_status( client, req_type, ST_OUT_OF_TURN );
}

int update_start_game( int client ) {
    Update upd;
    upd.status = ST_UPDATE;
    upd.context = UPD_START_GAME;
    upd.payload_length = 0;
    upd.payload = NULL;
    return update( client, &upd );
}


int update_move( int client, uint8_t * move, size_t move_size ) {
    Update upd;
    upd.status = ST_UPDATE;
    upd.context = UPD_MOVE_MADE;
    upd.payload_length = move_size;
    upd.payload = move;
    return update( client, &upd );
}


int update_end_game( int client, uint8_t * payload, size_t payload_len ) {
    Update upd;
    upd.status = ST_UPDATE;
    upd.context = UPD_END_OF_GAME;
    upd.payload_length = payload_len;
    upd.payload = payload;
    return update( client, &upd );
}


int update_opp_disconn( int client ) {
    Update upd;
    upd.status = ST_UPDATE;
    upd.context = UPD_OPP_DISCONNECTED;
    upd.payload_length = 0;
    upd.payload = NULL;
    return update( client, &upd );
}

int validate_field( int client, int field, uint8_t req_type, int expected, int (* handler)( int, uint8_t )) {
    if (( field & expected ) != field ) {
        int res_stat = handler( client, req_type );

        if ( res_stat <= 0 ) {
            return res_stat;
        }

        return INVALID_REQ;
    }

    return OK;
}

int validate_type( int client, Request * req, int expected ) {
    return validate_field( client, req->type, req->type, expected, respond_invalid_type );
}

int validate_context( int client, Request * req, int expected ) {
    return validate_field( client, req->context, req->type, expected, respond_invalid_context );
}

int read_game_choice( int client, Request * req ) {
    int stat = read_msg( client, req );

    int type_stat = validate_type( client, req, REQ_CONFIRMATION );
    if ( type_stat != OK ) {
        return type_stat;
    }

    int context_stat = validate_context( client, req, CONF_RULESET );
    if ( context_stat != OK ) {
        return context_stat;
    }

    uint8_t * games = games_available;

    bool game_available = false;
    while ( *games ) {
        if ( *games == req->payload[ PAYLOAD_GAME_INDEX ] ) {
            game_available = true;
        }
    }

    if ( !game_available ) {

        int res_stat = respond_invalid_payload( client, req->type );

        if ( res_stat <= 0 ) {
            return res_stat;
        }

        return INVALID_REQ;
    }

    return stat;
}


int read_move( int client, Request * req, uint8_t * available_moves ) {

    uint32_t expected_uid = req->uid;

    int stat = read_msg( client, req );

    int type_stat = validate_type( client, req, REQ_ACTION | REQ_META );
    if ( type_stat != OK ) {
        return type_stat;
    }

    int context_stat = validate_context( client, req, ACT_MAKE_MOVE | META_QUIT );
    if ( context_stat != OK ) {
        return context_stat;
    }

    if ( req->context == META_QUIT ) {
        return stat;
    }

    uint8_t * moves = available_moves;

    bool move_available = false;
    while ( *moves ) {
        if ( *moves == req->payload[ PAYLOAD_MOVE_INDEX ] ) {
            move_available = true;
        }
    }

    if ( !move_available ) {

        int res_stat = respond_invalid_action( client, req->type );

        if ( res_stat <= 0 ) {
            return res_stat;
        }

        return INVALID_REQ;
    }

    if ( req->uid != expected_uid ) {

        int res_stat = respond_invalid_uid( client, req->type );

        if ( res_stat <= 0 ) {
            return res_stat;
        }

        return INVALID_REQ;
    }

    return stat;
}



