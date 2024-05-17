cmake_minimum_required(VERSION 3.28)
project(kursac C)

set(CMAKE_C_STANDARD 11)

add_executable(server server.c
        structures.h)
target_link_libraries(server m)
