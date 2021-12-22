#ifndef REQUESTSTCP_DOT_H
#define REQUESTSTCP_DOT_H

char* parseUlist(userData* user, char* input);
char* parsePost(userData* user, char* input);
void processPost(userData* user, serverData* server, char* input);
void processRetrieve(userData* user, serverData* server, char* input);

int connectTCP(serverData *server, int* fd, struct addrinfo* res);
int sendTCP(int fd, char* message, int messageLen);
char* receiveWholeTCP(int fd);
void receiveNSizeTCP(int fd, char* buffer, int messageSize);
void processRequestTCP(userData *user, serverData *server, char* input, char* (*parser)(userData*,char*), void (*logger)(char*), void (*helper)(userData*,char*));

#endif