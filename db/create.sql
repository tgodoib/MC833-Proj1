create table genres
(
    id   integer not null
        constraint genres_pk primary key autoincrement,
    name TEXT    not null
);

create table movies
(
    id       integer not null
        constraint movies_pk primary key autoincrement,
    title    TEXT    not null,
    director TEXT    not null,
    year     integer not null
);

create table movies_genres
(
    movie_id integer not null,
    genre_id integer not null,
    constraint movies_genres_fk
        foreign key (movie_id, genre_id) references genres (id, id)
);

