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

void printLine(){
    printf("----------------------------------------------------\n");
}

void logError(int verbose, char* message){
    if(verbose){
        colorRed();
        printf("%s\n", message);
        printLine();
        colorReset();
    }
}

void logTCP(int verbose, char* IP, int port){
    if(verbose){
        printf("Received request to TCP socket from:\n");
        printf("IP: %s\n",IP);
        printf("Port: %d\n",port);
    }
}

void logUDP(int verbose, char* IP, int port){
    if(verbose){
        printf("Received request to UDP socket from:\n");
        printf("IP: %s\n",IP);
        printf("Port: %d\n",port);
    }
}

void logREG(int verbose, char* UID){
    if(verbose){
        colorGreen();
        printf("Registered user %s\n",UID);  
        printLine();
    }
}

void logUNR(int verbose, char* UID){
    if(verbose){
        colorGreen();
        printf("Unregistered user %s\n",UID);
        printLine();
    }
}

void logLOG(int verbose, char* UID){
    if(verbose){
        colorGreen();
        printf("User %s logged in\n",UID);
        printLine();
    }
}

void logOUT(int verbose, char* UID){
    if(verbose){
        colorGreen();
        printf("User %s logged out\n",UID);
        printLine();
    }
}

void logGLS(int verbose, int nGroups){
    if(verbose){
        colorGreen();
        printf("Listed %d group(s)\n", nGroups);
        printLine();
    }
}

void logGSR(int verbose, char* UID, char* GID){
    if(verbose){
        colorGreen();
        printf("User %s subscribed to group %s\n", UID, GID);
        printLine();
    }
}

void logGUR(int verbose, char* UID, char* GID){
    if(verbose){
        colorGreen();
        printf("User %s unsubscribed from group %s\n", UID, GID);
        printLine();
    }
}

void logGLM(int verbose, char* UID){
    if(verbose){
        colorGreen();
        printf("Listed groups that user %s is subscribed to\n", UID);
        printLine();
    }
}

void logULS(int verbose, char* GID){
    if(verbose){
        colorGreen();
        printf("Listed all users subscribed to group %s\n", GID);
        printLine();
    }
}

void logPST(int verbose, char* UID, char* GID, int textSize, int fileSize, char* fileName){
    if(verbose){
        colorGreen();
        if(fileName == NULL)
            printf("User %s successfully posted to group %s a message with a text sixe of %d\n", UID, GID, textSize);
        else
            printf("User %s successfully posted to group %s a message with %d bytes and a file named %s with %d bytes\n", UID, GID, textSize, fileName, fileSize);
        printLine();
    }
}

void logRTV(int verbose, char* UID, char* GID, int numberOfMessages){
    if(verbose){
        colorGreen();
        printf("User %s has retrieved %d message(s) from group %s\n", UID, numberOfMessages, GID);
    }
}

void logRTVMessage(int verbose, int MID, int textSize, int fileSize, char* fileName){
    if(verbose){
        colorGreen();
        if(fileName == NULL)
            printf("Retrieved message %d with a text size of %d\n", MID, textSize);
        else
            printf("Retrieved message %d with a text size of %d and a file named %s with %d bytes\n", MID, textSize, fileName, fileSize);
    }
}
