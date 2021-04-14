cmake_minimum_required(VERSION 3.17)
project(game_server C)

set(CMAKE_C_STANDARD 11)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

# BUILD THE UDP VOICE PROTOCOL LIB
add_library(voice_prot voice_chat_prot/voice_prot.c)
target_compile_options(voice_prot PRIVATE -Wpedantic -Wall -Wextra)

# BUILD THE BASE GAMING PROTOCOL LIB
add_library(gaming_prot gaming_prot/gaming_prot.c)
target_compile_options(gaming_prot PRIVATE -Wpedantic -Wall -Wextra)


# BUILD THE GAME SERVER
add_executable(tic_tac_toe
        lib/handshaking.c lib/handshaking.h
        lib/client_gateway.c lib/client_gateway.h
        lib/new_client_plexer.c lib/new_client_plexer.h
        lib_voice_server/voice_server.c lib_voice_server/voice_server.h
        game_server.c game_server.h
        games/ttt/ttt_utils.c games/ttt/ttt_utils.h
        games/ttt/ttt.c games/ttt/ttt.h
        games/rps/rps.c games/rps/rps.h
        lib/game_driver.c lib/game_driver.h
        lib/game_collection.c lib/game_collection.h
        lib/threaded_list.c lib/game_environment.h lib/common_symbols.h
        utils.c utils.h)

target_link_libraries(tic_tac_toe dc dcfsm dc_utils pthread gaming_prot voice_prot)
target_compile_options(tic_tac_toe PRIVATE -Wpedantic -Wall -Wextra)


# BUILD A TESTING ECHO SERVER FOR UDP
add_executable(upd_echo_server voice_chat_prot/test_voice_echo_server.c)
target_link_libraries(upd_echo_server voice_prot)
target_compile_options(upd_echo_server PRIVATE -Wpedantic -Wall -Wextra)
