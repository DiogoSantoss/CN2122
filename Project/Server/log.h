#ifndef LOG_DOT_H
#define LOG_DOT_H

#include "colors.h"

void logError(int verbose, char* message);
void logTCP(int verbose, char* IP, int port);
void logUDP(int verbose, char* IP, int port);
void logREG(int verbose, char* UID);
void logUNR(int verbose, char* UID);
void logLOG(int verbose, char* UID);
void logOUT(int verbose, char* UID);
void logGLS(int verbose);
void logGSR(int verbose, char* UID, char* GID);
void logGUR(int verbose, char* UID, char* GID);
void logGLM(int verbose, char* UID);
void logULS(int verbose, char* GID);
void logPST(int verbose, char* UID, char* GID);
void logRTV(int verbose, char* UID, char* GID, int numberOfMessages);

#endif