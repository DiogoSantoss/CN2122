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

void logREG(char* message){
    
    if(!strcmp(message,"RRG OK\n")){
        colorGreen();
        printf("register: User successfully registered.\n");

    } else if (!strcmp(message,"RRG DUP\n")){
        colorYellow();
        printf("register: User failed to registered because it's a duplicate.\n");

    } else if (!strcmp(message,"RRG NOK\n")){
        colorYellow();
        printf("register: User failed to register.\n");

    } else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("register: A fatal error has ocurred.");

    } else{
        logError("register: Unexpected message from server.");
    }
    colorReset();
}

void logUNR(char* message){

    if(!strcmp(message,"RUN OK\n")){
        colorGreen();
        printf("unregister: User successfully unregistered.\n");

    } else if (!strcmp(message,"RUN NOK\n")){
        colorYellow();
        printf("unregister: User failed to unregister.\n");

    } else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("unregister: A fatal error has ocurred.");

    } else{
        logError("unregister: Unexpected message from server.");
    }
    colorReset();
}

void logLOG(char* message){
    
    if(!strcmp(message,"RLO OK\n")){
        colorGreen();
        printf("login: User successfully logged in.\n");

    } else if (!strcmp(message,"RLO NOK\n")){
        colorYellow();
        printf("login: User failed to login.\n");

    } else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("login: A fatal error has ocurred.");

    } else{
        logError("login: Unexpected message from server.");
    }
    colorReset();
}

void logOUT(char* message){
    
    if(!strcmp(message,"ROU OK\n")){
        colorGreen();
        printf("logout: User successfully logged out.\n");

    } else if (!strcmp(message,"ROU NOK\n")){
        colorYellow();
        printf("logout: User failed to logout.\n");

    } else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("logout: A fatal error has ocurred.");

    } else{
        logError("logout: Unexpected message from server.");
    }
    colorReset();
}

void logSU(char* message){
    colorGreen();
    printf("Current UID: %s\n",message);
    colorReset();
}

void logGLS(char* message){

    char functionName[10];
    char command[MAXSIZE], nGroups[3], extra[EXTRAMAXSIZE], extraCopy[EXTRAMAXSIZE];
    char GID[3], GName[25], MID[5];

    sscanf(message, "%s %s %[^\n]s", command, nGroups, extra);
    
    if(!strcmp(message, "RGL 0\n")){
        colorGreen();
        printf("groups: No groups available to list.\n");
    }
    else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("groups: A fatal error has ocurred.");
    }
    else{
        colorGreen();
        strcpy(extraCopy, extra);

        if (!strcmp(command, "RGL"))
            printf("groups: List of groups:\n");
        else{
            logError("groups: A fatal error has ocurred.");
            return;
        }
        // Verifies if groups given by server are valid
        for (int i = 0; i < atoi(nGroups); i++){
            sscanf(extraCopy, "%s %s %s %[^\n]s", GID, GName, MID, extraCopy);
            if(!checkStringIsNumber(GID) || !checkStringIsGroupName(GName) || !checkStringIsNumber(MID)){
                logError("groups: Unexpected message from server.");
                return;
            }
        }
        // Prints groups 
        for (int i = 0; i < atoi(nGroups); i++){
            i % 2 == 0 ? colorCyan() : colorBlue(); // coloring
            sscanf(extra, "%s %s %s %[^\n]s", GID, GName, MID, extra);
            printf("Group ID: %s\tGroup Name: %-30.30sLast Message ID: %s\n", GID, GName, MID);
        }
    }
    colorReset();
}

void logGSR(char* message){

    char rgs[4], status[8], GID[3], extra[MAXSIZE];
    char string[MAXSIZE + 9];
    sscanf(message, "%s %s %s %s", rgs, status, GID, extra);

    sprintf(string, "RGS NEW %s\n", GID);

    if(!strcmp(message,"RGS OK\n")){
        colorGreen();
        printf("subscribe: User successfully subscribed.\n");

    } else if (!strcmp(message,"RGS NOK\n")){
        colorYellow();
        printf("subscribe: User failed to subscribe.\n");

    } else if (!strcmp(message,string)){
        colorGreen();
        printf("subscribe: New group %s was created and subscribed.\n", GID);

    } else if (!strcmp(message, "RGS E_USR\n")){
        colorYellow();
        printf("subscribe: Invalid UID.\n");

    } else if (!strcmp(message, "RGS E_GRP\n")){
        colorYellow();
        printf("subscribe: Invalid GID.\n");

    } else if (!strcmp(message, "RGS E_GNAME\n")){
        colorYellow();
        printf("subscribe: Invalid Group Name.\n");

    } else if (!strcmp(message, "RGS E_FULL\n")){
        colorYellow();
        printf("subscribe: New group could not be created - Group limit exceeded.\n");

    } else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("subscribe: A fatal error has ocurred.");

    } else{
        logError("subscribe: Unexpected message from server.");
    }
    colorReset();
}

