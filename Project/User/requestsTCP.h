#ifndef REQUESTSTCP_DOT_H
#define REQUESTSTCP_DOT_H

void processUlist(userData *user, serverData *server, char* input);
void processPost(userData* user, serverData* server, char* input);
void processRetrieve(userData* user, serverData* server, char* input);

#endif