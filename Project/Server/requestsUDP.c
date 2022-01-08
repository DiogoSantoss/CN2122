#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "log.h"
#include "structs.h"
#include "common.h"
#include "directories.h"

// Booleans
#define TRUE  1
#define FALSE 0

#define MAXSIZEUDP 39 //GSR is the biggest command 
#define EXTRAMAXSIZE 3169

#define MAXGROUPS 99

char* requestErrorUDP(userData user, serverData server){
    char* message = calloc(5, sizeof(char));
    strcpy(message, "ERR\n");
    return message;
}   

/**
 * Process register request.
 * @param[in] user User data
 * @param[in] request Client input to be parsed
 * @param[out] message Formarted message to respond to client
*/
char* processREG(userData user, serverData server, char* request){

    char prefix[4], suffix[MAXSIZEUDP];
    char UserID[6], password[9];

    memset(suffix, 0, MAXSIZEUDP);
    char* message = calloc(9, sizeof(char));

    sscanf(request, "%s %s %s %s", prefix, UserID, password, suffix);

    if (
        strlen(suffix) != 0 || strlen(UserID) != 5 || strlen(password) != 8 ||
        !checkStringIsNumber(UserID) || !checkStringIsAlphaNum(password)
    ){
        // Wrong size parameters
        strcpy(message, "ERR\n");
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

        if (!CreateUserDir(UserID)) strcpy(message, "RRG NOK\n");
        if (!CreatePassFile(UserID, password)) strcpy(message, "RRG NOK\n");
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

    char prefix[4], suffix[MAXSIZEUDP];
    char UserID[6], password[9];

    memset(suffix, 0, MAXSIZEUDP);
    char* message = calloc(9, sizeof(char));

    sscanf(request, "%s %s %s %s", prefix, UserID, password, suffix);

    if (
        strlen(suffix) != 0 || strlen(UserID) != 5 || strlen(password) != 8 ||
        !checkStringIsNumber(UserID) || !checkStringIsAlphaNum(password)
    ){
        // Wrong format parameters
        strcpy(message, "ERR\n");
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

        if (!DelUserFromGroups(UserID)) strcpy(message, "RUN NOK\n");
        if (!DelPassFile(UserID)) strcpy(message, "RUN NOK\n");

        if(CheckUserLogin(UserID)){
            if(!DelLoginFile(UserID)) 
                strcpy(message, "RUN NOK\n");
        }

        if (!DelUserDir(UserID)) strcpy(message, "RUN NOK\n");
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

    char prefix[4], suffix[MAXSIZEUDP];
    char UserID[6], password[9];

    memset(suffix, 0, MAXSIZEUDP);
    char* message = calloc(9, sizeof(char));

    sscanf(request, "%s %s %s %s", prefix, UserID, password, suffix);

    if (
        strlen(suffix) != 0 || strlen(UserID) != 5 || strlen(password) != 8 ||
        !checkStringIsNumber(UserID) || !checkStringIsAlphaNum(password)
    ){
        // Wrong format parameters
        strcpy(message, "ERR\n");
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

        if(!createLoginFile(UserID)) strcpy(message, "RLO NOK\n");
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

    char prefix[4], suffix[MAXSIZEUDP];
    char UserID[6], password[9];

    memset(suffix, 0, MAXSIZEUDP);
    char* message = calloc(9, sizeof(char));

    sscanf(request, "%s %s %s %s", prefix, UserID, password, suffix);

    if (
        strlen(suffix) != 0 || strlen(UserID) != 5 || strlen(password) != 8 ||
        !checkStringIsNumber(UserID) || !checkStringIsAlphaNum(password)
    ){
        // Wrong format parameters
        strcpy(message, "ERR\n");
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

        if (!DelLoginFile(UserID)) strcpy(message, "ROU NOK\n");
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

    int numberGroups;
    Group list[MAXGROUPS];;
    char prefix[4], suffix[MAXSIZEUDP];

    char* message = calloc(EXTRAMAXSIZE, sizeof(char));
    
    memset(suffix, 0, MAXSIZEUDP);
    sscanf(request, "%s %s", prefix, suffix);

    if (strlen(suffix) != 0){
        // Wrong size parameters
        strcpy(message, "ERR\n");
        return message;
    }

    // Fill list data structure with groups info
    numberGroups = ListGroupsDir(list);
    if(numberGroups == -1){
        strcpy(message,"ERR\n");
        return message;
    }
    else if(numberGroups == 0){
        strcpy(message, "RGL 0\n");
        return message;
    }

    sprintf(message, "RGL %d", numberGroups);
    for(int i = 0; i < numberGroups; i++){
        sprintf(message, "%s %s %s %s", message, list[i].groupNumber, list[i].groupName, list[i].groupLastMsg);
    }
    sprintf(message, "%s\n", message);

    return message;
}

/**
 * Process subscribe request.
 * @param[in] user User data
 * @param[in] request Client input to be parsed
 * @param[out] message Formarted message to respond to client
*/
char* processGSR(userData user, serverData server, char* request){

    char prefix[4], suffix[MAXSIZEUDP];
    char UserID[6], GroupID[3], GroupName[25];

    memset(suffix, 0, MAXSIZEUDP);
    char* message = calloc(13, sizeof(char));

    sscanf(request, "%s %s %s %s %s", prefix, UserID, GroupID, GroupName, suffix);
    int groupMax = maxGroupNumber();

    if(strlen(GroupID) == 1){
        sprintf(GroupID, "%02d", atoi(GroupID));
    }

    if (
        strlen(suffix) != 0 || strlen(UserID) != 5 || strlen(GroupID) != 2 || strlen(GroupName) > 24 || 
        !checkStringIsNumber(UserID) ||  !checkStringIsNumber(GroupID) || !checkStringIsGroupName(GroupName)
    ){
        // Wrong size parameters
        strcpy(message, "ERR\n");
        return message;
    }
    if (!UserExists(UserID) || !CheckUserLogin(UserID)){
        // Invalid UID
        strcpy(message, "RGS E_USR\n");
        return message;
    }
    // User want to create and subscribe to new group
    else if (strcmp(GroupID, "00") == 0){
       
        if (groupMax >= MAXGROUPS){
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

/**
 * Process unsubscribe request.
 * @param[in] user User data
 * @param[in] request Client input to be parsed
 * @param[out] message Formarted message to respond to client
*/
char* processGUR(userData user, serverData server, char* request){

    char prefix[4], suffix[MAXSIZEUDP];
    char UserID[6], GroupID[3];

    memset(suffix, 0, MAXSIZEUDP);
    char* message = calloc(13, sizeof(char));

    sscanf(request, "%s %s %s %s", prefix, UserID, GroupID, suffix);
    
    if (
        strlen(suffix) != 0 || strlen(UserID) != 5 || strlen(GroupID) != 2 ||
        !checkStringIsNumber(UserID) || !checkStringIsNumber(GroupID)
    ){
        // Wrong size parameters
        strcpy(message, "ERR\n");
        return message;
    }
    else if (!UserExists(UserID) || !CheckUserLogin(UserID)){
        // Invalid UID
        strcpy(message, "RGU E_USR\n");
        return message;
    }
    else if(!GroupExists(GroupID)){
        // Invalid GID
        strcpy(message, "RGU E_GRP\n");
        return message;
    }
    // User is actually subscribed to group
    else if(checkUserSubscribedToGroup(UserID, GroupID)){
        
        if(UnsubscribeUser(UserID, GroupID)){
            strcpy(message, "RGU OK\n");
            return message;
        }
        else{
            strcpy(message, "RGU NOK\n");
            return message;
        }
    }
    // In this case the user is not subscribed to the group
    // But it returns success nonetheless
    else if(!checkUserSubscribedToGroup(UserID, GroupID)){
        
        strcpy(message, "RGU OK\n");
        return message;
    }
}

/**
 * Process my_groups request.
 * @param[in] user User data
 * @param[in] request Client input to be parsed
 * @param[out] message Formarted message to respond to client
*/
char* processGLM(userData user, serverData server, char* request){

    int numberGroups;
    int subscribedGroups = 0;
    Group list[MAXGROUPS];
    char prefix[4], suffix[MAXSIZEUDP];
    char UserID[6];

    char* message = calloc(EXTRAMAXSIZE, sizeof(char));
    
    memset(suffix, 0, MAXSIZEUDP);
    sscanf(request, "%s %s %s", prefix, UserID, suffix);

    if (strlen(suffix) != 0 || strlen(UserID) != 5 || !checkStringIsNumber(UserID)){
        // Wrong size parameters
        strcpy(message, "ERR\n");
        return message;
    }
    else if (!UserExists(UserID) || !CheckUserLogin(UserID)){
        // Invalid UID
        strcpy(message, "RGM E_USR\n");
        return message;
    }

    // Fill list data structure with groups info
    numberGroups = ListGroupsDir(list);
    if(numberGroups == -1){
        strcpy(message,"ERR\n");
        return message;
    }
    else if(numberGroups == 0){
        strcpy(message, "RGM 0\n");
        return message;
    }

    for (int i = 0; i < numberGroups; i++){
        if(checkUserSubscribedToGroup(UserID, list[i].groupNumber)){
            subscribedGroups ++;
        }
    }
    
    sprintf(message, "RGM %d", subscribedGroups);
    
    for(int i = 0; i < numberGroups; i++){
        if(checkUserSubscribedToGroup(UserID, list[i].groupNumber)){
            sprintf(message, "%s %s %s %s", message, list[i].groupNumber, list[i].groupName, list[i].groupLastMsg);
        }
    }
    sprintf(message, "%s\n",message);
        
    return message;
}