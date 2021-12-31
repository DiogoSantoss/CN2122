#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "log.h"
#include "structs.h"
#include "common.h"
#include "directories.h"

int mkdir(const char *pathname, mode_t mode);

// Booleans
#define TRUE  1
#define FALSE 0

#define MAXSIZEUDP 39 //GSR is the biggest command 
#define EXTRAMAXSIZE 3169

/**
 * Process register request.
 * @param[in] user User data
 * @param[in] request Client input to be parsed
 * @param[out] message Formarted message to respond to client
*/
char* processREG(userData user, serverData server, char* request){

    char prefix[4], sufix[MAXSIZEUDP];
    char UserID[6], password[9];

    memset(sufix, 0, MAXSIZEUDP);
    char* message = calloc(9, sizeof(char));

    sscanf(request, "%s %s %s %s", prefix, UserID, password, sufix);

    printf("Request: %s", request);

    if (strlen(sufix) != 0 || strlen(UserID) != 5 || strlen(password) != 8){
        // Wrong size parameters
        //printf("sizesufix: %d\nsizeUID: %d\nsizepassword: %d\n", strlen(sufix), strlen(UserID), strlen(password));
        //printf("sufix: %s\n", sufix);
        strcpy(message, "RRG NOK\n");
        return message;
    }
    else if (!checkStringIsNumber(UserID) || !checkStringIsAlphaNum(password)){
        // Forbidden character in parameters
        strcpy(message, "RRG NOK\n");
        return message;
    }
    else if (UserExists(UserID)){
        // User exists
        strcpy(message, "RRG DUP\n");
        return message;
    }
    else{
        // Everything ok
        strcpy(message, "RRG OK\n");

        if (!CreateUserDir(UserID)) strcpy(message, "ERR\n");
        if (!CreatePassFile(UserID, password)) strcpy(message, "ERR\n");
    }

    return message;
}

/**
 * Process unregister request.
 * @param[in] user User data
 * @param[in] request Client input to be parsed
 * @param[out] message Formarted message to respond to client
*/
char* processURN(userData user, serverData server, char* request){

    char prefix[4], sufix[MAXSIZEUDP];
    char UserID[6], password[9];

    memset(sufix, 0, MAXSIZEUDP);
    char* message = calloc(9, sizeof(char));

    sscanf(request, "%s %s %s %s", prefix, UserID, password, sufix);

    if (strlen(sufix) != 0 || strlen(UserID) != 5 || strlen(password) != 8){
        // Wrong size parameters
        strcpy(message, "RUN NOK\n");
        return message;
    }
    else if (!checkStringIsNumber(UserID) || !checkStringIsAlphaNum(password)){
        // Forbidden character in parameters
        strcpy(message, "RUN NOK\n");
        return message;
    }
    else if (!UserExists(UserID) || !checkUserPassword(UserID,password)){
        // User doesn't exists or wrong password
        strcpy(message, "RUN NOK\n");
        return message;
    }
    else{
        // Everything ok
        strcpy(message, "RUN OK\n");

        if (!DelPassFile(UserID)) strcpy(message, "ERR\n");
        if (!DelLoginFile(UserID)) strcpy(message, "ERR\n");
        if (!DelUserDir(UserID)) strcpy(message, "ERR\n");
    }

    return message;
}

/**
 * Process login request.
 * @param[in] user User data
 * @param[in] request Client input to be parsed
 * @param[out] message Formarted message to respond to client
*/
char* processLOG(userData user, serverData server, char* request){

    char prefix[4], sufix[MAXSIZEUDP];
    char UserID[6], password[9];

    memset(sufix, 0, MAXSIZEUDP);
    char* message = calloc(9, sizeof(char));

    sscanf(request, "%s %s %s %s", prefix, UserID, password, sufix);

    if (strlen(sufix) != 0 || strlen(UserID) != 5 || strlen(password) != 8){
        // Wrong size parameters
        strcpy(message, "RLO NOK\n");
        return message;
    }
    else if (!checkStringIsNumber(UserID) || !checkStringIsAlphaNum(password)){
        // Forbidden character in parameters
        strcpy(message, "RLO NOK\n");
        return message;
    }
    else if (!UserExists(UserID) || !checkUserPassword(UserID,password)){
        // User doesn't exists or wrong password
        strcpy(message, "RLO NOK\n");
        return message;
    }
    else{
        // Everything ok
        strcpy(message, "RLO OK\n");

        if(!createLoginFile(UserID)) strcpy(message, "ERR\n");
    }

    return message;
}

