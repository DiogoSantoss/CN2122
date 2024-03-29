#ifndef REQUESTSUDP_DOT_H
#define REQUESTSUDP_DOT_H

char* parseRegister(userData* user, char* input);
char* parseUnregister(userData* user, char* input);
void helperUnregister(userData* user, char* input, char* response);
char* parseLogin(userData* user, char* input);
void helperLogin(userData* user, char* input, char* response);
char* parseLogout(userData* user, char* input);
void helperLogout(userData* user, char* input, char* response);
void processShowUID(userData* user, char* input);
char* parseSubscribe(userData* user, char* input);
char* parseUnsubscribe(userData* user, char* input);
char* parseGroups(userData* user, char* input);
char* parseMyGroups(userData* user, char* input);
void processSelect(userData* user, char* input);
void processShowGID(userData* user, char* input);

void processRequestUDP(userData* user, serverData* server, char* input, char* (*parser)(userData*,char*), void (*logger)(char*), void (*helper)(userData*,char*,char*));

#endif