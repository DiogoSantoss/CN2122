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

void logGLS(){
    printf("Listed all existing groups\n"); // TODO NEED TO LIST ALL GROUPS ?
}

void logGSR(char* UID, char* GID){
    printf("User %s subscribed to group %s\n", UID, GID);
}

void logGUR(char* UID, char* GID){
    printf("User %s unsubscribed from group %s\n", UID, GID);
}

void logGLM(char* UID){
    printf("Listed groups that user %s is subscribed to\n", UID);
}

void logULS(char* GID){
    printf("Listed all users subscribed to group %s\n", GID);
}

void logPST(char* UID, char* GID){
    printf("User %s successfully posted to group %s\n", UID, GID); // TODO NEED SHOW MESSAGE ?
}

void logRTV(char* UID, char* GID, int numberOfMessages){
    printf("User %s has retrieved %d message(s) from group %s\n", UID, numberOfMessages, GID); // TODO NEED SHOW MESSAGES ?
}
