#include <arpa/inet.h>
#include <getopt.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include "talk.h"
#include "mytalk.h"

/*TODO: Usage: mytalk [-v] [-a] [-N] [hostname] port*/
int main(int argc, char *argv[]) {

    int opt;
    int sockfd;
    int port_num;
    int num_args_read;
    int num_args_left;
    struct sockaddr_in sa;
    struct hostent *hostent;
    char *hostname = NULL;
    char *end = NULL;
    int verbose = 0;
    int ask = 0;
    int ncurse = 0;
    /*Error handling*/
    if (argc < 2) {
        perror("Usage: mytalk [-v] [-a] [-N] [hostname] port");
        exit(EXIT_FAILURE);
    }

    while ((opt = getopt(argc, argv, ":vaN")) != -1) {

        if(opt == 'v'){
                /*Set Verbosity*/
                verbose = 1;
                printf("v\n");
        }                
        
        if(opt == 'a'){
                /*Server accept connection without asking*/
                ask = 1;
                printf("a\n");
        }
        
        if(opt == 'N'){
                /*Ncurses windows*/
                ncurse = 1;
                printf("n\n");
        }
    }

    num_args_read = optind;
    num_args_left = argc - num_args_read;
    /*Client*/
    if (num_args_left == 2) {
        hostname = argv[optind];

        if ((port_num = strtol(argv[optind + 1], &end, 10)) == 0) {
            printf("strtol: invalid int\n");
            return 1;
        }
    }
        /*Server*/
    else if (num_args_left == 1) {
        if ((port_num = strtol(argv[optind], &end, 10)) == 0) {
            printf("strtol: invalid int\n");
            return 1;
        }
    } else {
        printf("Usage: mytalk [-v] [-a] [-N] [hostname] port\n");
        return 1;
    }

    /* Create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    /* connect it */
    sa.sin_family = AF_INET;

    /*use our port*/
    /*htons becuase range is 0 - 65535*/
    sa.sin_port = htons(port_num);

    /*Finish initializing struct sockaddr fields*/
    if (hostname != NULL) {
        /*Client*/
        /*network order*/
        if ((hostent = gethostbyname(hostname)) == NULL) {
            perror("gethostbyname");
            exit(EXIT_FAILURE);
        }

        /*Make cast to internet address which is a char* but void* behavior*/
        sa.sin_addr.s_addr = *(uint32_t *) hostent->h_addr_list[0];
        client_chat(sockfd,port_num,hostname,sa,verbose,ncurse);
    } else {
        /*Server*/
        /*allow connections from all valid addresses*/
        sa.sin_addr.s_addr = htonl(INADDR_ANY);
        server_chat(sockfd, port_num, sa, verbose,ask,ncurse);
    }

    close(sockfd);

    return 0;
}
