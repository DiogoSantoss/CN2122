#ifndef REQUESTSTCP_DOT_H
#define REQUESTSTCP_DOT_H

void TCPconnect(int* fd, struct addrinfo* res);
void TCPsendMessage(int fd, char* message, int messageLen);
char* TCPreceiveMessage(int fd);
void processRequestTCP(char* input, char* (*parser)(char*), void (*logger)(char*), void(*helper)(char*));

#endif