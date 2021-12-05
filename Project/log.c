#include <stdio.h>

/**
 * Types of messages (in user):
 * 
 * User successfully registered
 * You are now logged in
 * New group created and subscribed: GROUP_NUMBER – “GROUP_NAME”
 * Group GROUP_NUMBER – “GROUP_NAME” is now the active group
 * posted message number MESSAGE_NUMBER to group GROUP_NUMBER - “GROUP_NAME”
 * NUMBER_OF_MESSAGES message(s) retrieved:
 *  MESSAGE_NUMBER – “MESSAGE_CONTENT”; file stored: FILE_NAME
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

void logREG(int success){
    if(success)
        printf("User successfully registered.\n");
    else    
        printf("User failled to registered.\n");
}

void logLOG(int success){
    if(success)
        printf("You are now logged in.\n");
    else    
        printf("Failled login.\n");
}

void logGSR(int success, char* groupNumber, char* groupName){
    if(success)
        printf("New group created and subscribed: %s – “%s”.\n",groupNumber,groupName);
    else    
        printf("Failled to create new group: %s – “%s”.\n",groupNumber,groupName);
}

void logSLT(int success, char* groupNumber, char* groupName){
    if(success)
        printf("Group %s – “%s” is now the active group.\n",groupNumber,groupName);
    else    
        printf("Failled to select Group %s – “%s”.\n",groupNumber,groupName);
}

void logPST(int success, char* groupNumber, char* groupName, int messageNumber){
    if(success)
        printf("posted message number %d to group %s - “%s”.\n",messageNumber,groupNumber,groupName);
    else    
        printf("Failled to post message number %d to group %s - “%s”.\n",messageNumber,groupNumber,groupName);
}

void logRTV(int success, int amountMessages, char** messages){
    if(success){
        printf("%d message(s) retrieved:\n",amountMessages);
        for(int i = 0; i<amountMessages; i++){
            printf("%s",messages[i]);
        }
    } else {
        printf("Failled to retreive messages.\n");
    }
}

void logError(char* message){
    fprintf(stderr, "ERROR: %s\n", message);
}