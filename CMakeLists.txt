cmake_minimum_required(VERSION 3.25)
project(SOLO)
set(CMAKE_C_STANDARD 99)
add_executable(main main.c server.c client.c blackjack.c deck.c)
