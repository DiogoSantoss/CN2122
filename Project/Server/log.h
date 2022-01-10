#ifndef LOG_DOT_H
#define LOG_DOT_H

#include "colors.h"

void logError(char* message);
void logTCP(char* IP, int port);
void logUDP(char* IP, int port);

#endif