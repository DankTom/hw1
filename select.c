#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>

#define PORT 2223
#define BUF_SIZE 8096

char html[] =
"HTTP/1.1 200 OK\r\n"
"ContexT-Type: text/html;charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head><title>Pionxzh</title></head>\r\n"
"<body><center><div style='margin-top: 20\%;font-size: 80px;'>select web</div><br>\r\n"
"<img src=\"picture.jpg\"></center></body></html>\r\n";

void sendFile(int fdclient, char* route, char* att)
{
    int rt, fdimg;
    char buffer[BUF_SIZE+1];
    sprintf(buffer,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", att);

	if((fdimg=open(route, O_RDONLY)) == -1) {
        perror("No file");
        exit(1);
    }

    write(fdclient, buffer, strlen(buffer));
    while ((rt=read(fdimg, buffer, BUF_SIZE)) > 0) {
        write(fdclient, buffer, rt);
    }
    close(fdimg);
}

int main(int argc, char * argv[]) {
    struct sockaddr_in serveraddr, clientaddr;
    socklen_t sinlen = sizeof(clientaddr);
    int fdserver, fdsocket;

    fd_set activefdset, readfdset;
    char buf[2048];
    int i = 0, on = 1;

    if ((fdserver = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    setsockopt(fdserver, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));

    bind(fdserver, (struct sockaddr * ) &serveraddr, sizeof(serveraddr));

    listen(fdserver, 10);

    FD_ZERO(&activefdset);
    FD_SET(fdserver, &activefdset);


    printf("port : %d\n", PORT);
    while (1) {
        readfdset = activefdset;
        if (select(FD_SETSIZE, &readfdset, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        for (i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &readfdset)) {
                if (i == fdserver) { 
                    sinlen = sizeof(clientaddr);
                    fdsocket = accept(fdserver, (struct sockaddr * ) &clientaddr, &sinlen);

                    FD_SET(fdsocket, &activefdset);
                } else {
                    memset(buf, 0, 1024);
                    read(i, buf, 1023);
                    printf("%s\n", buf);

                    if (!strncmp(buf, "GET /picture.jpg", 13)) {
                        sendFile(i, "picture.jpg", "image/jpeg");
                    } else {
                        write(i, html, sizeof(html) - 1);
                    }

                    close(i);
                    FD_CLR(i, &activefdset);
                }
            }
        }
    }

	return 0;
}
