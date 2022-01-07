#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "colors.h"
#include "common.h"

//Boolean
#define TRUE 1
#define FALSE 0

//Constants
#define MAXSIZE 274
#define EXTRAMAXSIZE 3268

void logError(char* message){
    colorRed();
    printf("%s\n", message);
    colorReset();
}

void logTCP(int IP, int port, char* command){
    colorGreen();
    printf("Received request to TCP socket.\n");
    printf("IP: %d\n",IP);
    printf("Port: %d\n",port);
    printf("Command: %s\n",command);
    printf("-------------------------------\n");
    colorReset();
}

void logUDP(int IP, int port, char* command){
    colorGreen();
    printf("Received request to UDP socket.\n");
    printf("IP: %d\n",IP);
    printf("Port: %d\n",port);
    printf("Command: %s\n",command);
    printf("-------------------------------\n");
    colorReset();
}