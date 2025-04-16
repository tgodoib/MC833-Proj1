# Client

> docker build -t mc833_client .

> docker run -it --rm mc833_client /bin/bash

> ./Projeto1_Client "CMD"

> Command List:
> - NEW <TITLE>;<GENRE,GENRE,...>;<DIRECTOR>;<YEAR>;
> - ADD_GENRE <ID>;<GENRE,GENRE,...>;
> - DELETE <ID>;
> - LIST
> - LIST_DETAILS
> - DETAILS <ID>;
> - LIST_BY_GENRE <GENRE>;

> Example of usage:
> - ./Projeto1_Client NEW "Up;Animacao,Familia,Comedia;Pete Docter;2009;"