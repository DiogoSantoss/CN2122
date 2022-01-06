#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>

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
 * Checks if reads end of line (\n)
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
 * Skips a space when reading a file/socket
 * @param[in] fd File descriptor
 * @param[out] TRUE or FALSE depending if it was a space or not
*/
int skipSpace(int fd){

    char space[1];

    if(receiveNSizeTCP(fd, space, 1) == -1) return FALSE;
    
    if(space[0] != ' '){
        return FALSE;
    }
    return TRUE;
}

/**
 * Reads a word of max size maxRead from fd
 * @param[in] fd File descriptor
 * @param[in] buffer array to read word to
 * @param[in] maxRead number of bytes to read
 * @param[out] TRUE or FALSE depending if word was correctly read
*/
int readWord(int fd, char* buffer, int maxRead){

    char readChar[1];
    memset(buffer, 0, maxRead + 1);

    for (int i = 0; i < maxRead; i++){
        // Reads one char
        if(receiveNSizeTCP(fd, readChar, 1) == -1) return FALSE;
        // If space is found in buffer boundaries, return TRUE
        if (readChar[0] == ' ') return TRUE;
        // Save char to buffer
        buffer[i] = readChar[0];
    }
    // Read max size, next should be space if not, error
    return skipSpace(fd);
}

/**
 * Process ulist request.
 * @param[in] user User data
 * @param[in] server User data
 * @param[in] fd File descriptor
*/
void processULS(userData user, serverData server, int fd){

    int read;
    char buffer[40];
    char GroupID[3];
    int numberUsersSub;
    char* message = calloc(9, sizeof(char));

    memset(GroupID, 0, 3);
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

    // Iterate over GROUPS/GID directory and send subscribed user id
    DIR *d;
    FILE *fp;
    char path[10];
    char GroupName[25];
    struct dirent *dir;

    memset(GroupName, 0, 25);
    getGroupName(GroupID,GroupName);
    sprintf(path, "GROUPS/%s", GroupID);

    d = opendir(path);
    if (d){

        sprintf(buffer,"RUL OK %s ",GroupName);
        sendTCP(fd,buffer,strlen(buffer));

        while ((dir = readdir(d))){   

            if(strlen(dir->d_name) == 9){

                memset(buffer, 0, 40);
                strncpy(buffer, dir->d_name, 5);
                sendTCP(fd,buffer,strlen(buffer));
                
                strcpy(buffer, " ");
                sendTCP(fd,buffer,strlen(buffer));
            }
        }

        strcpy(buffer,"\n");
        sendTCP(fd,buffer,strlen(buffer));
    }
    else{
        strcpy(buffer,"ERR\n");
        sendTCP(fd,buffer,strlen(buffer));
    }
}

/**
 * Process post request.
 * @param[in] user User data
 * @param[in] server User data
 * @param[in] fd File descriptor
*/
void processPST(userData user, serverData server, int fd){

    //INCOMING
    //PST UID GID Tsize text [Fname Fsize data]
    //OUTGOING
    //RPT status

    //UID GID Tsize text [Fname Fsize data]

    // readUntilSpace()
    // UID , GID , Tsize , 


    char UserID[6];
    char GroupID[3];
    char Tsize[4];
    char Fsize[11];
    char Text[241];
    char fileName[25];

    char response[10];

    char buffer[512];
    char singleChar[1];
    FILE *fptr;
    char filePath[44];

    int messageID, sent, fileSize, actuallyRead;

    memset(Text,0,241);
    memset(fileName,0,25);

    // Reads User ID
    if(!readWord(fd,UserID,5) || strlen(UserID) != 5 || !checkStringIsNumber(UserID)){
        strcpy(response,"ERR\n");
        sendTCP(fd,response,strlen(response));
        return;
    }

    // Reads Group ID
    if(!readWord(fd,GroupID,2) || strlen(GroupID) != 2 || !checkStringIsNumber(GroupID)){
        strcpy(response,"ERR\n");
        sendTCP(fd,response,strlen(response));
        return;
    }

    if(!UserExists(UserID) || !GroupExists(GroupID)){
        strcpy(response,"RPT NOK\n");
        sendTCP(fd,response,strlen(response));
        return;
    }

    // Read text size
    if(!readWord(fd,Tsize,3) || strlen(Tsize) > 3 || !checkStringIsNumber(Tsize) || atoi(Tsize) > 240){
        strcpy(response,"ERR\n");
        sendTCP(fd,response,strlen(response));
        return;
    }

    // Read text
    if(!receiveNSizeTCP(fd,Text,atoi(Tsize))){
        strcpy(response,"RPT NOK\n");
        sendTCP(fd,response,strlen(response));
        return;
    }

    // Create Message dir, author file and text file
    messageID = CreateMessageDir(UserID,GroupID,Text);
    if(messageID == -1){
        strcpy(response,"RPT NOK\n");
        sendTCP(fd,response,strlen(response));
        return;
    }

    if(receiveNSizeTCP(fd, singleChar, 1) == -1){
        strcpy(response,"RPT NOK\n");
        sendTCP(fd,response,strlen(response));
        return;
    } 
    
    // Check what to do after reading text
    if(singleChar[0] == '\n'){
        sprintf(response,"RPT %04d\n",messageID);
        sendTCP(fd,response,strlen(response));
        return;

    }else if(singleChar[0] != ' '){
        strcpy(response,"ERR\n");
        sendTCP(fd,response,strlen(response));
        return;
    }

    // Read filename
    if(!readWord(fd,fileName,24) || !checkStringIsAlphaNum(fileName)){
        strcpy(response,"ERR\n");
        sendTCP(fd,response,strlen(response));
        return;
    }

    // Read file size
    if(!readWord(fd,Fsize,24) || !checkStringIsNumber(Fsize)){
        strcpy(response,"ERR\n");
        sendTCP(fd,response,strlen(response));
        return;
    }

    // Create file
    sprintf(filePath,"GROUPS/%s/MSG/%04d/%s",GroupID,messageID,fileName);
    if(!(fptr = fopen(filePath, "wb"))){
        strcpy(response,"RPT NOK\n");
        sendTCP(fd,response,strlen(response));
        return;
    }

    sent = 0;
    actuallyRead = 0;
    fileSize = atoi(Fsize);

    while(sent < fileSize){

        memset(buffer, 0, 512);
        // Read from socket
        actuallyRead = receiveNSizeTCP(fd,buffer,512);
        // Write to file
        fwrite(buffer, sizeof(char), actuallyRead, fptr);
        sent += actuallyRead;
    }

    if(!checkEndLine(fd)){
        strcpy(response,"ERR\n");
        sendTCP(fd,response,strlen(response));
        return;

    } else {
        sprintf(response,"RPT %04d\n",messageID);
        sendTCP(fd,response,strlen(response));
        return;
    }
}