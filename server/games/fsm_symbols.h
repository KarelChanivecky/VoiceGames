/**
* karel on 2021-04-12.
*/

#ifndef GAME_SERVER_FSM_SYMBOLS_H
#define GAME_SERVER_FSM_SYMBOLS_H


enum endgame_codes {
    QUIT,
    WON,
    LOST,
    TIE,

};


enum internal_errs {
    ERR_ADVERSARY_DISCONNECTED,
    ERR_INTERNAL_SERVER = 128,
    ERR_OUT_OF_TURN,
    ERR_POSITION_TAKEN,
    ERR_INVAL_CHOICE,
    ERR_INVAL_ID,
    ERR_UNEXPECTED_MSG_LENGTH,
    COMM_PING,
    NO_CODE // for internal use only
};

#define PLAYER_1_INDEX 0
#define PLAYER_2_INDEX 1
#define PLAYERS_IN_GAME 2
#define MOVE_INDEX 0
#define QUIT 0
#define NO_CHOICE 0

#endif //GAME_SERVER_FSM_SYMBOLS_H
