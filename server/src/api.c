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

/**
 * cria um novo filme no banco, incluindo seus gêneros, e faz os relacionamentos entre eles.
 * - insere o filme na tabela movies e recupera o id gerado.
 * - insere os gêneros (evita duplicatas) e recupera os ids.
 * - cria relações na tabela movies_genres.
 * retorna 0 em caso de sucesso, 1 em erro.
 */
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
        goto fail;
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
            goto fail;
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
            goto fail;
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
            goto fail;
        }
        sqlite3_finalize(stmt);
    }

    // sqlite3_close(db);

    free(genre_id);
    return 0;

fail:
    free(genre_id);
    sqlite3_finalize(stmt);
    return 1;
}

/**
 * adiciona novos gêneros a um filme já existente.
 * - insere os gêneros (evita duplicatas) e recupera os ids.
 * - cria relações na tabela movies_genres (evita duplicatas).
 * retorna 0 em caso de sucesso, 1 em erro.
 */
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
            goto fail;
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
            goto fail;
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
            goto fail;
        }
        sqlite3_finalize(stmt);
    }

    free(genre_id);
    return 0;

fail:
    free(genre_id);
    sqlite3_finalize(stmt);
    return 1;
}

/**
 * remove um filme da tabela movies com base no id.
 * as relações em movies_genres são automaticamente apagadas se usar foreign keys ON DELETE CASCADE.
 * retorna 0 em caso de sucesso, 1 em erro.
 */
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

/**
 * lista os filmes com apenas id e título.
 * aloca dinamicamente um array de `movie` e define o ponteiro e o count.
 * retorna 0 em caso de sucesso, 1 em erro.
 */
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

/**
 * lista todos os filmes com detalhes (título, diretor, ano, gêneros).
 * agrega os gêneros de forma manual com múltiplas linhas por filme.
 * aloca dinamicamente um array de `movie_details` e define o ponteiro e o count.
 * retorna 0 em caso de sucesso, 1 em erro.
 */
int api_listDetails(sqlite3 *db, movie_details **list_out, int *movie_count) {
    int res, count = 0, current_movie_id = -1;
    sqlite3_stmt *stmt;
    movie_details *list = nullptr;

    sqlite3_prepare_v2(
        db,
        "SELECT m.id, m.title, m.director, m.year, g.name "
        "FROM movies m "
        "LEFT JOIN movies_genres mg ON m.id = mg.movie_id "
        "LEFT JOIN genres g ON mg.genre_id = g.id "
        "ORDER BY m.id;",
        -1, &stmt, nullptr
    );

    while ((res = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);

        if (id != current_movie_id) {
            current_movie_id = id;
            list = realloc(list, (count + 1) * sizeof(movie_details));

            movie_details *m = &list[count];
            m->title = strdup((const char *) sqlite3_column_text(stmt, 1));
            m->director = strdup((const char *) sqlite3_column_text(stmt, 2));
            m->year = sqlite3_column_int(stmt, 3);
            m->genre = nullptr;
            m->genre_count = 0;

            count++;
        }

        const unsigned char *genre = sqlite3_column_text(stmt, 4);
        if (genre) {
            movie_details *m = &list[count - 1];
            m->genre = realloc(m->genre, (m->genre_count + 1) * sizeof(char *));
            m->genre[m->genre_count] = strdup((const char *) genre);
            m->genre_count++;
        }
    }

    sqlite3_finalize(stmt);

    if (res != SQLITE_DONE) {
        printf("Error reading movies details : %d\n", res);
        for (int i = 0; i < count; i++) {
            free(list[i].title);
            free(list[i].director);
            for (int j = 0; j < list[i].genre_count; j++) {
                free(list[i].genre[j]);
            }
            free(list[i].genre);
        }
        free(list);
        return 1;
    }

    *list_out = list;
    *movie_count = count;

    return 0;
}

/**
 * retorna os detalhes completos de um filme específico, incluindo os gêneros.
 * popula a struct passada por ponteiro.
 * assume que `details` foi alocada, mas não seus campos internos.
 * retorna 0 em caso de sucesso, 1 em erro.
 */
