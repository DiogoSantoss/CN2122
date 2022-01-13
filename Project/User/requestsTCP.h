#ifndef REQUESTSTCP_DOT_H
#define REQUESTSTCP_DOT_H

char* parseUlist(userData* user, char* input);
void processUlist(userData *user, serverData *server, char* input, char* (*parser)(userData*,char*), void (*logger)(char*), void (*helper)(userData*,char*));
char* parsePost(userData* user, char* input);
void processPost(userData* user, serverData* server, char* input);
void processRetrieve(userData* user, serverData* server, char* input);

#endif