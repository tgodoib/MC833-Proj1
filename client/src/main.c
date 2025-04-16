#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define IP "127.0.0.1"
#define PORT 9833

void sendCmd(const char *ip, int port, const char *msg) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Connect failed.");
        close(sock);
        exit(1);
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
    };
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("Connect failed.");
        close(sock);
        exit(1);
    }

    send(sock, msg, strlen(msg), 0);
    shutdown(sock, SHUT_WR);

    char buffer[1024];
    ssize_t bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes < 0) {
        printf("Receive failed.");
        close(sock);
        exit(1);
    }

    buffer[bytes] = '\0';
    printf("%s", buffer);

    close(sock);
}


int main(int argc, char **argv) {
    if (argc > 1) {
        int total_len = 0;
        for (int i = 1; i < argc; i++) total_len += strlen(argv[i]) + 1;
        char *joined = malloc(total_len);
        joined[0] = '\0';

        for (int i = 1; i < argc; i++) {
            strcat(joined, argv[i]);
            if (i < argc - 1) {
                strcat(joined, " ");
            }
        }

        sendCmd("65.21.179.185", 9833, joined);

        free(joined);
    }
    else {
        printf("No command.\n");
        printf("Usage: ./main <command>\n");
        printf("Commands:\n");
        printf("NEW <TITLE>;<GENRE,GENRE,...>;DIRECTOR;YEAR;\n");
        printf("ADD_GENRE <ID>;<GENRE,GENRE,...>;\n");
        printf("DELETE <ID>;\n");
        printf("LIST;\n");
        printf("LIST_DETAILS;\n");
        printf("DETAILS <ID>;\n");
        printf("LIST_BY_GENRE <GENRE>;\n");
        printf("Example: ./main NEW \"Up;Animacao,Familia,Comedia;Pete Docter;2009;\"\n");
    }
    return 0;
}
