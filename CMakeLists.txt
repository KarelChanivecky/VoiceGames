cmake_minimum_required(VERSION 3.17)
project(tic_tac_toe C)

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
add_executable(tic_tac_toe tic_tac_toe.c utils.c utils.h tic_tac_toe.h)
target_link_libraries(tic_tac_toe dc dcfsm dc_utils)
target_compile_options(tic_tac_toe PRIVATE -Wpedantic -Wall -Wextra)


# BUILD A TESTING ECHO SERVER FOR UDP
add_executable(upd_echo_server voice_chat_prot/voice_echo_server.c)
target_link_libraries(upd_echo_server voice_prot)
target_compile_options(upd_echo_server PRIVATE -Wpedantic -Wall -Wextra)
