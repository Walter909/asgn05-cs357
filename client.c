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

void client_chat(int sockfd, int port, char *hostname, struct sockaddr_in sa, int verbose, int ncurse) {
    int bytes_read;
    int recv_len;
    char buff[MAXLINE + 1];
    char vmsg[MAXLINE + 1];
    struct pollfd fds[TOTAL_FD];
    char* closemsg = "Connection closed. ^C to terminate";

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
    
    /*Only start window if server sends ok*/
    if(recv(sockfd,buff,2,0) == -1) {
        perror("recv");
        exit(EXIT_FAILURE);
    }

    /*Server sends anything than ok decline connection*/
    if(!(strncmp(buff,"ok",2) == 0)){
        printf("%s declined connection\n.", hostname);
        return;
    } 
    
   if(ncurse == 0){ 
        /*use ncurse functions*/
        start_windowing();
        if(verbose == 1){
            set_verbosity(1);    
        }
   }
    /*poll socket and keyboard input*/
    while(!has_hit_eof()){
        if(poll(fds,TOTAL_FD,-1) == -1){
            perror("poll");
            exit(EXIT_FAILURE);
        }
        
        /*check if server hungup handling*/        
        if(fds[SOCKET].revents & POLLHUP){
           write_to_output(closemsg,strlen(closemsg));
           pause();
           break; 
        }
        
        /*Trying to send something*/
        if(fds[KEYBOARD].revents & POLLIN){
            update_input_buffer();

            if(has_whole_line()){
                /*Read is blocking  so only read when internal buffer is full*/
                if((bytes_read = read_from_input(buff,MAXLINE)) == -1){
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                /*In verbose write bytes read*/
                if(verbose == 1){
                    snprintf(vmsg,MAXLINE + 1,"debug: bytes read from stdin %d\n",bytes_read);
                    write_to_output(vmsg,strlen(vmsg));
                }

                if(send(sockfd,buff, bytes_read,0) ==-1){
                    perror("send");
                    exit(EXIT_FAILURE);
                }
            }
        }
        /*Trying to receive something*/
        if(fds[SOCKET].revents & POLLIN){
            if((recv_len = recv(sockfd,buff,MAXLINE,0)) == -1){
                perror("receive");
                exit(EXIT_FAILURE);
            }
             /*If the client shutsdown*/
            if(recv_len == 0){
                write_to_output(closemsg,strlen(closemsg));
                pause();
                break;
            }
            /*In verbose write received from server*/
            if(verbose == 1){
                snprintf(vmsg,MAXLINE + 1,"debug: bytes received from server %d\n",recv_len);
                write_to_output(vmsg,strlen(vmsg));
            }
 
            write_to_output(buff,recv_len);
        }

   
    }
    stop_windowing();
}
