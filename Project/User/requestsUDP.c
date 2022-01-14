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

// Max size user can input
#define MAXINPUTSIZE 274
// Max size server can respond
// Corresponds to a groups response with all groups and max size names
// RGL N[ GID GName MID]* -> 3+1+2+99*(1+2+1+24+1+4)+1 = 3274
#define MAXRESPONSESIZE 3274

// Max size for each command
#define LONGINPUTREG 19
#define LONGINPUTUNR 26
#define LONGINPUTLOG 21
#define LONGINPUTOUT 7
#define LONGINPUTGLS 7
#define LONGINPUTGSR 38
#define LONGINPUTGUR 15
#define LONGINPUTGLM 10
#define LONGINPUTSID 8
#define LONGINPUTSEL 10

// Alternative max sixe
#define SHORTINPUTUNR 19
#define SHORTINPUTGLS 3
#define SHORTINPUTGSR 29
#define SHORTINPUTGUR 5
#define SHORTINPUTGLM 4
#define SHORTINPUTSID 3
#define SHORTINPUTSEL 7

/**
 * Parse register command.
 * @param user User data
 * @param input User input to be parsed
 * @return message to send to server
*/
char* parseRegister(userData* user, char* input){

    char* message;
    char command[LONGINPUTREG+1],userID[LONGINPUTREG+1],password[LONGINPUTREG+1],extra[LONGINPUTREG+1];
    memset(extra, 0, LONGINPUTREG+1);
    
    if((strlen(input) != LONGINPUTREG)){
        logError("Wrong size parameters.");
        return NULL;
    }

    sscanf(input,"%s %s %s %s\n",command,userID,password,extra);

    if((strlen(extra) != 0) || (strlen(userID) != 5) || (strlen(password) != 8)){
        logError("Wrong size parameters.");
        return NULL;

    } else if(!checkStringIsNumber(userID) || !checkStringIsAlphaNum(password)){
        logError("Forbidden character in parameters.");
        return NULL;
    }

    message = malloc(sizeof(char)*19);
    sprintf(message,"REG %s %s\n",userID,password);

    return message;
}

/**
 * Parse unregister command.
 * @param user User data
 * @param input User input to be parsed
 * @return message Formarted message to send to server
*/
char* parseUnregister(userData* user, char* input){

    char* message;
    char command[LONGINPUTUNR+1],userID[LONGINPUTUNR+1],password[LONGINPUTUNR+1],extra[LONGINPUTUNR+1];

    memset(extra, 0, LONGINPUTUNR+1);

    if(strlen(input) != LONGINPUTUNR && strlen(input) != SHORTINPUTUNR){
        logError("Wrong size parameters.");
        return NULL;
    }

    sscanf(input,"%s %s %s %s\n",command,userID,password,extra);

    if((strlen(extra) != 0) || (strlen(userID) != 5) || (strlen(password) != 8)){
        logError("Wrong size parameters.");
        return NULL;

    } else if(!checkStringIsNumber(userID) || !checkStringIsAlphaNum(password)){
        logError("Forbidden character in parameters.");
        return NULL;
    }

    message = malloc(sizeof(char)*19);
    sprintf(message,"UNR %s %s\n",userID,password);

    return message;
} 

/**
 * If logged in user was unregister then logout.
 * @param user User data
 * @param input Client response
 * @param response Server response
*/
void helperUnregister(userData* user, char* input, char* response){

    char command[MAXINPUTSIZE], userID[MAXINPUTSIZE], password[MAXINPUTSIZE];

    sscanf(input,"%s %s %s\n",command,userID,password);

    if(!strcmp(user->ID,userID) && !strcmp(response,"RUN OK\n")){
        strcpy(user->ID,"");
        strcpy(user->password,"");
    }
}

