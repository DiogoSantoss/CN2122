#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "log.h"
#include "structs.h"
#include "common.h"

// Booleans
#define TRUE  1
#define FALSE 0

// Constants
#define MAXSIZE 274
#define EXTRAMAXSIZE 3169

/**
 * Parse register command.
 * @param[in] user User data
 * @param[in] input User input to be parsed
 * @param[out] message Formarted message to send to server
*/
char* parseRegister(userData* user, char* input){

    char* message;
    char command[MAXSIZE],UID[MAXSIZE],pass[MAXSIZE],extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s %s %s\n",command,UID,pass,extra);

    if((strlen(extra) != 0) || (strlen(input) != 19) || (strlen(UID) != 5) || (strlen(pass) != 8)){
        logError("Wrong size parameters.");
        return NULL;

    } else if(!checkStringIsNumber(UID) || !checkStringIsAlphaNum(pass)){
        logError("Forbidden character in parameters.");
        return NULL;
    }

    message = malloc(sizeof(char)*18);
    sprintf(message,"REG %s %s\n",UID,pass);

    return message;
}

/**
 * Parse unregister command.
 * @param[in] user User data
 * @param[in] input User input to be parsed
 * @param[out] message Formarted message to send to server
*/
char* parseUnregister(userData* user, char* input){

    char* message;
    char command[MAXSIZE],UID[MAXSIZE],pass[MAXSIZE],extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s %s %s\n",command,UID,pass,extra);

    if((strlen(extra) != 0) || ((strlen(input) != 19) && (strlen(input) != 26)) || (strlen(UID) != 5) || (strlen(pass) != 8)){
        logError("Wrong size parameters.");
        return NULL;

    } else if(!checkStringIsNumber(UID) || !checkStringIsAlphaNum(pass)){
        logError("Forbidden character in parameters.");
        return NULL;
    }

    message = malloc(sizeof(char)*18);
    sprintf(message,"UNR %s %s\n",UID,pass);

    return message;
} 

/**
 * Parse login command.
 * @param[in] user User data
 * @param[in] input User input to be parsed
 * @param[out] message Formarted message to send to server
*/
char* parseLogin(userData* user, char* input){

    char* message;
    char command[MAXSIZE],UID[MAXSIZE],pass[MAXSIZE],extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s %s %s\n",command,UID,pass,extra);

    if((strlen(extra) != 0) || (strlen(input) != 21) || (strlen(UID) != 5) || (strlen(pass) != 8)){
        logError("Wrong size parameters.");
        return NULL;

    } else if(!checkStringIsNumber(UID) || !checkStringIsAlphaNum(pass)){
        logError("Forbidden character in parameters.");
        return NULL;
    }

    if(strcmp(user->ID,"")){
        logError("A user is already logged in.");
        return NULL;

    } else {
        strcpy(user->ID,UID);
        strcpy(user->password,pass);
    }

    message = malloc(sizeof(char)*18);
    sprintf(message,"LOG %s %s\n",UID,pass);
    
    return message;
}

/**
 * Resets UID if server gives bad response.
 * @param[in] user User data
 * @param[in] response Server response
*/
void helperLogin(userData *user, char *response){
    if(!strcmp(response,"RLO NOK\n")){
        strcpy(user->ID,"");
    }
}

/**
 * Parse logout command.
 * @param[in] user User data
 * @param[in] input User input to be parsed
 * @param[out] message Formarted message to send to server
*/
char* parseLogout(userData* user, char* input){

    char* message;
    char command[MAXSIZE],extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s\n",command,extra);

    if((strlen(extra) != 0) || (strlen(input) != 7)){
        logError("Wrong size parameters.");
        return NULL;
    }

    if(!strcmp(user->ID,"")){
        logError("No user is logged in.");
        return NULL;
    }

    message = malloc(sizeof(char)*18);
    sprintf(message,"OUT %s %s\n",user->ID,user->password);

    return message;
}

/**
 * Resets UID and password if server gives good response.
 * @param[in] user User data
 * @param[in] response Server response
*/
void helperLogout(userData* user, char* response){
    if(!strcmp(response,"ROU OK\n")){
        strcpy(user->ID,"");
        strcpy(user->password,"");
    }
}

