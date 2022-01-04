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

/**
 * Send message via TCP socket to user.
 * @param[in] fd File descriptor of TCP socket
 * @param[in] message Message to be sent
 * @param[in] messageLen Message length
*/
int sendTCP(int fd, char* message, int messageLen){
    int nLeft = messageLen;
    int nWritten;
    char* ptr;

    ptr = message;

    while (nLeft > 0){
        nWritten = write(fd, ptr, nLeft);
        if(nWritten <= 0){
            logError("Couldn't send message via TCP socket");
            return FALSE;
        }
        nLeft -= nWritten;
        ptr += nWritten;
    }

    return TRUE;
}

/**
 * Receive messageSize message via TCP socket from user.
 * @param[in] fd File descriptor of UDP socket
 * @param[in] buffer Buffer to be filled with message from the user
 * @param[in] messageSize Size of the desired message
 * @param[out] number of bytes read
*/
int receiveNSizeTCP(int fd, char* buffer, int messageSize){
    
    char* ptr = buffer;
    int sum = 0;
    int nRead = 0;

    while (messageSize > sum){
        nRead = read(fd, ptr, messageSize);
        if (nRead == -1){
            logError("Couldn't receive message via TCP socket.");
            return -1;
        }
        if (nRead == 0){
            break;
        }
        ptr += nRead;
        sum += nRead;
        messageSize -= nRead;
    }
    return sum;
}

/**
 * Skips a space when reading a file/socket
 * @param[in] fd File descriptor
 * @param[out] TRUE or FALSE depending if it was a space or not
*/
int checkEndLine(int fd){

    char endLine[1];

    if(receiveNSizeTCP(fd, endLine, 1) == -1) return FALSE;
    
    if(endLine[0] != '\n'){
        return FALSE;
    }
    return TRUE;
}

/**
 * Process ulist request.
 * @param[in] user User data
 * @param[in] request Client input to be parsed
 * @param[out] message Formarted message to respond to client
*/
void processULS(userData user, serverData server, int fd){

    int read;
    char buffer[40];
    char GroupID[3];
    int numberUsersSub;
    //char** usersSubscribed;
    memset(GroupID, 0, 3);

    char* message = calloc(9, sizeof(char));

    read = receiveNSizeTCP(fd, GroupID, 2);
    if (read != 2){
        strcpy(buffer,"ERR\n");
        sendTCP(fd,buffer,strlen(buffer));
        return;
    }
    else if(!checkEndLine(fd)){
        strcpy(buffer,"ERR\n");
        sendTCP(fd,buffer,strlen(buffer));
        return;
    }
    else if (!checkStringIsNumber(GroupID) || !GroupExists(GroupID)){
        strcpy(buffer,"RUL NOK\n");
        sendTCP(fd,buffer,strlen(buffer));
        return;
    }

    numberUsersSub = NumberUsersSub(GroupID);
    if (numberUsersSub == -1){
        strcpy(buffer,"ERR\n");
        sendTCP(fd,buffer,strlen(buffer));
        return;
    }
    
    //TODO - this
    char usersSubscribed[numberUsersSub][5];
    
    if(ListSubscribedUsers(usersSubscribed, GroupID) == -1){
        strcpy(buffer,"ERR\n");
        sendTCP(fd,buffer,strlen(buffer));
        return;
    }

    sprintf(buffer, "RUL OK %s ", GroupID);
    sendTCP(fd,buffer,strlen(buffer));

    for(int i = 0; i<numberUsersSub; i++){
        sprintf(buffer, "%s ", usersSubscribed[i]);
        sendTCP(fd,buffer,strlen(buffer));
    }

    strcpy(buffer,"\n");
    sendTCP(fd,buffer,strlen(buffer));
}

