/*
 * Author: Tiantian Gao<gtt116@gmail.com>
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

#define MAXLINE 4096
#define PORT  8888


void handle_error(char* msg){
    printf("%s: %s (errno: %d)\n", msg, strerror(errno), errno); 
    exit(errno);
}


/*
 * A server do follow steps:
 *
 * 1. Create socket, create sockaddr_in.
 * 2. Bind sockaddr with socket.
 * 3. Listen on socket.
 * 4. Accept new connection from socket.
 */


int main(int argc, char** argv){
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    char buff[MAXLINE];
    int n;

    printf("Init listenfd %d\n", listenfd);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    if (listenfd == -1|| listenfd == 0){
        handle_error("Create listen socket failed.");
    }else{
        printf("Create socket success %d\n", listenfd);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
        handle_error("Bind failed.");
    }

    if (listen(listenfd, 10) == -1){
        handle_error("Listen failed.");
    }

    /* 
     * After here, client can establish TCP connection with me, 
     * even no accept(). All ESTB connection will sit in backlog queue
     * wait for accept()
     */
    
    printf("Server is running, wait for client.\n");

    while(1){
        /*
         * Serve A client once a time.
         */
        connfd = accept(listenfd, (struct sockaddr*) NULL, NULL);
        if (connfd == -1){
            handle_error("Accept error"); 
        }else{
            printf("A new connection\n");
        }

        char hi_msg[] = "Hi\0";
        send(connfd, hi_msg, sizeof(hi_msg), 0);

        n = recv(connfd, buff, MAXLINE, 0);
        printf("Recv %d bytes", n);
        buff[n] = '\0';

        printf("Recv: %s", buff);
        
        char bye_msg[] = "Bye\n";
        send(connfd, bye_msg, sizeof(bye_msg), 0);

        close(connfd);
    }

    exit(0);
}
