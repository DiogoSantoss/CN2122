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
#define EXTRAMAXSIZE 3268

char* parseUlist(userData* user, char* input){
    // TODO: Need to check if the group is selected before doing ulist
    // For now, you can do ulist without the group being selected
    char* message;
    char command[MAXSIZE], extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s\n",command, extra);

    if (!strcmp(user->groupID,"")){
        logError("No group is selected");
        return NULL;
    }
    else if((strlen(extra) != 0) || (strlen(command) != 5)){
        logError("Wrong size parameters.");
        return NULL;
    }

    message = malloc(sizeof(char)*7);
    sprintf(message, "ULS %02d\n", atoi(user->groupID));

    return message;
}

/**
 * Connect via TCP socket to server.
 * @param[in] fd File descriptor of UDP socket
 * @param[in] res Information about server 
*/
void connectTCP(serverData *server, int* fd, struct addrinfo* res){
    int errcode,n;
    struct addrinfo hints;

    *fd = socket(AF_INET, SOCK_STREAM, 0);
    if(*fd==-1){
        logError("Couldn't create TCP socket.");
        exit(1);
    }

    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_STREAM;

    errcode=getaddrinfo((*server).ipAddress,(*server).port,&hints,&res) ;
    if(errcode!=0){
        logError("Couldn't get server info.");
        exit(1);
    }

    n = connect(*fd,res->ai_addr,res->ai_addrlen);
    if(n==-1){
        logError("Couldn't connect to server.");
        exit(1);
    }
}

/**
 * Send message via TCP socket to server.
 * @param[in] fd File descriptor of TCP socket
 * @param[in] message Message to be sent
 * @param[in] messageLen Message length
*/
void sendMessageTCP(int fd, char* message, int messageLen){
    int nLeft = messageLen;
    int nWritten;
    char* ptr;

    ptr = message;

    while (nLeft > 0){
        nWritten = write(fd, ptr, nLeft);
        if(nWritten <= 0){
            logError("Couldn't send message via TCP socket");
            exit(1);
        }
        nLeft -= nWritten;
        ptr += nWritten;
    }
}

/**
 * Receive message via UDP socket from server.
 * @param[in] fd File descriptor of UDP socket
 * @param[out] message Message from server
*/
char* receiveMessageTCP(int fd){
    int nRead;
    char* message = calloc(EXTRAMAXSIZE,sizeof(char));
    char* ptr;

    ptr = message;

    while (nRead != 0){
        nRead = read(fd, ptr, EXTRAMAXSIZE);
        if(nRead == -1){
            logError("Couldn't receive message via TCP socket");
            exit(1);
        }
        ptr += nRead;
    }
     
    return message;
}

/**
 * Generic function to proccess commands that access the server via TCP protocol.
 * This function receives the user input and a set of function specific for each
 * command.
 * @param[in] input User input
 * @param[in] parser Function to parse the command
 * @param[in] logger Function to log the messages related to the command
 * @param[in] helper "Optional" function when processRequest needs to do additional tasks
 * 
*/
void processRequestTCP(
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

    connectTCP(server,&fd,res);
    printf("%s\n", message);
    sendMessageTCP(fd,message,strlen(message));
    response = receiveMessageTCP(fd);

    if(helper != NULL){
        (*helper)(user,response);
    }

    (*logger)(response);

    free(message);
    free(response);
}