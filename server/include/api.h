#ifndef API_H
#define API_H

#include "../include/sqlite3.h"

typedef struct movie_details {
    char *title;
    char **genre;
    int genre_count;
    char *director;
    int year;
} movie_details;

typedef struct movie {
    int id;
    movie_details *details;
} movie;

// NEW Up;Animacao,Familia,Comedia;Pete Docter;2009

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



int api_new(sqlite3 *db, movie_details *m);

int api_addGenre(sqlite3 *db, int movie_id, char **genre, int genre_count);

int api_delete(sqlite3 *db, int movie_id);

int api_list(sqlite3 *db, movie **list, int *movie_count);

int api_listDetails(sqlite3 *db, movie_details **list_out, int *movie_count);

int api_details(sqlite3 *db, int movie_id, movie_details *details);

int api_listByGenre(sqlite3 *db, char *genre, movie_details **list_out, int *movie_count);


#endif //API_H
