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

#include "api.h"

char *readInput(int client_fd) {
    constexpr int READ_BUFFER_SIZE = 1024;

    char buffer[READ_BUFFER_SIZE];
    size_t total_size = 0;
    size_t capacity = READ_BUFFER_SIZE;

    char *data = malloc(READ_BUFFER_SIZE);

    ssize_t bytes_read;
    while ((bytes_read = read(client_fd, buffer, READ_BUFFER_SIZE)) > 0) {
        if (total_size + bytes_read >= capacity) {
            capacity *= 2;
            char *new_data = realloc(data, capacity);
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

/**
 * NEW <TITLE>;<GENRE,GENRE,...>;DIRECTOR;YEAR;
 * ADD_GENRE <ID>;<GENRE,GENRE,...>;
 * DELETE <ID>;
 *
 * LIST
 * LIST_DETAILS
 * DETAILS <ID>;
 * LIST_BY_GENRE <GENRE>;
 **/

void parseInput(sqlite3 *db, int client_fd, const char *input) {
    char *command = strdup(input);
    char *action = strtok(command, " ");
    action[strcspn(action, "\r\n")] = '\0';

    printf("%s\n", action);
    printf("%d\n", strcmp(action, "LIST"));

    if (strcmp(action, "NEW") == 0) {
        movie_details m;
        m.genre = nullptr;
        m.genre_count = 0;

        char *title = strtok(nullptr, ";");
        char *genres = strtok(nullptr, ";");
        char *director = strtok(nullptr, ";");
        char *year_str = strtok(nullptr, ";");

        m.title = strdup(title);
        m.director = strdup(director);
        m.year = atoi(year_str);

        char *genre_token = strtok(genres, ",");
        while (genre_token != nullptr) {
            m.genre = realloc(m.genre, (m.genre_count + 1) * sizeof(char *));
            m.genre[m.genre_count] = strdup(genre_token);
            m.genre_count++;
            genre_token = strtok(nullptr, ",");
        }

        if (api_new(db, &m) != 0) {
            const char *msg = "Algo deu errado! Confira o comando.\n";
            write(client_fd, msg, strlen(msg));
        }

        free(m.title);
        free(m.director);
        for (int i = 0; i < m.genre_count; i++) {
            free(m.genre[i]);
        }
        free(m.genre);
    } else if (strcmp(action, "ADD_GENRE") == 0) {
        char **genre = nullptr;
        int genre_count = 0;

        char *movie_id = strtok(nullptr, ";");
        char *genres = strtok(nullptr, ";");

        char *genre_token = strtok(genres, ",");
        while (genre_token != nullptr) {
            genre = realloc(genre, (genre_count + 1) * sizeof(char *));
            genre[genre_count] = strdup(genre_token);
            genre_count++;
            genre_token = strtok(nullptr, ",");
        }

        if (api_addGenre(db, atoi(movie_id), genre, genre_count) != 0) {
            const char *msg = "Algo deu errado! Confira o comando.\n";
            write(client_fd, msg, strlen(msg));
        }

        free(genre);
    } else if (strcmp(action, "DELETE") == 0) {
        char *movie_id = strtok(nullptr, ";");
        if (api_delete(db, atoi(movie_id)) != 0) {
            const char *msg = "Algo deu errado! Confira o comando.\n";
            write(client_fd, msg, strlen(msg));
        }
    } else if (strcmp(action, "LIST") == 0) {
        movie *list = nullptr;
        int movie_count = 0;
        if (api_list(db, &list, &movie_count) != 0) {
            const char *msg = "Algo deu errado! Confira o comando.\n";
            write(client_fd, msg, strlen(msg));
        }

        char out[1024];
        snprintf(
            out, sizeof(out),
            "%-2s | %-30s\n"
            "--------------------------------\n",
            "ID", "Title"
        );
        write(client_fd, out, strlen(out));

        for (int i = 0; i < movie_count; i++) {
            movie m = list[i];

            snprintf(
                out, sizeof(out),
                "%-2u | %-30s\n",
                m.id,
                m.details->title
            );

            write(client_fd, out, strlen(out));
        }
    } else if (strcmp(action, "LIST_DETAILS") == 0) {
        movie_details *list = nullptr;
        int movie_count = 0;
        if (api_listDetails(db, &list, &movie_count) != 0) {
            const char *msg = "Algo deu errado! Confira o comando.\n";
            write(client_fd, msg, strlen(msg));
        }

        char out[1024];
        snprintf(
            out, sizeof(out),
            "%-30s | %-20s | %-20s | %-4s\n"
            "-----------------------------------------------------------------------------------------\n",
            "Title", "Genres", "Director", "Year"
        );
        write(client_fd, out, strlen(out));

        for (int i = 0; i < movie_count; i++) {
            movie_details m = list[i];

            char genres[256] = {0};
            for (int j = 0; j < m.genre_count; j++) {
                strcat(genres, m.genre[j]);
                if (j < m.genre_count - 1)
                    strcat(genres, ", ");
            }

            snprintf(
                out, sizeof(out),
                "%-30s | %-20s | %-20s | %-4d\n",
                m.title,
                genres,
                m.director,
                m.year
            );

            write(client_fd, out, strlen(out));
        }
    } else if (strcmp(action, "DETAILS") == 0) {
        movie_details m;

        char *movie_id = strtok(nullptr, ";");

        if (api_details(db, atoi(movie_id), &m) != 0) {
            const char *msg = "Algo deu errado! Confira o comando.\n";
            write(client_fd, msg, strlen(msg));
        }

        char out[1024];

        char genres[256] = {0};
        for (int j = 0; j < m.genre_count; j++) {
            strcat(genres, m.genre[j]);
            if (j < m.genre_count - 1)
                strcat(genres, ", ");
        }

        snprintf(
            out, sizeof(out),
            "%-30s | %-20s | %-20s | %-4s\n"
            "-----------------------------------------------------------------------------------------\n"
            "%-30s | %-20s | %-20s | %-4d\n",
            "Title", "Genres", "Director", "Year",
            m.title,
            genres,
            m.director,
            m.year
        );

        write(client_fd, out, strlen(out));
    } else if (strcmp(action, "LIST_BY_GENRE") == 0) {
        movie_details *list = nullptr;
        int movie_count = 0;

        char *genre = strtok(nullptr, ";");

        if (api_listByGenre(db, genre, &list, &movie_count) != 0) {
            const char *msg = "Algo deu errado! Confira o comando.\n";
            write(client_fd, msg, strlen(msg));
        }

        char out[1024];
        snprintf(
            out, sizeof(out),
            "%-30s | %-20s | %-20s | %-4s\n"
            "-----------------------------------------------------------------------------------------\n",
            "Title", "Genres", "Director", "Year"
        );
        write(client_fd, out, strlen(out));

        for (int i = 0; i < movie_count; i++) {
            movie_details m = list[i];

            char genres[256] = {0};
            for (int j = 0; j < m.genre_count; j++) {
                strcat(genres, m.genre[j]);
                if (j < m.genre_count - 1)
                    strcat(genres, ", ");
            }

            snprintf(
                out, sizeof(out),
                "%-30s | %-20s | %-20s | %-4d\n",
                m.title,
                genres,
                m.director,
                m.year
            );

            write(client_fd, out, strlen(out));
        }
    } else {
        const char *msg = "Comando inválido.\n";
        write(client_fd, msg, strlen(msg));
    }

    free(command);
}

sqlite3 *db_connect() {
    sqlite3 *db;
    const int res = sqlite3_open("../db.sqlite", &db);

    if (res != SQLITE_OK) {
        printf("Error opening database : %d\n", res);
        exit(1);
    }

    return db;
}


void createServer() {
    sqlite3 *db = db_connect();
    const int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(9833)
    };

    if (bind(server_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        printf("bind failed");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
        printf("listen failed");
        close(server_fd);
        exit(1);
    }

    printf("Server listening on port 9833...\n");

    while (1) {
        const int client_fd = accept(server_fd, nullptr, nullptr);
        if (fork() == 0) {
            close(server_fd);

            char *req = readInput(client_fd);
            parseInput(db, client_fd, req);
            free(req);

            exit(0);
        }
        close(client_fd);
    }

    // sqlite3_close(db);
}
