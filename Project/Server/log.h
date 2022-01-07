#ifndef LOG_DOT_H
#define LOG_DOT_H

#include "colors.h"

void logError(char* message);
void logTCP(int IP, int port, char* command);
void logUDP(int IP, int port, char* command);

#endif