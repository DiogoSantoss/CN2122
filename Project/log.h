#ifndef LOG_DOT_H
#define LOG_DOT_H

void logREG(int success);
void logLOG(int success);
void logGSR(int success, char* groupNumber, char* groupName);
void logSLT(int success, char* groupNumber, char* groupName);
void logPST(int success, char* groupNumber, char* groupName, int messageNumber);
void logRTV(int success, int amountMessages, char** messages);
void logError(char* message);

#endif