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

#define MAXSIZEUDP 39       //GSR is the biggest command 
#define EXTRAMAXSIZE 3169   // TODO better name?

#define MAXGROUPS 99

/**
 * Send response to server.
 * @param user User data
 * @param response Response to send
*/
void sendUDP(userData user, char* response){

    if(sendto(user.fd, response, strlen(response), 0, (struct sockaddr*)user.addr, user.addrlen) == -1){
        logError(TRUE, "Couldn't send response via UDP socket");
    }
}

/**
 * Send error response.
 * @param user User data
 * @param server Server data
*/
void requestErrorUDP(userData user, serverData server){
    char response[5];
    strcpy(response, "ERR\n");
    sendUDP(user,response);
}   

/**
 * Process register request.
 * @param user User data
 * @param server Server data
 * @param request Client input to be parsed
*/
void processREG(userData user, serverData server, char* request){

    char command[4], userID[6], password[9], extra[MAXSIZEUDP];
    char response[9];
    memset(response,0,9);
    memset(extra, 0, MAXSIZEUDP);

    sscanf(request, "%s %s %s %s", command, userID, password, extra);

    if (
        strlen(extra) != 0 || strlen(userID) != 5 || strlen(password) != 8 ||
        !checkStringIsNumber(userID) || !checkStringIsAlphaNum(password)
    ){
        // Wrong size parameters
        strcpy(response, "ERR\n");
        logError(server.verbose, "Failed to register user because wrong format.");
        sendUDP(user,response);
        return;
    }
    else if (UserExists(userID)){
        // User already exists
        strcpy(response, "RRG DUP\n");
        logError(server.verbose, "Failed to register user because already exists.");
        sendUDP(user,response);
        return;
    }
    // Create user directory and password file
    if (CreateUserDir(userID) && CreatePassFile(userID, password)){
        strcpy(response, "RRG OK\n");
        logREG(server.verbose, userID);
    } else{
        strcpy(response, "RRG NOK\n");
        logError(server.verbose, "Failed to register user failed to create files.");
    }
    sendUDP(user, response);
}

/**
 * Process unregister request.
 * @param user User data
 * @param server Server data
 * @param request Client input to be parsed
*/
void processURN(userData user, serverData server, char* request){

    char command[4], userID[6], password[9], extra[MAXSIZEUDP];
    char response[9];
    memset(response,0,9);
    memset(extra, 0, MAXSIZEUDP);

    sscanf(request, "%s %s %s %s", command, userID, password, extra);

    if (
        strlen(extra) != 0 || strlen(userID) != 5 || strlen(password) != 8 ||
        !checkStringIsNumber(userID) || !checkStringIsAlphaNum(password)
    ){
        // Wrong format parameters
        strcpy(response, "ERR\n");
        logError(server.verbose, "Failed to unregister user because wrong format.");
        sendUDP(user,response);
        return;
    }
    else if (!UserExists(userID) || !checkUserPassword(userID,password)){
        // User doesn't exists or wrong password
        strcpy(response, "RUN NOK\n");
        logError(server.verbose, "Failed to unregister user because unknown user or wrong password.");
        sendUDP(user,response);
        return;
    }

    strcpy(response, "RUN OK\n");

    if (!DelUserFromGroups(userID))                     strcpy(response, "RUN NOK\n");
    if (!DelPassFile(userID))                           strcpy(response, "RUN NOK\n");
    if (CheckUserLogin(userID) && !DelLoginFile(userID))strcpy(response, "RUN NOK\n");
    if (!DelUserDir(userID))                            strcpy(response, "RUN NOK\n");

    if(strcmp(response,"RUN OK\n")){
        logUNR(server.verbose, userID);
    }else{
        logError(server.verbose, "Failed to unregister user because failed to delete files/directory");
    }

    sendUDP(user, response);
}

/**
 * Process login request.
 * @param user User data
 * @param server Server data
 * @param request Client input to be parsed
*/
void processLOG(userData user, serverData server, char* request){


    char command[4], userID[6], password[9], extra[MAXSIZEUDP];
    char response[9];
    memset(response,0,9);
    memset(extra, 0, MAXSIZEUDP);

    sscanf(request, "%s %s %s %s", command, userID, password, extra);

    if (
        strlen(extra) != 0 || strlen(userID) != 5 || strlen(password) != 8 ||
        !checkStringIsNumber(userID) || !checkStringIsAlphaNum(password)
    ){
        // Wrong format parameters
        strcpy(response, "ERR\n");
        logError(server.verbose, "Failed to login user because wrong format.");
        sendUDP(user,response);
        return;
    }
    else if (!UserExists(userID) || !checkUserPassword(userID,password)){
        // User doesn't exists or wrong password
        strcpy(response, "RLO NOK\n");
        logError(server.verbose, "Failed to unregister user because user doesnt exist or wrong password.");
        sendUDP(user,response);
        return;
    }

    if(createLoginFile(userID)){
        strcpy(response, "RLO OK\n"); 
        logLOG(server.verbose, userID);   
    }else{
        strcpy(response, "RLO NOK\n");
        logError(server.verbose, "Failed to unregister user because failed to create login file.");
    } 
  
    sendUDP(user, response);
}

