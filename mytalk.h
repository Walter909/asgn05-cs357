#ifndef MYTALK_H
#define MYTALK_H

void client_chat(int sockfd, int port, char *hostname, struct sockaddr_in sa, int verbose, int ncurses);

void server_chat(int sockfd, int port, const struct sockaddr_in sa, int verbose, int ask, int ncurses);

extern int signal_stop;

void setup_signal();

#endif