int api_details(sqlite3 *db, int movie_id, movie_details *details) {
    char buffer[1024];
    int res;
    sqlite3_stmt *stmt;

    snprintf(
        buffer, 1024,
        "SELECT m.title, m.director, m.year, g.name "
        "FROM movies m "
        "LEFT JOIN movies_genres mg ON m.id = mg.movie_id "
        "LEFT JOIN genres g ON mg.genre_id = g.id "
        "WHERE m.id = %d;",
        movie_id
    );

    sqlite3_prepare_v2(
        db,
        buffer,
        -1, &stmt, nullptr
    );

    details->title = nullptr;
    details->director = nullptr;
    details->year = 0;
    details->genre = nullptr;
    details->genre_count = 0;

    int first = 1;

    while ((res = sqlite3_step(stmt)) == SQLITE_ROW) {
        if (first) {
            details->title = strdup((const char *) sqlite3_column_text(stmt, 0));
            details->director = strdup((const char *) sqlite3_column_text(stmt, 1));
            details->year = sqlite3_column_int(stmt, 2);
            first = 0;
        }

        const unsigned char *genre = sqlite3_column_text(stmt, 3);
        if (genre) {
            details->genre = realloc(details->genre, (details->genre_count + 1) * sizeof(char *));
            details->genre[details->genre_count] = strdup((const char *) genre);
            details->genre_count++;
        }
    }

    if (res != SQLITE_DONE) {
        printf("error reading movie details: %d\n", res);
        free(details->title);
        free(details->director);
        for (int i = 0; i < details->genre_count; i++) {
            free(details->genre[i]);
        }
        free(details->genre);
        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

/**
 * lista todos os filmes que possuem um certo gênero.
 * inclui todos os gêneros de cada filme, mesmo que não seja o procurado.
 * aloca dinamicamente um array de `movie_details` e define o ponteiro e o count.
 * retorna 0 em caso de sucesso, 1 em erro.
 */
int api_listByGenre(sqlite3 *db, char *genre, movie_details **list_out, int *movie_count) {
    char buffer[1024];
    int res, count = 0, current_movie_id = -1;
    sqlite3_stmt *stmt;
    movie_details *list = nullptr;

    snprintf(
        buffer, 1024,
        "SELECT m.id, m.title, m.director, m.year, g.name "
        "FROM movies m "
        "LEFT JOIN movies_genres mg ON m.id = mg.movie_id "
        "LEFT JOIN genres g ON mg.genre_id = g.id "
        "WHERE EXISTS ("
        "    SELECT 1 FROM movies_genres mg2 "
        "    JOIN genres g2 ON mg2.genre_id = g2.id "
        "    WHERE mg2.movie_id = m.id AND g2.name = '%s'"
        ") "
        "ORDER BY m.id;",
        genre
    );


    sqlite3_prepare_v2(db, buffer, -1, &stmt, nullptr);

    while ((res = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);

        if (id != current_movie_id) {
            current_movie_id = id;
            list = realloc(list, (count + 1) * sizeof(movie_details));

            movie_details *m = &list[count];
            m->title = strdup((const char *) sqlite3_column_text(stmt, 1));
            m->director = strdup((const char *) sqlite3_column_text(stmt, 2));
            m->year = sqlite3_column_int(stmt, 3);
            m->genre = nullptr;
            m->genre_count = 0;

            count++;
        }

        const unsigned char *g = sqlite3_column_text(stmt, 4);
        if (g) {
            movie_details *m = &list[count - 1];
            m->genre = realloc(m->genre, (m->genre_count + 1) * sizeof(char *));
            m->genre[m->genre_count] = strdup((const char *) g);
            m->genre_count++;
        }
    }

    sqlite3_finalize(stmt);

    if (res != SQLITE_DONE) {
        printf("Error reading movies details : %d\n", res);
        for (int i = 0; i < count; i++) {
            free(list[i].title);
            free(list[i].director);
            for (int j = 0; j < list[i].genre_count; j++) {
                free(list[i].genre[j]);
            }
            free(list[i].genre);
        }
        free(list);
        return 1;
    }

    *list_out = list;
    *movie_count = count;

    return 0;
}