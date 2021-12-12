#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "colors.h"

/**
 * Types of messages (in user):
 * 
 * User successfully registered
 * You are now logged in
 * New group created and subscribed: GROUP_NUMBER - “GROUP_NAME”
 * Group GROUP_NUMBER - “GROUP_NAME” is now the active group
 * posted message number MESSAGE_NUMBER to group GROUP_NUMBER - “GROUP_NAME”
 * NUMBER_OF_MESSAGES message(s) retrieved:
 *  MESSAGE_NUMBER - “MESSAGE_CONTENT”; file stored: FILE_NAME
 * 
 * Types of messages (in server):
 * 
 * UID=USER_ID: new user
 * UID=USER_ID: login ok
 * UID=USER_ID: new group: GROUP_NUMBER - “GROUP_NAME”
 * UID=USER_ID: post group GROUP_NUMBER: 
 *              “MESSAGE_CONTENT” FILE_NAME
 * UID=USER_ID: retrieve group GROUP_NUMBER, message(s):
 *              MESSAGE_NUMBER - USER_ID_AUTHOR “MESSAGE_CONTENT” FILE_NAME
 * 
*/

//Constants
#define MAXSIZE 274
#define EXTRAMAXSIZE 3268

//Boolean
#define TRUE 1
#define FALSE 0

// should be stderr ?
void logError(char* message){
    red();
    printf("%s\n", message);
    reset();
}

void logREG(char* message){
    green();
    if(!strcmp(message,"RRG OK\n")){
        printf("reg: User successfully registered.\n");

    } else if (!strcmp(message,"RRG DUP\n")){
        printf("reg: User failled to registered because it's a duplicate.\n");

    } else if (!strcmp(message,"RRG NOK\n")){
        printf("reg: User failed to register.\n");

    } else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("reply: A fatal error has ocurred.");
    }
    reset();
}

void logUNR(char* message){
    green();
    if(!strcmp(message,"RUN OK\n")){
        printf("unregister: User successfully unregistered.\n");

    } else if (!strcmp(message,"RUN NOK\n")){
        printf("unregister: User failed to unregister.\n");

    } else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("reply: A fatal error has ocurred.");
    }
    reset();
}

void logLOG(char* message){
    green();
    if(!strcmp(message,"RLO OK\n")){
        printf("login: You are now logged in.\n");

    } else if (!strcmp(message,"RLO NOK\n")){
        printf("login: Failed to login.\n");

    } else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("reply: A fatal error has ocurred.");
    }
    reset();
}

void logOUT(char* message){
    green();
    if(!strcmp(message,"ROU OK\n")){
        printf("logout: You are now logged out.\n");

    } else if (!strcmp(message,"ROU NOK\n")){
        printf("logout: Failed to logout.\n");

    } else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("reply: A fatal error has ocurred.");
    }
    reset();
}

void logGLS(char* message){

    char functionName[10];
    char prefix[MAXSIZE], nGroups[3], suffix[EXTRAMAXSIZE];
    char GID[3], GName[25], MID[5];

    sscanf(message, "%s %s %[^\n]s", prefix, nGroups, suffix);
    
    if(!strcmp(message, "RGL 0\n") || !strcmp(message, "RGM 0\n")){
        printf("%s: No groups available to list.\n", !strcmp(prefix, "RGL") ? "groups" : "my_groups");
    }
    else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("reply: A fatal error has ocurred.");
    }
    else{
        for (int i = 0; i < atoi(nGroups); i++){
            i % 2 == 0 ? cyan() : white(); // coloring
            sscanf(suffix, "%s %s %s %[^\n]s", GID, GName, MID, suffix);
            printf("Group ID: %s\nGroup Name: %s\nLast Message ID: %s\n\n", GID, GName, MID);
        }
        reset(); // color reset
    }
}

void logGSR(char* message){

    char rgs[4], status[8], GID[3], extra[MAXSIZE];
    char string[MAXSIZE + 9];
    sscanf(message, "%s %s %s %s", rgs, status, GID, extra);

    sprintf(string, "RGS NEW %s\n", GID);

    if(!strcmp(message,"RGS OK\n")){
        printf("subscribe: A group was subscribed.\n");

    } else if (!strcmp(message,"RGS NOK\n")){
        printf("subscribe: Failed to subscribe to group.\n");

    } else if (!strcmp(message,string)){
        printf("subscribe: New group %s was created and subscribed.\n", GID);

    } else if (!strcmp(message, "RGS E_USR\n")){
        printf("subscribe: Invalid UID.\n");

    } else if (!strcmp(message, "RGS E_GRP\n")){
        printf("subscribe: Invalid GID.\n");

    } else if (!strcmp(message, "RGS E_GNAME\n")){
        printf("subscribe: Invalid Group Name.\n");

    } else if (!strcmp(message, "RGS E_FULL\n")){
        printf("subscribe: New group could not be created - Group limit exceeded.\n");

    } else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("reply: A fatal error has ocurred.");
    }
}

void logGUR(char* message){

    if(!strcmp(message,"RGU OK\n")){
        printf("unsubsribe: Group successfully unsubscribed.\n");

    } else if(!strcmp(message, "RGU NOK\n")){
        printf("unsubsribe: Group unsuccessfully unsubscribed.\n");

    } else if(!strcmp(message, "RGU E_USR\n")){
        printf("unsubsribe: Invalid UID.\n");

    } else if(!strcmp(message, "RGU E_GRP\n")){
        printf("unsubsribe: Invalid Group Name.\n");

    } else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("reply: A fatal error has ocurred.");
    }
}

void logULS(char* message){

    char prefix[MAXSIZE], status[3], GName[25], suffix[EXTRAMAXSIZE];
    
    printf("MESSAGE: %s\n", message);
}

void logPST(int success, char* groupNumber, char* groupName, int messageNumber){
    if(success)
        printf("post: Posted message number %d to group %s - “%s”.\n",messageNumber,groupNumber,groupName);
    else    
        printf("post: Failed to post message number %d to group %s - “%s”.\n",messageNumber,groupNumber,groupName);
}

void logRTV(int success, int amountMessages, char** messages){
    if(success){
        printf("retrieve: %d message(s) retrieved:\n",amountMessages);
        for(int i = 0; i<amountMessages; i++){
            printf("%s",messages[i]);
        }
    } else {
        printf("retrieve: Failed to retreive messages.\n");
    }
}