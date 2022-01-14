#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "colors.h"
#include "common.h"

//Boolean
#define TRUE 1
#define FALSE 0

// Max size read from server response
#define MAXRESPONSESIZE 3274

void logError(char* message){
    colorRed();
    printf("%s\n", message);
    colorReset();
}

void logREG(char* message){
    
    if(!strcmp(message,"RRG OK\n")){
        colorGreen();
        printf("User successfully registered.\n");

    } else if (!strcmp(message,"RRG DUP\n")){
        colorYellow();
        printf("User failed to registered because it's a duplicate.\n");

    } else if (!strcmp(message,"RRG NOK\n")){
        colorYellow();
        printf("User failed to register.\n");

    } else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("A fatal error has ocurred.");

    } else{
        logError("Unexpected message from server.");
    }
    colorReset();
}

void logUNR(char* message){

    if(!strcmp(message,"RUN OK\n")){
        colorGreen();
        printf("User successfully unregistered.\n");

    } else if (!strcmp(message,"RUN NOK\n")){
        colorYellow();
        printf("User failed to unregister.\n");

    } else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("A fatal error has ocurred.");

    } else{
        logError("Unexpected message from server.");
    }
    colorReset();
}

void logLOG(char* message){
    
    if(!strcmp(message,"RLO OK\n")){
        colorGreen();
        printf("User successfully logged in.\n");

    } else if (!strcmp(message,"RLO NOK\n")){
        colorYellow();
        printf("User failed to login.\n");

    } else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("A fatal error has ocurred.");

    } else{
        logError("Unexpected message from server.");
    }
    colorReset();
}

void logOUT(char* message){
    
    if(!strcmp(message,"ROU OK\n")){
        colorGreen();
        printf("User successfully logged out.\n");

    } else if (!strcmp(message,"ROU NOK\n")){
        colorYellow();
        printf("User failed to logout.\n");

    } else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("A fatal error has ocurred.");

    } else{
        logError("Unexpected message from server.");
    }
    colorReset();
}

void logSU(char* message){
    colorGreen();
    printf("Current UID: %s\n",message);
    colorReset();
}

void logGLS(char* message){

    // TODO should we allocate more space for each?
    char command[MAXRESPONSESIZE], nGroups[MAXRESPONSESIZE], extra[MAXRESPONSESIZE], extraCopy[MAXRESPONSESIZE];
    char groupID[MAXRESPONSESIZE], groupName[MAXRESPONSESIZE], messageID[MAXRESPONSESIZE];

    sscanf(message, "%s %s %[^\n]s", command, nGroups, extra);
    
    if(!strcmp(message, "RGL 0\n")){
        colorGreen();
        printf("No groups available to list.\n");
    }
    else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("A fatal error has ocurred.");
    }
    else{
        colorGreen();
        strcpy(extraCopy, extra);

        if (!strcmp(command, "RGL"))
            printf("List of groups:\n");
        else{
            logError("A fatal error has ocurred.");
            return;
        }

        if(!checkStringIsNumber(nGroups)){
            logError("Server response bad formatting.");
            return;
        }
        // Verifies if groups given by server are valid
        for (int i = 0; i < atoi(nGroups); i++){
            sscanf(extraCopy, "%s %s %s %[^\n]s", groupID, groupName, messageID, extraCopy);
            if(
                strlen(groupID) != 2 || strlen(groupName) > 24 || strlen(messageID) != 4 ||
                !checkStringIsNumber(groupID) || !checkStringIsGroupName(groupName) || !checkStringIsNumber(messageID)
            ){
                logError("Unexpected message from server.");
                return;
            }
        }
        // Prints groups 
        for (int i = 0; i < atoi(nGroups); i++){
            i % 2 == 0 ? colorCyan() : colorBlue(); // coloring
            sscanf(extra, "%s %s %s %[^\n]s", groupID, groupName, messageID, extra);
            printf("Group ID: %s\tGroup Name: %-30.30sLast Message ID: %s\n", groupID, groupName, messageID);
        }
    }
    colorReset();
}

void logGSR(char* message){

    // TODO should we allocate more space for each?
    char command[MAXRESPONSESIZE], status[MAXRESPONSESIZE], groupID[MAXRESPONSESIZE], extra[MAXRESPONSESIZE];
    char string[MAXRESPONSESIZE + 9];
    sscanf(message, "%s %s %s %s", command, status, groupID, extra);

    sprintf(string, "RGS NEW %s\n", groupID);

    if(!strcmp(message,"RGS OK\n")){
        colorGreen();
        printf("User successfully subscribed.\n");

    } else if (!strcmp(message,"RGS NOK\n")){
        colorYellow();
        printf("User failed to subscribe.\n");

    } else if (!strcmp(message,string)){
        colorGreen();
        printf("New group %s was created and subscribed.\n", groupID);

    } else if (!strcmp(message, "RGS E_USR\n")){
        colorYellow();
        printf("Invalid UID.\n");

    } else if (!strcmp(message, "RGS E_GRP\n")){
        colorYellow();
        printf("Invalid groupID.\n");

    } else if (!strcmp(message, "RGS E_GNAME\n")){
        colorYellow();
        printf("Invalid Group Name.\n");

    } else if (!strcmp(message, "RGS E_FULL\n")){
        colorYellow();
        printf("New group could not be created - Group limit exceeded.\n");

    } else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("A fatal error has ocurred.");

    } else{
        logError("Unexpected message from server.");
    }
    colorReset();
}