/**
 * Process logout request.
 * @param user User data
 * @param server Server data
 * @param request Client input to be parsed
*/
void processOUT(userData user, serverData server, char* request){

    char command[4], userID[6], password[9],  extra[MAXSIZEUDP];
    char response[9];
    memset(response,0,9);
    memset(extra, 0, MAXSIZEUDP);

    sscanf(request, "%s %s %s %s", command, userID, password, extra);

    if (
        strlen(extra) != 0 || strlen(userID) != 5 || strlen(password) != 8 ||
        !checkStringIsNumber(userID) || !checkStringIsAlphaNum(password)
    ){
        // Wrong format parameters
        strcpy(response, "ERR\n");
        logError(server.verbose, "Failed to logout user because wrong format.");
        sendUDP(user,response);
        return;
    }
    else if (!UserExists(userID) || !checkUserPassword(userID,password)){
        // User doesn't exists or wrong password
        strcpy(response, "ROU NOK\n");
        logError(server.verbose, "Failed to login user because user doesnt exist or wrong password.");
        sendUDP(user,response);
        return;
    }

    if (DelLoginFile(userID)){
        strcpy(response, "ROU OK\n");
        logOUT(server.verbose, userID);
    } else{
        strcpy(response, "ROU NOK\n");
        logError(server.verbose, "Failed to login user because failed to delete login file.");
    }
    
    sendUDP(user, response);
}

/**
 * Process groups request.
 * @param user User data
 * @param server Server data
 * @param request Client input to be parsed
*/
void processGLS(userData user, serverData server, char* request){

    int numberGroups;
    Group groupsList[MAXGROUPS];

    char command[4], extra[MAXSIZEUDP];
    char response[EXTRAMAXSIZE]; // TODO BEST APPROACH ? SHOULDNT DO SPRINTF TO THE SAME
    char groupLine[34]; // 1 + 2 + 1 + 24 + 1 + 4
    memset(response,0,EXTRAMAXSIZE);
    memset(extra, 0, MAXSIZEUDP);

    sscanf(request, "%s %s", command, extra);

    if (strlen(extra) != 0){
        // Wrong size parameters
        strcpy(response, "ERR\n");
        logError(server.verbose, "Failed to get groups because wrong format.");
        sendUDP(user,response);
        return;
    }

    // Fill data structure with groups info
    numberGroups = ListGroupsDir(groupsList);
    if(numberGroups == -1){
        strcpy(response,"ERR\n");
        logError(server.verbose, "Failed to get groups because of server error.");
        sendUDP(user,response);
        return;
    }
    else if(numberGroups == 0){
        strcpy(response, "RGL 0\n");
        logGLS(server.verbose);
        sendUDP(user,response);
        return;
    }

    sprintf(response, "RGL %d", numberGroups);
    for(int i = 0; i < numberGroups; i++){
        sprintf(groupLine, " %s %s %s", groupsList[i].number, groupsList[i].name, groupsList[i].lastMsg);
        strcat(response, groupLine);
    }
    strcat(response, "\n");

    logGLS(server.verbose);

    sendUDP(user, response);
}

/**
 * Process subscribe request.
 * @param user User data
 * @param server Server data
 * @param request Client input to be parsed
*/
void processGSR(userData user, serverData server, char* request){

    int maxGroupID;

    char command[4], userID[6], groupID[3], groupName[25], extra[MAXSIZEUDP];
    char newGroupID[3];
    char response[13];
    memset(response,0,13);
    memset(extra, 0, MAXSIZEUDP);

    sscanf(request, "%s %s %s %s %s", command, userID, groupID, groupName, extra);

    if(strlen(groupID) == 1){
        sprintf(groupID, "%02d", atoi(groupID));
    }

    if (
        strlen(extra) != 0 || strlen(userID) != 5 || strlen(groupID) != 2 || strlen(groupName) > 24 || 
        !checkStringIsNumber(userID) ||  !checkStringIsNumber(groupID) || !checkStringIsGroupName(groupName)
    ){
        // Wrong size parameters
        strcpy(response, "ERR\n");
        logError(server.verbose, "Failed to subscribe because wrong format.");
        sendUDP(user, response);
        return;
    }
    if (!UserExists(userID) || !CheckUserLogin(userID)){
        // Invalid UID
        strcpy(response, "RGS E_USR\n"); // TODO LOG SOMETHING GOOD/BAD ?
        sendUDP(user, response);
        return;
    }
    // User want to create and subscribe to new group
    else if (strcmp(groupID, "00") == 0){

        maxGroupID = maxGroupNumber();
        if(maxGroupID >= MAXGROUPS){
            // Max number of groups, can't create more
            strcpy(response, "RGS E_FULL\n");
            sendUDP(user, response);
            return;
        }

        sprintf(newGroupID, "%02d", maxGroupID + 1);

        if(!CreateGroupDir(newGroupID)){
            strcpy(response,"ERR\n");
            sendUDP(user, response);
            return;
        }

        if(!CreateGroupFile(newGroupID,groupName)){
            strcpy(response,"ERR\n");
            sendUDP(user, response);
            return;
        }

        if(!SubscribeUser(userID, newGroupID)){
            // Failed to subscribe user to newly created group
            strcpy(response, "RGS NOK\n");
            sendUDP(user, response);
            return;
        }
        // Created and subscribed to new group
        sprintf(response, "RGS NEW %s\n", newGroupID);
        logGSR(server.verbose, userID,newGroupID);

        sendUDP(user, response);
    }  
    // User wants to subscribe to group 
    else{ 
        if(!GroupExists(groupID)){
            // Invalid GID
            strcpy(response, "RGS E_GRP\n");
            sendUDP(user, response);
            return;
        }
        if(!checkGroupName(groupID, groupName)){
            // Invalid Gname
            strcpy(response, "RGS E_GNAME\n");
            sendUDP(user, response);
            return;
        }
        if(!SubscribeUser(userID, groupID)){
            // Failed to subscribe user
            strcpy(response, "RGS NOK\n");
            sendUDP(user, response);
            return;
        }
        // Subscribed user 
        strcpy(response, "RGS OK\n");
        logGSR(server.verbose, userID,groupID);

        sendUDP(user, response);
    }
}

