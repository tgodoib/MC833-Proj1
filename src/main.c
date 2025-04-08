#include <stdlib.h>

#include "api.h"


int main(void) {
    sqlite3 *db = db_connect();

    char *genres[2];
    genres[0] = "rwer";
    genres[1] = "werwr";

    movie_details details = {
        .title = "werwer",
        .genre = genres,
        .genre_count = 2,
        .director = "werwer",
        .year = 2025
    };

    api_new(db, &details);

    // char *genres[1];
    // genres[0] = "teste";
    // // genres[1] = "hmm";
    //
    // api_addGenre(db, 1, genres, 1);

    // api_delete(db, 1);

    // createServer();

    movie* list = nullptr;
    int movie_count;
    api_list(db, &list, &movie_count);

    for (int i = 0; i < movie_count; i++) {
        free(list[i].details->title);
        free(list[i].details);
    }
    free(list);

    return 0;
}
