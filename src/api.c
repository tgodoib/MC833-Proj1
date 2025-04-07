//
// Created by Tiago Bannwart on 06/04/25.
//

#include "api.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * NEW <TITLE>;<GENRE,GENRE,...>;DIRECTOR;YEAR;
 * ADD_GENRE <ID>;<GENRE,GENRE,...>;
 * DELETE <ID>
 *
 * LIST
 * LIST_DETAILS
 * DETAILS <ID>
 * LIST_BY_GENRE <GENRE>
 **/

sqlite3 *db_connect() {
    sqlite3 *db;
    const int res = sqlite3_open("../db.sqlite", &db);

    if (res != SQLITE_OK) {
        printf("Error opening database : %d\n", res);
        exit(1);
    }

    // sqlite3_close(db);
    return db;
}

int api_new(sqlite3 *db, movie_details *m) {
    int res, ret = 0;
    char buffer[1024];
    sqlite3_stmt *stmt;
    int *genre_id = malloc(sizeof(int) * m->genre_count);

    // Inserting movie and retrieving ID
    snprintf(
        buffer, 1024,
        "INSERT INTO movies (title, director, year) VALUES ('%s', '%s', %d) RETURNING id;",
        m->title, m->director, m->year
    );
    sqlite3_prepare_v2(db, buffer, -1, &stmt, nullptr);
    if ((res = sqlite3_step(stmt)) != SQLITE_DONE) {
        printf("Error writing to database : %d\n", res);
        ret = 1;
    }
    const int movie_id = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    // Inserting genres and retrieving IDs
    for (int i = 0; i < m->genre_count; i++) {
        snprintf(
            buffer, 1024,
            "INSERT INTO genres (name) VALUES ('%s') RETURNING id;",
            m->genre[i]
        );
        sqlite3_prepare_v2(db, buffer, -1, &stmt, nullptr);
        if ((res = sqlite3_step(stmt)) != SQLITE_DONE) {
            printf("Error writing to database : %d\n", res);
            ret = 1;
        }
        genre_id[i] = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
    }

    // Inserting movie-genre relations
    for (int i = 0; i < m->genre_count; i++) {
        snprintf(
            buffer, 1024,
            "INSERT INTO movies_genres (movie_id, genre_id) VALUES (%d, %d);",
            movie_id, genre_id[i]
        );
        sqlite3_prepare_v2(db, buffer, -1, &stmt, nullptr);
        if ((res = sqlite3_step(stmt)) != SQLITE_DONE) {
            printf("Error writing to database : %d\n", res);
            ret = 1;
        }
        sqlite3_finalize(stmt);
    }

    free(genre_id);
    return ret;
}

int api_addGenre(sqlite3 *db, int id, char **genre) {
    return 0;
}

int api_delete(sqlite3 *db, int id) {
    return 0;
}

int api_list(sqlite3 *db, movie *list) {
    return 0;
}

int api_listDetails(sqlite3 *db, movie_details *list) {
    return 0;
}

int api_details(sqlite3 *db, int id, movie_details *details) {
    return 0;
}

int api_listByGenre(sqlite3 *db, char *genre, movie_details *list) {
    return 0;
}