/**
 * Process unsubscribe request.
 * @param user User data
 * @param server Server data
 * @param request Client input to be parsed
*/
void processGUR(userData user, serverData server, char* request){

    char command[4], userID[6], groupID[3], extra[MAXSIZEUDP];
    char response[13];
    memset(response,0,13);
    memset(extra, 0, MAXSIZEUDP);

    sscanf(request, "%s %s %s %s", command, userID, groupID, extra);
    
    if (
        strlen(extra) != 0 || strlen(userID) != 5 || strlen(groupID) != 2 ||
        !checkStringIsNumber(userID) || !checkStringIsNumber(groupID)
    ){
        // Wrong size parameters
        strcpy(response, "ERR\n");
        sendUDP(user, response);
        return;
    }
    else if (!UserExists(userID) || !CheckUserLogin(userID)){
        // Invalid UID
        strcpy(response, "RGU E_USR\n");
        sendUDP(user, response);
        return;
    }
    else if(!GroupExists(groupID)){
        // Invalid GID
        strcpy(response, "RGU E_GRP\n");
        sendUDP(user, response);
        return;
    }

    // User is actually subscribed to group
    if(checkUserSubscribedToGroup(userID, groupID)){
        
        if(UnsubscribeUser(userID, groupID)){
            strcpy(response, "RGU OK\n");
            logGUR(server.verbose, userID,groupID);

            sendUDP(user, response);
            return;
        }
        else{
            strcpy(response, "RGU NOK\n");
            sendUDP(user, response);
            return;
        }
    }
    // In this case the user is not subscribed to the group
    // But it returns success nonetheless
    else {
        strcpy(response, "RGU OK\n");
        logGUR(server.verbose, userID,groupID);

        sendUDP(user, response);
        return;
    }
}

/**
 * Process my_groups request.
 * @param user User data
 * @param server Server data
 * @param request Client input to be parsed
*/
void processGLM(userData user, serverData server, char* request){

    Group groupsList[MAXGROUPS];
    int numberGroups, subscribedGroups;
    
    char command[4], userID[6], extra[MAXSIZEUDP];
    char response[EXTRAMAXSIZE];
    char groupLine[34];
    memset(response,0,EXTRAMAXSIZE);
    memset(extra, 0, MAXSIZEUDP);

    sscanf(request, "%s %s %s", command, userID, extra);

    if (strlen(extra) != 0 || strlen(userID) != 5 || !checkStringIsNumber(userID)){
        // Wrong size parameters
        strcpy(response, "ERR\n");
        sendUDP(user, response);
        return;
    }
    else if (!UserExists(userID) || !CheckUserLogin(userID)){
        // Invalid UID
        strcpy(response, "RGM E_USR\n");
        sendUDP(user, response);
        return;
    }

    // Fill groupsList data structure with groups info
    numberGroups = ListGroupsDir(groupsList);
    if(numberGroups == -1){
        strcpy(response,"ERR\n");
        sendUDP(user, response);
        return;
    }
    else if(numberGroups == 0){
        strcpy(response, "RGM 0\n");
        logULS(server.verbose, userID);
        sendUDP(user, response);
        return;
    }

    subscribedGroups = 0;
    for (int i = 0; i < numberGroups; i++){
        if(checkUserSubscribedToGroup(userID, groupsList[i].number)){
            subscribedGroups ++;
        }
    }
    
    sprintf(response, "RGM %d", subscribedGroups);
    for(int i = 0; i < numberGroups; i++){
        if(checkUserSubscribedToGroup(userID, groupsList[i].number)){
            sprintf(groupLine, " %s %s %s", groupsList[i].number, groupsList[i].name, groupsList[i].lastMsg);
            strcat(response, groupLine);
        }
    }
    strcat(response, "\n");
    logULS(server.verbose, userID);
    
    sendUDP(user, response);
    return; 
}