/**
 * Process showUID command.
 * @param[in] user User data
 * @param[in] input User input to be parsed
*/
void processShowUID(userData* user, char* input){

    char* message;
    char command[MAXSIZE],extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s\n",command,extra);

    if(strlen(extra) != 0){
        logError("Wrong size parameters.");
        return;
    }

    if(!strcmp(user->ID,"")){
        logError("No user is logged in.");
        return;
    }
    // todo should be login, also current group and current gid
    logSU(user->ID);
}

/**
 * Parse groups command.
 * @param[in] user User data
 * @param[in] input User input to be parsed
 * @param[out] message Formarted message to send to server
*/
char* parseGroups(userData* user, char* input){
    char* message;
    char command[MAXSIZE], extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s\n",command,extra);

    if((strlen(extra) != 0) || ((strlen(command) != 2) && (strlen(command) != 6))){
        logError("Wrong size parameters.");
        return NULL;
    }

    message = malloc(sizeof(char)*4);
    sprintf(message,"GLS\n");

    return message;
}

/**
 * Parse subscribe command.
 * @param[in] user User data
 * @param[in] input User input to be parsed
 * @param[out] message Formarted message to send to server
*/
char* parseSubscribe(userData* user, char* input){

    char* message;
    char command[MAXSIZE], GID[MAXSIZE], GName[MAXSIZE], extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s %s %s\n", command, GID, GName, extra);

    if((strlen(extra) != 0) || ((strlen(command) != 9) && (strlen(command) != 1)) || (strlen(GID) > 2) || (strlen(GName) > 25)){
        logError("Wrong size parameters.");
        return NULL;

    } else if(!checkStringIsNumber(GID) || !checkStringIsGroupName(GName)){
        logError("Forbidden character in parameters.");
        return NULL;
    }

    if(!strcmp(user->ID,"")){
        logError("No user is logged in.");
        return NULL;
    }

    message = malloc(sizeof(char)*37);
    sprintf(message,"GSR %s %02d %s\n", user->ID, atoi(GID), GName);

    return message;
}

/**
 * Parse unsubscribe command.
 * @param[in] user User data
 * @param[in] input User input to be parsed
 * @param[out] message Formarted message to send to server
*/
char* parseUnsubscribe(userData* user, char* input){

    char* message;
    char command[MAXSIZE], GID[MAXSIZE], extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s %s\n", command, GID, extra);

    if((strlen(extra) != 0) || ((strlen(command) != 11) && (strlen(command) != 1)) || (strlen(GID) > 2)){
        logError("Wrong size parameters.");
        return NULL;

    } else if(!checkStringIsNumber(GID)){
        logError("Forbidden character in parameters.");
        return NULL;
    }

    if(!strcmp(user->ID,"")){
        logError("No user is logged in.");
        return NULL;
    }

    message = malloc(sizeof(char)*37);
    sprintf(message,"GUR %s %02d\n", user->ID, atoi(GID));

    return message;
}

/**
 * Parse my_groups command.
 * @param[in] user User data
 * @param[in] input User input to be parsed
 * @param[out] message Formarted message to send to server
*/
char* parseMyGroups(userData* user, char* input){

    char* message;
    char command[MAXSIZE],extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s\n",command,extra);

    if((strlen(extra) != 0) || ((strlen(input) != 4) && (strlen(input) != 10))){
        logError("Wrong size parameters.");
        return NULL;
    }

    if(!strcmp(user->ID,"")){
        logError("No user is logged in.");
        return NULL;
    }

    message = malloc(sizeof(char)*10);
    sprintf(message,"GLM %s\n", user->ID);

    return message;
}