void logGUR(char* message){

    if(!strcmp(message,"RGU OK\n")){
        colorGreen();
        printf("User successfully unsubscribed.\n");

    } else if(!strcmp(message, "RGU NOK\n")){
        colorYellow();
        printf("User unsuccessfully unsubscribed.\n");

    } else if(!strcmp(message, "RGU E_USR\n")){
        colorYellow();
        printf("Invalid UID.\n");

    } else if(!strcmp(message, "RGU E_GRP\n")){
        colorYellow();
        printf("Invalid Group Name.\n");

    } else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("A fatal error has ocurred.");
        
    } else{
        logError("Unexpected message from server.");
    }
    colorReset();
}

void logGLM(char* message){

    // TODO should we allocate more space for each?
    char command[MAXRESPONSESIZE], nGroups[MAXRESPONSESIZE], extra[MAXRESPONSESIZE], extraCopy[MAXRESPONSESIZE];
    char groupID[MAXRESPONSESIZE], groupName[MAXRESPONSESIZE], messageID[MAXRESPONSESIZE];

    sscanf(message, "%s %s %[^\n]s", command, nGroups, extra);
    
    if(!strcmp(message, "RGM 0\n")){
        colorGreen();
        printf("No groups available to list.\n");
    }
    else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("A fatal error has ocurred.");
    }
    else{
        colorGreen();
        strcpy(extraCopy, extra);

        if(!strcmp(command, "RGM"))
            printf("List of groups:\n");
        else{
            logError("A fatal error has ocurred.");
            return;
        }

        if(!checkStringIsNumber(nGroups)){
            logError("Server response bad formatting.");
            return;
        }
        // Verifies if groups given by server are valid
        for (int i = 0; i < atoi(nGroups); i++){
            sscanf(extraCopy, "%s %s %s %[^\n]s", groupID, groupName, messageID, extraCopy);
            if(
                strlen(groupID) != 2 || strlen(groupName) > 24 || strlen(messageID) != 4 ||
                !checkStringIsNumber(groupID) || !checkStringIsGroupName(groupName) || !checkStringIsNumber(messageID)
            ){
                logError("Unexpected message from server.");
                return;
            }
        }
        // Prints groups 
        for (int i = 0; i < atoi(nGroups); i++){
            i % 2 == 0 ? colorCyan() : colorBlue();
            sscanf(extra, "%s %s %s %[^\n]s", groupID, groupName, messageID, extra);
            printf("Group ID: %s\tGroup Name: %-30.30sLast Message ID: %s\n", groupID, groupName, messageID);
        }
    }
    colorReset();
}

void logSAG(char* message){
    colorGreen();
    printf("Current group selected: %s\n", message);
    colorReset();
}

void logSG(char* message){
    colorGreen();
    printf("Current GID: %s\n", message);
    colorReset();
}

int logULS(char* message){

    int success = FALSE;
    
    char command[7], status[7], extra[7]; 
    sscanf(message, "%s %s %s\n", command, status, extra);

    if(!strcmp(message, "RUL NOK")){
        colorYellow();
        printf("This group does not exist.\n");
    }
    else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("A fatal error has ocurred.");
    }
    else if(!strcmp(message, "RUL OK ")){
        success = TRUE;
    }
    else{
        logError("Failed to list the users.");  
    }
    colorReset();
    return success;
}

void logPST(char* message){

    // 7 size of the message read from the server in the main function
    char command[9], status[9], extra[9]; 

    sscanf(message, "%s %s %s\n", command, status, extra);

    if(!strcmp(message, "RPT NOK\n")){
        colorYellow();
        printf("Failed to post the message.\n");
    }
    else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("A fatal error has ocurred.");
    }
    else if(strlen(status) == 4 && checkStringIsNumber(status)){
        colorGreen();
        printf("Message with number %s was successfully posted.\n", status);
    }
    else{
        logError("A fatal error has ocurred with the message number.");  
    }
    colorReset();
}

int logRTV(char* message){

    int success = FALSE;
    // 7 size of the message read from the server in the main function
    char command[9], status[9], numberOfMessages[9], extra[9];

    sscanf(message, "%s %s %s %s\n", command, status, numberOfMessages, extra);

    if(!strcmp(message, "RRT NOK\n")){
        colorYellow();
        printf("Failed to retrieve the message.\n");
    }
    else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("A fatal error has ocurred.");
    }
    else if(!strcmp(message,"RRT EOF\n")){
        colorGreen();
        printf("0 messages retrieved.\n");
    }
    else if(!strcmp(command,"RRT") && !strcmp(status,"OK") && checkStringIsNumber(numberOfMessages)){
        colorGreen();
        printf("%s message(s) retrieved:\n", numberOfMessages);
        success = TRUE;
    }
    else{
        logError("Server response wrongly formatted");
    }
    colorReset();
    return success;
}