#ifndef REQUESTSUDP_DOT_H
#define REQUESTSUDP_DOT_H

void createDirectories();

char* processREG(userData user, serverData server, char* request);
char* processURN(userData user, serverData server, char* request);
char* processLOG(userData user, serverData server, char* request);
char* processOUT(userData user, serverData server, char* request);
char* processGLS(userData user, serverData server, char* request);

#endif