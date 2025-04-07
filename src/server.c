//
// Created by Tiago Bannwart on 06/04/25.
//

#include "../include/server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

char* readInput(int client_fd) {
    constexpr int READ_BUFFER_SIZE = 1024;

    char buffer[READ_BUFFER_SIZE];
    size_t total_size = 0;
    size_t capacity = READ_BUFFER_SIZE;

    char* data = malloc(READ_BUFFER_SIZE);

    ssize_t bytes_read;
    while ((bytes_read = read(client_fd, buffer, READ_BUFFER_SIZE)) > 0) {
        if (total_size + bytes_read >= capacity) {
            capacity *= 2;
            char* new_data = realloc(data, capacity);
            data = new_data;
        }
        memcpy(data + total_size, buffer, bytes_read);
        total_size += bytes_read;
    }

    if (total_size >= capacity) {
        data = realloc(data, capacity + 1);
    }
    data[total_size] = '\0';

    return data;
}

void parseInput(char* input) {

}

void createServer() {
    const int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(8888)
    };

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("bind failed");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
        printf("listen failed");
        close(server_fd);
        exit(1);
    }

    printf("Server listening on port 99833...\n");

    while (1) {
        const int client_fd = accept(server_fd, nullptr, nullptr);
        if (fork() == 0) {
            close(server_fd);

            char* req = readInput(client_fd);
            parseInput(req);

            exit(0);
        }
        close(client_fd);
    }
}