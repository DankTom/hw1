#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/wait.h>

#define PORT 1112
#define BUF_SIZE 8096

char html[] =
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html;charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head><title>Pionxzh</title></head>\r\n"
"<body><center><div style='margin-top: 20\%;font-size: 80px;'>fork web</div><br>\r\n"
"<img src=\"picture.jpg\"></center></body></html>\r\n";

void sig_fork() {
	int state;
	pid_t pid = waitpid(0, &state, WNOHANG);
	return;
}

void sendFile(int fdclient, char* path, char* attr)
{
    int rt, fdimg;
    char buffer[BUF_SIZE+1];
    sprintf(buffer,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", attr);

	if((fdimg=open(path, O_RDONLY)) == -1) {
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
    int fdserver, fdclient, on = 1;
    char buf[2048];

    signal(SIGCHLD, sig_fork);

    fdserver = socket(AF_INET, SOCK_STREAM, 0);

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    setsockopt(fdserver, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));

    if (bind(fdserver, (struct sockaddr * ) &serveraddr, sizeof(serveraddr)) == -1) {
        perror("bind");
        close(fdserver);
        exit(1);
    }

    if (listen(fdserver, 10) == -1) {
        perror("listen");
        close(fdserver);
        exit(1);
    }

    printf("port : %d\n", PORT);
    while (1) {
        fdclient = accept(fdserver, (struct sockaddr * ) &clientaddr, &sinlen);

        if (!fork()) {
            close(fdserver);
            memset(buf, 0, 1024);
            read(fdclient, buf, 1023);

            printf("%s\n", buf);

            if (!strncmp(buf, "GET /picture.jpg", 13)) {
                sendFile(fdclient, "picture.jpg", "image/jpeg");

            } else {
                write(fdclient, html, sizeof(html) - 1);
			}

            close(fdclient);
            exit(0);
        }
    }

    return 0;
}
