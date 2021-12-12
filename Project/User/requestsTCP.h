#ifndef REQUESTSTCP_DOT_H
#define REQUESTSTCP_DOT_H

char* parseUlist(userData* user, char* input);

void connectTCP(serverData *server, int* fd, struct addrinfo* res);
void sendMessageTCP(int fd, char* message, int messageLen);
char* receiveMessageTCP(int fd);
void processRequestTCP(userData *user, serverData *server, char* input, char* (*parser)(userData*,char*), void (*logger)(char*), void (*helper)(userData*,char*));

#endif