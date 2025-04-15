//
// Created by Tiago Bannwart on 06/04/25.
//

#ifndef SERVER_H
#define SERVER_H
#include <sqlite3.h>

char* readInput(int client_fd);

void parseInput(sqlite3 *db, int client_fd, const char *input);

sqlite3* db_connect();

void createServer();

#endif //SERVER_H
