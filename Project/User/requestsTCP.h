#ifndef REQUESTSTCP_DOT_H
#define REQUESTSTCP_DOT_H

void connectTCP(int* fd, struct addrinfo* res);
void sendMessageTCP(int fd, char* message, int messageLen);
char* receiveMessageTCP(int fd);
void processRequestTCP(char* input, char* (*parser)(char*), void (*logger)(char*), void(*helper)(char*));

#endif