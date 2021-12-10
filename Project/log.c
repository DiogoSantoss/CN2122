#include <stdio.h>
#include <string.h>

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

// should be stderr ?
void logError(char* message){
    printf("%s\n", message);
}

void logREG(char* message){
    if(!strcmp(message,"RRG OK\n")){
        printf("User successfully registered.\n");

    } else if (!strcmp(message,"RRG DUP\n")){
        printf("User failled to registered because it's a duplicate.\n");

    } else if (!strcmp(message,"RRG NOK\n")){
        printf("User failed to register.\n");

    } else {
        logError(message);
    }
}

void logUNR(char* message){
    if(!strcmp(message,"RUN OK\n")){
        printf("User successfully unregistered.\n");

    } else if (!strcmp(message,"RUN NOK\n")){
        printf("User failed to unregister.\n");

    } else {
        logError(message);
    }
}


void logLOG(char* message){
    if(!strcmp(message,"RLO OK\n")){
        printf("You are now logged in.\n");

    } else if (!strcmp(message,"RLO NOK\n")){
        printf("Failed to login.\n");

    } else {
        logError(message);
    }
}

void logOUT(char* message){
    if(!strcmp(message,"ROU OK\n")){
        printf("You are now logged out.\n");

    } else if (!strcmp(message,"ROU NOK\n")){
        printf("Failed to logout.\n");

    } else {
        logError(message);
    }
}

void logGSR(char* message){

    //The fact that any message can be an argument implies that any variable can be full with that message
    char rgs[MAXSIZE], status[MAXSIZE], GID[MAXSIZE], extra[MAXSIZE];
    char string[MAXSIZE + 9];
    sscanf(message, "%s %s %s %s", rgs, status, GID, extra);

    sprintf(string, "RGS NEW %s\n", GID);

    if(!strcmp(message,"RGS OK\n")){
        printf("A group was subscribed.\n");
    } 
    else if (!strcmp(message,string)){
        printf("New group %s was created and subscribed.\n", GID);
    } 
    else if (!strcmp(message, "RGS E_USR\n")){
        printf("Invalid UID.\n");
    }
    else if (!strcmp(message, "RGS E_GRP\n")){
        printf("Invalid GID.\n");
    }
    else if (!strcmp(message, "RGS E_GNAME\n")){
        printf("Invalid Group Name.\n");
    }
    else if (!strcmp(message, "RGS E_FULL\n")){
        printf("New group could not be created - Group limit exceded).\n");
    }
    else {
        logError(message);
    }
}

void logSLT(int success, char* groupNumber, char* groupName){
    if(success)
        printf("Group %s - “%s” is now the active group.\n",groupNumber,groupName);
    else    
        printf("Failed to select Group %s - “%s”.\n",groupNumber,groupName);
}

void logPST(int success, char* groupNumber, char* groupName, int messageNumber){
    if(success)
        printf("Posted message number %d to group %s - “%s”.\n",messageNumber,groupNumber,groupName);
    else    
        printf("Failed to post message number %d to group %s - “%s”.\n",messageNumber,groupNumber,groupName);
}

void logRTV(int success, int amountMessages, char** messages){
    if(success){
        printf("%d message(s) retrieved:\n",amountMessages);
        for(int i = 0; i<amountMessages; i++){
            printf("%s",messages[i]);
        }
    } else {
        printf("Failed to retreive messages.\n");
    }
}