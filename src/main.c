#include "api.h"


int main(void) {
    sqlite3 *db = db_connect();

    movie_details details = {
        .title = "oii",
        .genre = nullptr,
        .genre_count = 0,
        .director = "eu",
        .year = 2025
    };

    api_new(db, &details);

    // createServer();

    return 0;
}