/**
 * Process select command.
 * @param[in] user User data
 * @param[in] input User input to be parsed
*/
void processSelect(userData* user, char* input){

    char* message;
    char command[MAXSIZE], GID[MAXSIZE], extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s %s\n", command, GID, extra);

    if((strlen(extra) != 0) || ((strlen(command) != 6) && (strlen(command) != 3)) || (strlen(GID) > 2)){
        logError("Wrong size parameters.");
        return;

    } else if(!checkStringIsNumber(GID)){
        logError("Forbidden character in parameters.");
        return;
    } else if(!strcmp(GID, "0") || !strcmp(GID, "00")){
        logError("Group 0 doesn't exist");
        return;
    }

    if(!strcmp(user->ID,"")){
        logError("No user is logged in.");
        return;
    }

    strcpy(user->groupID, GID);
    logSAG(user->groupID);
}

/**
 * Parse showGID command.
 * @param[in] user User data
 * @param[in] input User input to be parsed
*/
void processShowGID(userData* user, char* input){

    char* message;
    char command[MAXSIZE],extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s\n",command,extra);

    if(strlen(extra) != 0){
        logError("Wrong size parameters.");
        return;
    }

    if(!strcmp(user->ID,"")){
        logError("No user is logged in.");
        return;
    }

    if(!strcmp(user->groupID,"")){
        logError("No group is selected.");
        return;
    }
    
    logSG(user->groupID);
}

/**
 * Connect via UDP socket to server.
 * @param[in] server Data about the server
 * @param[in] fd File descriptor of UDP socket
 * @param[in] res Information about server 
*/
int connectUDP(serverData *server, int* fd, struct addrinfo** res){
    int errcode;
    struct addrinfo hints;

    *fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(*fd==-1){
        logError("Couldn't create UDP socket.");
        return FALSE;
    }

    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;

    errcode=getaddrinfo(server->ipAddress,server->port,&hints,res);
    if(errcode!=0){
        logError("Couldn't get server info.");
        return FALSE;
    }

    return TRUE;
}

/**
 * Send message via UDP socket to server.
 * @param[in] fd File descriptor of UDP socket
 * @param[in] res Information about server 
 * @param[in] message Message to be sent
 * @param[in] messageLen Message length
*/
int sendMessageUDP(int fd, struct addrinfo* res, char* message, int messageLen){
    int n;
    socklen_t addrlen;
    struct sockaddr_in addr;

    n = sendto(fd, message,messageLen,0,res->ai_addr,res->ai_addrlen);
    if(n==-1){
        logError("Couldn't send message via UDP socket");
        return FALSE;
    }   
    return TRUE;
}

/**
 * Receive message via UDP socket from server.
 * @param[in] fd File descriptor of UDP socket
 * @param[out] message Message from server
*/
char* receiveMessageUDP(int fd){
    int n;
    socklen_t addrlen;
    struct sockaddr_in addr;

    char* message = calloc(EXTRAMAXSIZE,sizeof(char));

    addrlen = sizeof(addr);
    n = recvfrom(fd,message,EXTRAMAXSIZE,0,(struct sockaddr*)&addr,&addrlen);
    if(n==-1){
        logError("Couldn't receive message via UDP socket");
        free(message);
        return NULL;
    } 
    return message;
}

/**
 * Generic function to proccess commands that access the server via UDP protocol.
 * This function receives the user input and a set of function specific for each
 * command.
 * @param[in] user User data
 * @param[in] server Server data
 * @param[in] input User input
 * @param[in] parser Function to parse the command
 * @param[in] logger Function to log the messages related to the command
 * @param[in] helper "Optional" function when processRequest needs to do additional tasks
 * 
*/
void processRequestUDP(
    userData *user, 
    serverData *server, 
    char* input, 
    char* (*parser)(userData*,char*), 
    void (*logger)(char*), 
    void (*helper)(userData*,char*)
    ){

    int fd;
    int msgSize;
    struct addrinfo *res;
    char *message, *response;

    message = (*parser)(user,input);
    if(message == NULL) return;

    if(!connectUDP(server,&fd,&res)) return;
    if(!sendMessageUDP(fd,res,message,strlen(message))) return;
    response = receiveMessageUDP(fd);
    if(response == NULL) return;

    if(helper != NULL){
        (*helper)(user,response);
    }

    (*logger)(response);

    free(message);
    free(response);
}