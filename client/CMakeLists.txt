cmake_minimum_required(VERSION 3.30)
project(Projeto1_Client C)

set(CMAKE_C_STANDARD 23)


add_executable(Projeto1_Client src/main.c
        src/api.c
)