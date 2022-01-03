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