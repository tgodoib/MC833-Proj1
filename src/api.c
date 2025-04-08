//
// Created by Tiago Bannwart on 06/04/25.
//

#include "api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    int res;
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
    if ((res = sqlite3_step(stmt)) != SQLITE_ROW) {
        printf("Error writing movies to database : %d\n", res);
        free(genre_id);
        sqlite3_finalize(stmt);
        return 1;
    }
    const int movie_id = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    // Inserting genres and retrieving IDs
    for (int i = 0; i < m->genre_count; i++) {
        snprintf(
            buffer, 1024,
            "INSERT INTO genres (name) VALUES ('%s') ON CONFLICT DO NOTHING;",
            m->genre[i]
        );
        sqlite3_prepare_v2(db, buffer, -1, &stmt, nullptr);
        if ((res = sqlite3_step(stmt)) != SQLITE_DONE) {
            printf("Error writing genres to database : %d\n", res);
            free(genre_id);
            sqlite3_finalize(stmt);
            return 1;
        }
        sqlite3_finalize(stmt);

        snprintf(
            buffer, 1024,
            "SELECT id FROM genres WHERE name = '%s';",
            m->genre[i]
        );
        sqlite3_prepare_v2(db, buffer, -1, &stmt, nullptr);
        if ((res = sqlite3_step(stmt)) != SQLITE_ROW) {
            printf("Error retrieving genre id to database : %d\n", res);
            free(genre_id);
            sqlite3_finalize(stmt);
            return 1;
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
            printf("Error writing movies_genres to database : %d\n", res);
            free(genre_id);
            sqlite3_finalize(stmt);
            return 1;
        }
        sqlite3_finalize(stmt);
    }

    // sqlite3_close(db);

    free(genre_id);
    return 0;
}

int api_addGenre(sqlite3 *db, int movie_id, char **genre, int genre_count) {
    int res;
    char buffer[1024];
    sqlite3_stmt *stmt;
    int *genre_id = malloc(sizeof(int) * genre_count);

    // Inserting genres and retrieving IDs
    for (int i = 0; i < genre_count; i++) {
        snprintf(
            buffer, 1024,
            "INSERT INTO genres (name) VALUES ('%s') ON CONFLICT DO NOTHING;",
            genre[i]
        );
        sqlite3_prepare_v2(db, buffer, -1, &stmt, nullptr);
        if ((res = sqlite3_step(stmt)) != SQLITE_DONE) {
            printf("Error writing genres to database : %d\n", res);
            free(genre_id);
            sqlite3_finalize(stmt);
            return 1;
        }
        sqlite3_finalize(stmt);

        snprintf(
            buffer, 1024,
            "SELECT id FROM genres WHERE name = '%s';",
            genre[i]
        );
        sqlite3_prepare_v2(db, buffer, -1, &stmt, nullptr);
        if ((res = sqlite3_step(stmt)) != SQLITE_ROW) {
            printf("Error retrieving genre id to database : %d\n", res);
            free(genre_id);
            sqlite3_finalize(stmt);
            return 1;
        }
        genre_id[i] = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
    }

    // Inserting movie-genre relations
    for (int i = 0; i < genre_count; i++) {
        snprintf(
            buffer, 1024,
            "INSERT OR IGNORE INTO movies_genres (movie_id, genre_id) "
            "SELECT m.id, g.id FROM movies m, genres g WHERE m.id = %d AND g.id = %d;",
            movie_id, genre_id[i]
        );

        sqlite3_prepare_v2(db, buffer, -1, &stmt, nullptr);
        if ((res = sqlite3_step(stmt)) != SQLITE_DONE) {
            printf("Error writing movies_genres to database : %d\n", res);
            free(genre_id);
            sqlite3_finalize(stmt);
            return 1;
        }
        sqlite3_finalize(stmt);
    }

    free(genre_id);
    return 0;
}

int api_delete(sqlite3 *db, int movie_id) {
    int res;
    char buffer[1024];
    sqlite3_stmt *stmt;

    snprintf(
        buffer, 1024,
        "DELETE FROM movies WHERE id = %d;",
        movie_id
    );
    sqlite3_prepare_v2(db, buffer, -1, &stmt, nullptr);
    if ((res = sqlite3_step(stmt)) != SQLITE_DONE) {
        printf("Error writing movies_genres to database : %d\n", res);
        sqlite3_finalize(stmt);
        return 1;
    }
    sqlite3_finalize(stmt);

    return 0;
}

int api_list(sqlite3 *db, movie **list_out, int *movie_count) {
    int res, count = 0;
    sqlite3_stmt *stmt;
    movie *list = nullptr;

    sqlite3_prepare_v2(db, "SELECT id, title FROM movies;", -1, &stmt, nullptr);

    while ((res = sqlite3_step(stmt)) == SQLITE_ROW) {
        list = realloc(list, (count + 1) * sizeof(movie));

        list[count].id = sqlite3_column_int(stmt, 0);
        list[count].details = malloc(sizeof(movie_details));
        list[count].details->title = strdup((const char *) sqlite3_column_text(stmt, 1));

        count++;
    }

    sqlite3_finalize(stmt);

    if (res != SQLITE_DONE) {
        printf("Error reading movies : %d\n", res);
        for (int i = 0; i < count; i++) {
            free(list[i].details->title);
            free(list[i].details);
        }
        free(list);
        return 1;
    }

    *list_out = list;
    *movie_count = count;

    return 0;
}

int api_listDetails(sqlite3 *db, movie_details **list, int *movie_count) {
    return 0;
}

int api_details(sqlite3 *db, int movie_id, movie_details *details) {
    return 0;
}

int api_listByGenre(sqlite3 *db, char *genre, movie_details **list, int *movie_count) {
    return 0;
}
