#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <poll.h>
#include <netdb.h>
#include <pwd.h>
#include "talk.h"

#define MAXLINE 90
#define KEYBOARD 0
#define SOCKET 1
#define TOTAL_FD 2
#define DEFAULT_BACKLOG 100

#define ANSWER 3

void server_chat(int sockfd, int port, const struct sockaddr_in sa) {
    char answer[ANSWER + 1];
    char* closemsg = "Connection closed. ^C to terminate.";
    int bytes_read;
    int recv_len;
    char buff[MAXLINE + 1];
    struct pollfd fds[TOTAL_FD];
    int newsock;
    socklen_t peeraddr_len;
    struct sockaddr_in peerinfo;
    struct passwd *pwd;

    /*bind*/
    if (bind(sockfd, (struct sockaddr *) &sa, sizeof(sa)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    /*listen*/
    if (listen(sockfd, DEFAULT_BACKLOG) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    /*accept*/
    peeraddr_len = sizeof(peerinfo);
    if ((newsock = accept(sockfd, (struct sockaddr *) &peerinfo,
                          &peeraddr_len)) == -1) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    /*Get user we are trying to accept connections from on school server*/
    /*TODO: need to test with another user to make sure getuid is working*/
    if ((pwd = getpwuid(getuid())) != NULL){
        printf("Mytalk request from %s Accept (y/n)?\n", pwd->pw_name);
    }

    scanf("%s",answer);

    /*Check answer*/
    if(!(strcmp(answer,"yes") == 0 || strcmp(answer,"y") == 0 ||
    strcmp(answer,"YES") == 0 || strcmp(answer,"Y") == 0)){
        if(send(newsock,"nope",4,0) == -1){
            perror("send");
            exit(EXIT_FAILURE);
        }  
        close(newsock);
        return;
    }
    else
    {
        if(send(newsock,"ok",2,0) == -1){
            perror("send");
            exit(EXIT_FAILURE);
        }
    }

    fds[KEYBOARD].events = POLLIN;
    fds[KEYBOARD].fd = STDIN_FILENO;
    fds[KEYBOARD].revents = 0;
    fds[SOCKET].events = POLLIN;
    fds[SOCKET].fd = newsock;
    fds[SOCKET].revents = 0;

   /*ncurses function*/
    start_windowing();

    /*TODO: verbose mode is upto me maybe I
     * change the format of strings using inet_top*/
   

    /*poll socket and keyboard input*/
    while(!has_hit_eof()){
        if(poll(fds,TOTAL_FD,-1) == -1){
            perror("poll");
            exit(EXIT_FAILURE);
        }
        /*Check if socketfd is hungup by client*/
        if(fds[SOCKET].revents & POLLHUP){
             write_to_output(closemsg,strlen(closemsg));
             pause();
             break;
        }

        if(fds[KEYBOARD].revents & POLLIN){
            update_input_buffer();
            
            if(has_whole_line()){
            /*Read is blocking so only read when internal buffer is full*/
                if((bytes_read = read_from_input(buff,MAXLINE)) == -1){
                    perror("read");
                    exit(EXIT_FAILURE);
                }

                if(send(newsock,buff, bytes_read,0) ==-1){
                    perror("send");
                    exit(EXIT_FAILURE);
                }
            }
        }

        if(fds[SOCKET].revents & POLLIN){
            if((recv_len = recv(newsock,buff, MAXLINE,0)) == -1){
                perror("receive");
                exit(EXIT_FAILURE);
            }

            write_to_output(buff,recv_len);
        }
    }
    
    stop_windowing();
    close(newsock);
}