/**
 * Parse login command.
 * @param user User data
 * @param input User input to be parsed
 * @return message Formarted message to send to server
*/
char* parseLogin(userData* user, char* input){

    char* message;
    char command[LONGINPUTLOG+1], userID[LONGINPUTLOG+1], password[LONGINPUTLOG+1], extra[LONGINPUTLOG+1];

    memset(extra, 0, LONGINPUTLOG+1);
    
    if((strlen(input) != LONGINPUTLOG)){
        logError("Wrong size parameters.");
        return NULL;
    }

    sscanf(input,"%s %s %s %s\n",command,userID,password,extra);

    if((strlen(extra) != 0) || (strlen(userID) != 5) || (strlen(password) != 8)){
        logError("Wrong size parameters.");
        return NULL;

    } else if(!checkStringIsNumber(userID) || !checkStringIsAlphaNum(password)){
        logError("Forbidden character in parameters.");
        return NULL;
    }

    if(strcmp(user->ID,"")){
        logError("A user is already logged in.");
        return NULL;
    }

    message = malloc(sizeof(char)*19);
    sprintf(message,"LOG %s %s\n",userID,password);
    
    return message;
}

/**
 * Logins user.
 * @param user User data
 * @param input Client input
 * @param response Server response
*/
void helperLogin(userData* user, char* input, char* response){

    char command[MAXINPUTSIZE],userID[MAXINPUTSIZE],password[MAXINPUTSIZE];

    sscanf(input,"%s %s %s\n",command,userID,password);

    if(!strcmp(response,"RLO OK\n")){
        strcpy(user->ID,userID);
        strcpy(user->password,password);
    }
}

/**
 * Parse logout command.
 * @param user User data
 * @param input User input to be parsed
 * @return message Formarted message to send to server
*/
char* parseLogout(userData* user, char* input){

    char* message;
    char command[LONGINPUTOUT+1],extra[LONGINPUTOUT+1];

    memset(extra, 0, LONGINPUTOUT+1);

    if((strlen(input) != LONGINPUTOUT)){
        logError("Wrong size parameters.");
        return NULL;
    }
    
    sscanf(input,"%s %s\n",command,extra);

    if((strlen(extra) != 0)){
        logError("Wrong size parameters.");
        return NULL;
    }

    if(!strcmp(user->ID,"")){
        logError("No user is logged in.");
        return NULL;
    }

    message = malloc(sizeof(char)*19);
    sprintf(message,"OUT %s %s\n",user->ID,user->password);

    return message;
}

/**
 * Resets userID and password if server gives good response.
 * @param user User data
 * @param input Client input
 * @param response Server response
*/
void helperLogout(userData* user, char* input, char* response){
    if(!strcmp(response,"ROU OK\n")){
        strcpy(user->ID,"");
        strcpy(user->password,"");
    }
}

/**
 * Process showuserID command.
 * @param user User data
 * @param input User input to be parsed
*/
void processShowUID(userData* user, char* input){

    char command[LONGINPUTSID+1], extra[LONGINPUTSID+1];

    memset(extra, 0, LONGINPUTSID+1);

    if(strlen(input) != LONGINPUTSID && strlen(input) != SHORTINPUTSID){
        logError("Wrong size parameters.");
        return;
    }

    sscanf(input,"%s %s\n",command,extra);

    if(strlen(extra) != 0){
        logError("Wrong size parameters.");
        return;
    }

    if(!strcmp(user->ID,"")){
        logError("No user is logged in.");
        return;
    }

    logSU(user->ID);
}

