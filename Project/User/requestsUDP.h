#ifndef REQUESTSUDP_DOT_H
#define REQUESTSUDP_DOT_H

char* parseRegister(userData* user, char* input);
char* parseUnregister(userData* user, char* input);
char* parseLogin(userData* user, char* input);
void helperLogin(userData* user, char *response);
char* parseLogout(userData* user, char* input);
void helperLogout(userData* user, char* response);
void processShowUID(userData* user, char* input);
char* parseSubscribe(userData* user, char* input);
char* parseUnsubscribe(userData* user, char* input);
char* parseGroups(userData* user, char* input);
char* parseMyGroups(userData* user, char* input);
void processSelect(userData* user, char* input);
void processShowGID(userData* user, char* input);

int connectUDP(serverData *server, int* fd, struct addrinfo** res);
int sendMessageUDP(int fd, struct addrinfo* res, char* message, int messageLen);
char* receiveMessageUDP(int fd);
void processRequestUDP(userData* user, serverData *server, char* input, char* (*parser)(userData*,char*), void (*logger)(char*), void (*helper)(userData*,char*));

#endif