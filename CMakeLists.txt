cmake_minimum_required(VERSION 3.28)
project(kursac C)

set(CMAKE_C_STANDARD 11)

add_executable(server server.c
        structures.h)
add_executable(client client.c
        structures.h)