/**
 * Parse groups command.
 * @param user User data
 * @param input User input to be parsed
 * @return message Formarted message to send to server
*/
char* parseGroups(userData* user, char* input){
    char* message;
    char command[LONGINPUTGLS+1], extra[LONGINPUTGLS+1];

    memset(extra, 0, LONGINPUTGLS+1);

    if(strlen(input) != LONGINPUTGLS && strlen(input) != SHORTINPUTGLS){
        logError("Wrong size parameters.");
        return NULL;
    }
    
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
 * @param user User data
 * @param input User input to be parsed
 * @return message Formarted message to send to server
*/
char* parseSubscribe(userData* user, char* input){

    char* message;
    char command[LONGINPUTGSR+1], groupID[LONGINPUTGSR+1], groupName[LONGINPUTGSR+1], extra[LONGINPUTGSR+1];

    memset(extra, 0, LONGINPUTGSR+1);
    
    if(strlen(input) > LONGINPUTGSR){
        logError("Wrong size parameters.");
        return NULL;
    }

    sscanf(input,"%s %s %s %s\n", command, groupID, groupName, extra);

    
    if(strlen(input) != (strlen(command)+strlen(groupID)+strlen(groupName)+3)){
        logError("Wrong size parameters.");
        return NULL;
    }
    else if((strlen(extra) != 0) || ((strlen(command) != 9) && (strlen(command) != 1)) || (strlen(groupID) > 2) || (strlen(groupName) > 25)){
        logError("Wrong size parameters.");
        return NULL;

    } 
    else if(!checkStringIsNumber(groupID) || !checkStringIsGroupName(groupName)){
        logError("Forbidden character in parameters.");
        return NULL;
    }

    if(!strcmp(user->ID,"")){
        logError("No user is logged in.");
        return NULL;
    }

    message = malloc(sizeof(char)*38);
    sprintf(message,"GSR %s %02d %s\n", user->ID, atoi(groupID), groupName);

    return message;
}

/**
 * Parse unsubscribe command.
 * @param user User data
 * @param input User input to be parsed
 * @return message Formarted message to send to server
*/
char* parseUnsubscribe(userData* user, char* input){

    char* message;
    char command[LONGINPUTGUR+1], groupID[LONGINPUTGUR+1], extra[LONGINPUTGUR+1];

    memset(extra, 0, LONGINPUTGUR+1);

    if(strlen(input) > LONGINPUTGUR){
        logError("Wrong size parameters.");
        return NULL;
    }

    sscanf(input,"%s %s %s\n", command, groupID, extra);

    if(strlen(input) != (strlen(command)+strlen(groupID)+2)){
        logError("Wrong size parameters.");
        return NULL;
    }
    else if((strlen(extra) != 0) || ((strlen(command) != 11) && (strlen(command) != 1)) || (strlen(groupID) > 2)){
        logError("Wrong size parameters.");
        return NULL;

    } 
    else if(!checkStringIsNumber(groupID)){
        logError("Forbidden character in parameters.");
        return NULL;
    }

    if(!strcmp(user->ID,"")){
        logError("No user is logged in.");
        return NULL;
    }

    message = malloc(sizeof(char)*38);
    sprintf(message,"GUR %s %02d\n", user->ID, atoi(groupID));

    return message;
}

/**
 * Parse my_groups command.
 * @param user User data
 * @param input User input to be parsed
 * @return message Formarted message to send to server
*/
char* parseMyGroups(userData* user, char* input){

    char* message;
    char command[LONGINPUTGLM+1], extra[LONGINPUTGLM+1];

    memset(extra, 0, LONGINPUTGLM+1);

    if((strlen(input) != LONGINPUTGLM) && (strlen(input) != SHORTINPUTGLM)){
        logError("Wrong size parameters.");
        return NULL;
    }

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
 * @param user User data
 * @param input User input to be parsed
*/
void processSelect(userData* user, char* input){

    char* message;
    char command[LONGINPUTSEL+1], groupID[LONGINPUTSEL+1], extra[LONGINPUTSEL+1];

    memset(extra, 0, LONGINPUTSEL+1);

    if((strlen(input) > LONGINPUTSEL)){
        logError("Wrong size parameters.");
        return;
    }

    sscanf(input,"%s %s %s\n", command, groupID, extra);

    if(strlen(input) != (strlen(command)+strlen(groupID)+2)){
        logError("Wrong size parameters.");
        return;
    }
    else if((strlen(extra) != 0) || ((strlen(command) != 6) && (strlen(command) != 3)) || (strlen(groupID) > 2)){
        logError("Wrong size parameters.");
        return;

    } 
    else if(!checkStringIsNumber(groupID)){
        logError("Forbidden character in parameters.");
        return;
    } 
    else if(!strcmp(groupID, "0") || !strcmp(groupID, "00")){
        logError("Group 0 doesn't exist");
        return;
    }

    if(!strcmp(user->ID,"")){
        logError("No user is logged in.");
        return;
    }

    sprintf(user->groupID,"%02d",atoi(groupID));
    logSAG(user->groupID);
}

/**
 * Parse showgroupID command.
 * @param user User data
 * @param input User input to be parsed
*/
void processShowGID(userData* user, char* input){

    char* message;
    char command[LONGINPUTSID+1], extra[LONGINPUTSID+1];

    memset(extra, 0, LONGINPUTSID+1);

    if((strlen(input) != LONGINPUTSID) && (strlen(input) != SHORTINPUTSID)){
        logError("Wrong size parameters.");
        return;
    }

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
 * @param server Data about the server
 * @param fd File descriptor of UDP socket
 * @param res Information about server 
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
 * @param fd File descriptor of UDP socket
 * @param res Information about server 
 * @param message Message to be sent
 * @param messageLen Message length
 * @return 1 for success or 0 for errors
*/
int sendMessageUDP(int fd, struct addrinfo* res, char* message, int messageLen){
    int n;
    socklen_t addrlen;
    struct sockaddr_in addr;

    n = sendto(fd, message, messageLen, 0, res->ai_addr, res->ai_addrlen);
    if(n==-1){
        logError("Couldn't send message via UDP socket");
        return FALSE;
    }   
    return TRUE;
}

int TimerON(int sd){

    struct timeval tmout;
    memset((char *)&tmout,0,sizeof(tmout)); /* clear time structure */
    tmout.tv_sec=10; /* Wait for 15 sec for a reply from server. */
    return(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tmout,sizeof(struct timeval)));
}

int TimerOFF(int sd){

    struct timeval tmout;
    memset((char *)&tmout,0,sizeof(tmout)); /* clear time structure */
    return(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tmout,sizeof(struct timeval)));
}

/**
 * Receive response via UDP socket from server.
 * @param fd File descriptor of UDP socket
 * @return 1 for success or 0 for errors
*/
int receiveMessageUDP(int fd, char* response){
    int n;
    socklen_t addrlen;
    struct sockaddr_in addr;

    addrlen = sizeof(addr);
    if(TimerON(fd) == -1){
        logError("Error setting timeout for UDP socket.");
        TimerOFF(fd);
        return FALSE;
    }

    n = recvfrom(fd,response,MAXRESPONSESIZE,0,(struct sockaddr*)&addr,&addrlen);
    if(n==-1){
        logError("Couldn't receive response via UDP socket.");
        if(TimerOFF(fd) == -1){
            logError("Error setting timeout for UDP socket.");
            return FALSE;
        }
        return FALSE;
    } 
    if(TimerOFF(fd) == -1){
        logError("Error setting timeout for UDP socket.");
        return FALSE;
    }
    return TRUE;
}

/**
 * Generic function to proccess commands that access the server via UDP protocol.
 * This function receives the user input and a set of function specific for each
 * command.
 * @param user User data
 * @param server Server data
 * @param input User input
 * @param parser Function to parse the command
 * @param logger Function to log the messages related to the command
 * @param helper "Optional" function when processRequest needs to do additional tasks
*/
void processRequestUDP(
    userData *user, 
    serverData *server, 
    char* input, 
    char* (*parser)(userData*,char*), 
    void (*logger)(char*), 
    void (*helper)(userData*,char*,char*)
    ){

    int msgSize, attempts, n;
    char *message; // will get calloc'd inside each parser and should be free'd
    char response[MAXRESPONSESIZE];
    memset(response, 0, MAXRESPONSESIZE);

    message = (*parser)(user,input);
    if(message == NULL) return;

    if(!connectUDP(server, &(user->fd), &(user->res)))
        return;

    if(!sendMessageUDP(user->fd, user->res, message, strlen(message)))
        return;

    n = receiveMessageUDP(user->fd, response);

    attempts = 1;
    // Resend message in case of failure/lost packages
    while(n == FALSE && attempts < 3){
        logError("Trying to resend...");
        if(!sendMessageUDP(user->fd, user->res, message, strlen(message))){
            free(message);
            return;
        } 
        n = receiveMessageUDP(user->fd, response);
        attempts++;
    }
    if(response == NULL && attempts == 3){
        logError("Stopped trying to send after 3 attemps.");
        free(message);
        return;
    }

    if(helper != NULL){
        (*helper)(user,input,response);
    }

    (*logger)(response);

    free(message);

}