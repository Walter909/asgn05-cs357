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
#include <errno.h>
#include "mytalk.h"
#include "talk.h"

#define MAXLINE 90
#define KEYBOARD 0
#define SOCKET 1
#define TOTAL_FD 2
#define DEFAULT_BACKLOG 100

#define ANSWER 3

void server_chat(int sockfd, int port, const struct sockaddr_in sa, int verbose, int ask, int ncurse){
    char answer[ANSWER + 1];
    char* closemsg = "Connection closed. ^C to terminate.";
    char vmsg[MAXLINE + 1];
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
    if ((newsock = accept(sockfd, (struct sockaddr *) &peerinfo, &peeraddr_len)) == -1) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    
    if(ask == 0){
        /*Get user we are trying to accept connections from on school server*/
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
     }
    /*Response for client to know to connect*/ 
    if(send(newsock,"ok",2,0) == -1){
          perror("send");
          exit(EXIT_FAILURE);
    }
    

    fds[KEYBOARD].events = POLLIN;
    fds[KEYBOARD].fd = STDIN_FILENO;
    fds[KEYBOARD].revents = 0;
    fds[SOCKET].events = POLLIN;
    fds[SOCKET].fd = newsock;
    fds[SOCKET].revents = 0;
    
    if(ncurse == 0){
        /*ncurses function*/
        start_windowing();
        /*Then also set verbose*/
        if(verbose == 1){
            set_verbosity(1);
        }
    }
    /*Register signal*/
    setup_signal(); 
    
    /*poll socket and keyboard input*/
    while(!has_hit_eof() && signal_stop != 1){
      
        if(poll(fds,TOTAL_FD,-1) == -1 && errno != EINTR){
            perror("poll");
            exit(EXIT_FAILURE);
        }
        /*Check if socketfd is hungup by client*/
        if(fds[SOCKET].revents & POLLHUP){
             if(write_to_output(closemsg,strlen(closemsg)) == ERR && errno != EINTR){
               perror("write_to_output");
               exit(EXIT_FAILURE);
             }
             pause();
             break;
        }
        
        /*Trying to send something*/
        if(fds[KEYBOARD].revents & POLLIN){
            update_input_buffer();
            
            if(has_whole_line()){
            /*Read is blocking so only read when internal buffer is full*/
                if((bytes_read = read_from_input(buff,MAXLINE)) == ERR  && errno != EINTR){
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                
                /*In verbose write bytes_read*/
                if(verbose == 1){
                    snprintf(vmsg,MAXLINE + 1,"debug: bytes read from stdin %d\n",bytes_read);
                    if(write_to_output(vmsg,strlen(vmsg)) == ERR && errno != EINTR){
                        perror("write_to_output");
                        exit(EXIT_FAILURE);
                    }
                }

                if(send(newsock,buff, bytes_read,0) == -1 && errno != EINTR){
                    perror("send");
                    exit(EXIT_FAILURE);
                }
            }
        }

        /*Trying to receive something*/
        if(fds[SOCKET].revents & POLLIN){
            if((recv_len = recv(newsock,buff, MAXLINE,0)) == -1 && errno != EINTR){
                perror("receive");
                exit(EXIT_FAILURE);
            }
            /*If the client shutsdown*/
            if(recv_len == 0){
                if(write_to_output(closemsg,strlen(closemsg)) == ERR && errno != EINTR){
                    perror("write_to_output");
                    exit(EXIT_FAILURE);
                }
                pause();
                break; 
            }
            /*In verbose write received from client*/
            if(verbose == 1){
                snprintf(vmsg,MAXLINE + 1,"debug: bytes received from client %d\n",recv_len);
               if(write_to_output(vmsg,strlen(vmsg)) == ERR && errno != EINTR){
                    perror("write_to_output");
                    exit(EXIT_FAILURE);
                }
            }

            if(write_to_output(buff,recv_len) == ERR  && errno != EINTR){
                perror("write_to_output");
                exit(EXIT_FAILURE);
            }
        }
    }
    
    stop_windowing();
    close(newsock);
}
