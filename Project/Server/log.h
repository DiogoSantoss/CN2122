#ifndef LOG_DOT_H
#define LOG_DOT_H

#include "colors.h"

void logError(char* message);
void logTCP(char* IP, int port);
void logUDP(char* IP, int port);
void logREG(char* UID);
void logUNR(char* UID);
void logLOG(char* UID);
void logOUT(char* UID);
void logGLS();
void logGSR(char* UID, char* GID);
void logGUR(char* UID, char* GID);
void logGLM(char* UID);
void logULS(char* GID);
void logPST(char* UID, char* GID);
void logRTV(char* UID, char* GID);

#endif