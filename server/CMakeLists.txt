cmake_minimum_required(VERSION 3.24)
project(Projeto1_Server C)

set(CMAKE_C_STANDARD 23)


include_directories(include)

add_executable(Projeto1_Server src/main.c
        src/sqlite3.c
        src/api.c
        src/server.c
)