/**
 * Process logout request.
 * @param[in] user User data
 * @param[in] request Client input to be parsed
 * @param[out] message Formarted message to respond to client
*/
char* processOUT(userData user, serverData server, char* request){

    char prefix[4], sufix[MAXSIZEUDP];
    char UserID[6], password[9];

    memset(sufix, 0, MAXSIZEUDP);
    char* message = calloc(9, sizeof(char));

    sscanf(request, "%s %s %s %s", prefix, UserID, password, sufix);

    if (strlen(sufix) != 0 || strlen(UserID) != 5 || strlen(password) != 8){
        // Wrong size parameters
        strcpy(message, "ROU NOK\n");
        return message;
    }
    else if (!checkStringIsNumber(UserID) || !checkStringIsAlphaNum(password)){
        // Forbidden character in parameters
        strcpy(message, "ROU NOK\n");
        return message;
    }
    else if (!UserExists(UserID) || !checkUserPassword(UserID,password)){
        // User doesn't exists or wrong password
        strcpy(message, "ROU NOK\n");
        return message;
    }
    else{
        // Everything ok
        strcpy(message, "ROU OK\n");

        if (!DelLoginFile(UserID)) strcpy(message, "ERR\n");
    }

    return message;
}

/**
 * Process groups request.
 * @param[in] user User data
 * @param[in] request Client input to be parsed
 * @param[out] message Formarted message to respond to client
*/
char* processGLS(userData user, serverData server, char* request){

}

/**
 * Process subscribe request.
 * @param[in] user User data
 * @param[in] request Client input to be parsed
 * @param[out] message Formarted message to respond to client
*/
char* processGSR(userData user, serverData server, char* request){

    char prefix[4], sufix[MAXSIZEUDP];
    char UserID[6], GroupID[3], GroupName[25];

    memset(sufix, 0, MAXSIZEUDP);
    char* message = calloc(13, sizeof(char));

    sscanf(request, "%s %s %s %s %s", prefix, UserID, GroupID, GroupName, sufix);

    int groupMax = maxGroupNumber();


    if(strlen(GroupID) == 1){
        sprintf(GroupID, "%02d", atoi(GroupID));
    }

    if (strlen(sufix) != 0){
        // Wrong size parameters
        strcpy(message, "RGS NOK\n");
        return message;
    }
    if (strlen(UserID) != 5 || !checkStringIsNumber(UserID) || !UserExists(UserID)){
        // Invalid UID
        strcpy(message, "RGS E_USR\n");
        return message;
    }
    else if(strlen(GroupID) != 2 || !checkStringIsNumber(GroupID)){
        // Invalid GID
        strcpy(message, "RGS E_GRP\n");
        return message;
    }
    else if (strlen(GroupName) > 24 || !checkStringIsGroupName(GroupName)){
        // Invalid Gname
        strcpy(message, "RGS E_GNAME\n");
        return message;
    }
    // User want to create and subscribe to new group
    else if (strcmp(GroupID, "00") == 0){
       
        if (groupMax >= 99){
            // Max number of groups, can't create more
            strcpy(message, "RGS E_FULL\n");
            return message;
        }

        char newGroupID[3];
        sprintf(newGroupID, "%02d", groupMax + 1);

        if(!CreateGroupDir(newGroupID)){
            strcpy(message,"ERR\n");
            return message;
        }

        if(!CreateGroupFile(newGroupID,GroupName)){
            strcpy(message,"ERR\n");
            return message;
        }

        if(!SubscribeUser(UserID, newGroupID)){
            // Failed to subscribe user to newly created group
            strcpy(message, "RGS NOK\n");
            return message;
        }
        // Created and subscribed to new group
        sprintf(message, "RGS NEW %s\n", newGroupID);
        return message;
    }  
    // User wants to subscribe to group 
    else{ 
        if(!GroupExists(GroupID)){
            // Invalid GID
            strcpy(message, "RGS E_GRP\n");
            return message;
        }
        if(!checkGroupName(GroupID, GroupName)){
            // Invalid Gname
            strcpy(message, "RGS E_GNAME\n");
            return message;
        }
        if(!SubscribeUser(UserID, GroupID)){
            // Failed to subscribe user
            strcpy(message, "RGS NOK\n");
            return message;
        }
        // Subscribed user 
        strcpy(message, "RGS OK\n");
        return message;
    }
}