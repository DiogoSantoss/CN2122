#ifndef LOG_DOT_H
#define LOG_DOT_H

#include "colors.h"

void logREG(char* message);
void logUNR(char* message);
void logLOG(char* message);
void logOUT(char* message);
void logGLS(char* message);
void logGSR(char* message);
void logGUR(char* message);
//void logGLM(char* message);
void logPST(int success, char* groupNumber, char* groupName, int messageNumber);
void logRTV(int success, int amountMessages, char** messages);
void logError(char* message);

#endif