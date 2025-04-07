//
// Created by Tiago Bannwart on 06/04/25.
//

#ifndef SERVER_H
#define SERVER_H

char* readInput(int client_fd);

void parseInput(char *input);

void createServer();

#endif //SERVER_H
