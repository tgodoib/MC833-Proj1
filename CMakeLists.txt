cmake_minimum_required(VERSION 3.30)
project(Projeto1 C)

set(CMAKE_C_STANDARD 23)


include_directories(./include)

add_executable(Projeto1 src/main.c src/sqlite3.c
        src/api.c
        include/api.h
        src/server.c
        include/server.h)