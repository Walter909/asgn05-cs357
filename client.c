#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <poll.h>
#include "talk.h"

#define MAXLINE 90
#define KEYBOARD 0
#define SOCKET 1
#define TOTAL_FD 2

void client_chat(int sockfd, int port, char *hostname, struct sockaddr_in sa) {
    int bytes_read;
    int recv_len;
    char buff[MAXLINE + 1];
    struct pollfd fds[TOTAL_FD];

    fds[KEYBOARD].events = POLLIN;
    fds[KEYBOARD].fd = STDIN_FILENO;
    fds[KEYBOARD].revents = 0;
    fds[SOCKET].events = POLLIN;
    fds[SOCKET].fd = sockfd;
    fds[SOCKET].revents = 0;

    printf("Waiting for response from %s.\n", hostname);

    /*try to make a connection*/
    if(connect(sockfd,(struct sockaddr *) &sa, sizeof(sa)) == -1){
        perror("connect");
        exit(EXIT_FAILURE);
    }
    
    /*Only start window if server sends okay*/
    if(recv(sockfd,buff,2,0) == -1) {
        perror("recv");
        exit(EXIT_FAILURE);
    }

    if(!(strncmp(buff,"ok",2) == 0)){
        printf("%s declined connection\n.", hostname);
        return;
    }

    /*use ncurse functions*/
    start_windowing();

    /*poll socket and keyboard input*/
    while(!has_hit_eof()){
        if(poll(fds,TOTAL_FD,-1) == -1){
            perror("poll");
            exit(EXIT_FAILURE);
        }
        
        if(fds[KEYBOARD].revents & POLLIN){
            update_input_buffer();

            if(has_whole_line()){
            /*Read is blocking  so only read when internal buffer is full*/
                if((bytes_read = read_from_input(buff,MAXLINE)) == -1){
                    perror("read");
                    exit(EXIT_FAILURE);
                }

                if(send(sockfd,buff, bytes_read,0) ==-1){
                    perror("send");
                    exit(EXIT_FAILURE);
                }
            }
        }
        if(fds[SOCKET].revents & POLLIN){
            if((recv_len = recv(sockfd,buff,MAXLINE,0)) == -1){
                perror("receive");
                exit(EXIT_FAILURE);
            }

            write_to_output(buff,recv_len);
        }

   
    }
    stop_windowing();
}
