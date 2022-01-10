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
    printf("-------------------------------\n");
    colorReset();
}

void logTCP(char* IP, int port){
    printf("Received request to TCP socket from:\n");
    printf("IP: %s\n",IP);
    printf("Port: %d\n",port);
}

void logUDP(char* IP, int port){
    printf("Received request to UDP socket from:\n");
    printf("IP: %s\n",IP);
    printf("Port: %d\n",port);
}

void logREG(char* UID){
    printf("Registered user %s\n",UID);
}

void logUNR(char* UID){
    printf("Unregistered user %s\n",UID);
}

void logLOG(char* UID){
    printf("User %s logged in\n",UID);
}

void logOUT(char* UID){
    printf("User %s logged out\n",UID);
}

void logGLS(char* UID){
    // TODO
}
