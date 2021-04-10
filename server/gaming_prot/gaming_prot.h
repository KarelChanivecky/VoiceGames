#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef GAME_PROTOCOL_LIBRARY_H
#define GAME_PROTOCOL_LIBRARY_H

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

#ifndef OK
#define OK true
#endif

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

bool set_games_supported( uint8_t * game_ids );

int respond_success( int client, uint8_t req_type );

int respond_success_payload( int client, uint8_t req_type, uint8_t * payload, size_t payload_size );

int respond_invalid_type( int client, uint8_t req_type );

int respond_invalid_context( int client, uint8_t req_type );

int respond_invalid_payload( int client, uint8_t req_type );

int respond_server_error( int client, uint8_t req_type );

int respond_invalid_uid( int client, uint8_t req_type);

int respond_uid( int client, uint32_t uid );

int respond_invalid_action( int client, uint8_t req_type );

int respond_out_of_turn( int client, uint8_t req_type );

int update_start_game( int client );

int update_move( int client, uint8_t * move, size_t move_size );

int update_end_game( int client, uint8_t * payload, size_t payload_len );

int update_opp_disconn( int client );

int read_game_choice( int client, Request * req );

int read_move( int client, Request * req, uint8_t * available_moves );

#endif //GAME_PROTOCOL_LIBRARY_H