void logGUR(char* message){

    if(!strcmp(message,"RGU OK\n")){
        colorGreen();
        printf("unsubscribe: User successfully unsubscribed.\n");

    } else if(!strcmp(message, "RGU NOK\n")){
        colorYellow();
        printf("unsubscribe: User unsuccessfully unsubscribed.\n");

    } else if(!strcmp(message, "RGU E_USR\n")){
        colorYellow();
        printf("unsubscribe Invalid UID.\n");

    } else if(!strcmp(message, "RGU E_GRP\n")){
        colorYellow();
        printf("unsubscribe: Invalid Group Name.\n");

    } else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("unsubscribe: A fatal error has ocurred.");
        
    } else{
        logError("subscribe: Unexpected message from server.");
    }
    colorReset();
}

void logGLM(char* message){

    char functionName[10];
    char command[MAXSIZE], nGroups[3], extra[EXTRAMAXSIZE], extraCopy[EXTRAMAXSIZE];
    char GID[3], GName[25], MID[5];

    sscanf(message, "%s %s %[^\n]s", command, nGroups, extra);
    
    if(!strcmp(message, "RGM 0\n")){
        colorGreen();
        printf("my_groups: No groups available to list.\n");
    }
    else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("my_groups: A fatal error has ocurred.");
    }
    else{
        colorGreen();
        strcpy(extraCopy, extra);

        if(!strcmp(command, "RGM"))
            printf("my_groups: List of groups:\n");
        else{
            logError("my_groups: A fatal error has ocurred.");
            return;
        }
        for (int i = 0; i < atoi(nGroups); i++){
            sscanf(extraCopy, "%s %s %s %[^\n]s", GID, GName, MID, extraCopy);
            if(!checkStringIsNumber(GID) || !checkStringIsGroupName(GName) || !checkStringIsNumber(MID)){
                logError("my_groups: Unexpected message from server.");
                return;
            }
        }
        // Prints groups 
        for (int i = 0; i < atoi(nGroups); i++){
            i % 2 == 0 ? colorCyan() : colorBlue(); // coloring
            sscanf(extra, "%s %s %s %[^\n]s", GID, GName, MID, extra);
            printf("Group ID: %s\tGroup Name: %-30.30sLast Message ID: %s\n", GID, GName, MID);
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

void logULS(char* message){

    char command[MAXSIZE], status[3], GName[25], extra[EXTRAMAXSIZE], extraCopy[EXTRAMAXSIZE];
    char userIDTemp[6];
    int length;
    
    memset(extra, 0, EXTRAMAXSIZE);
    memset(extraCopy, 0, EXTRAMAXSIZE);
    sscanf(message, "%s %s %s %[^\n]s", command, status, GName, extra);

    if(!strcmp(message, "RUL NOK\n")){
        colorYellow();
        printf("ulist: This group does not exist.\n");
    }
    else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("ulist: A fatal error has ocurred.");
    }
    else if(!strcmp(command,"RUL") && !strcmp(status,"OK")){
        colorGreen();
        strcpy(extraCopy, extra);

        length = (strlen(extra) + 1) / 6;
        // Verifies if users given by server are valid
        for (int i = 0; i < length; i++){
            sscanf(extraCopy, "%s %[^\n]s", userIDTemp, extraCopy);
            if(!checkStringIsNumber(userIDTemp) || strlen(userIDTemp) != 5){
                logError("ulist: Unexpected message from server.");
                return;
            }
        }  
        // Prints groups 
        printf("List of UIDs subscribed to %s\n", GName);
        for (int i = 0; i < length; i++){
            i % 2 == 0 ? colorCyan() : colorBlue(); // coloring
            sscanf(extra, "%s %[^\n]s", userIDTemp, extra);
            printf("%s\n", userIDTemp);
        }   
    }
    else{
        logError("ulist: Failed to list the users.");
    }
    colorReset();
}

void logPST(char* message){

    char command[4], status[8], extra[MAXSIZE];
    sscanf(message, "%s %s %s\n", command, status, extra);

    if(!strcmp(message, "RPT NOK\n")){
        colorYellow();
        printf("post: Failed to post the message.\n");
    }
    else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("post: A fatal error has ocurred.");
    }
    else if(strlen(status) == 4 && checkStringIsNumber(status)){
        colorGreen();
        printf("post: Message with number %s was successfully posted.\n", status);
    }
    else{
        logError("post: A fatal error has ocurred with the message number.");  
    }
    colorReset();
}

int logRTV(char* message){

    int success = FALSE;
    char rrt[10], status[10], numberOfMessages[10], extra[MAXSIZE];

    sscanf(message, "%s %s %s %s\n", rrt, status, numberOfMessages, extra);

    if(!strcmp(message, "RRT NOK\n")){
        colorYellow();
        printf("retrieve: Failed to retrieve the message.\n");
    }
    else if(!strcmp(message, "ERR\n") || !strcmp(message, "ERROR\n")){
        logError("retrieve: A fatal error has ocurred.");
    }
    else if(!strcmp(message,"RRT EOF\n")){
        colorGreen();
        printf("retrieve: 0 messages retrieved.\n");
    }
    else if(!strcmp(rrt,"RRT") && !strcmp(status,"OK") && checkStringIsNumber(numberOfMessages)){
        colorGreen();
        printf("%s message(s) retrieved:\n", numberOfMessages);
        success = TRUE;
    }
    else{
        logError("retrieve: Server response wrongly formatted");
    }
    colorReset();
    return success;
}