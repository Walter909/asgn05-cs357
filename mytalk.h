#ifndef MYTALK_H
#define MYTALK_H

void client_chat(int sockfd, int port, char *hostname, struct sockaddr_in sa);

void server_chat(int sockfd, int port, const struct sockaddr_in sa);

#endif
