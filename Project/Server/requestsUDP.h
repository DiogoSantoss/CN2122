#ifndef REQUESTSUDP_DOT_H
#define REQUESTSUDP_DOT_H

void createDirectories();

void requestErrorUDP(userData user, serverData server);
void processREG(userData user, serverData server, char* request);
void processURN(userData user, serverData server, char* request);
void processLOG(userData user, serverData server, char* request);
void processOUT(userData user, serverData server, char* request);
void processGLS(userData user, serverData server, char* request);
void processGSR(userData user, serverData server, char* request);
void processGUR(userData user, serverData server, char* request);
void processGLM(userData user, serverData server, char* request);

#endif