#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

extern int signal_stop;
int signal_stop = 0;

void handler(int signum){
    printf("here\n");
    signal_stop = 1;
}

void setup_signal(){
    struct sigaction sa;    
    /*setting sigaction struct fields*/
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (-1 == sigaction(SIGINT, &sa, NULL)) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

}
