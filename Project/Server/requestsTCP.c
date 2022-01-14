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

#define MAXGROUPS 99

#define TEXTSIZE 240
#define FILENAMESIZE 24
#define FILEBUFFERSIZE 512
#define FILESIZEMAXDIGITS 10

/**
 * Send message via TCP socket to user.
 * @param fd File descriptor of TCP socket
 * @param message Message to be sent
 * @param messageLen Message length
 * @return 1 for success or 0 for errors
*/
int sendTCP(int fd, char* message, int messageLen){

    char* ptr;
    int nLeft, nWritten;

    ptr = message;
    nLeft = messageLen;
    while (nLeft > 0){
        nWritten = write(fd, ptr, nLeft);
        if(nWritten <= 0){
            logError(TRUE, "Couldn't send message via TCP socket");
            return FALSE;
        }
        nLeft -= nWritten;
        ptr += nWritten;
    }

    return TRUE;
}

/**
 * Receive messageSize message via TCP socket from user.
 * @param fd File descriptor of UDP socket
 * @param buffer Buffer to be filled with message from the user
 * @param messageSize Size of the desired message
 * @return number of bytes read
*/
int receiveNSizeTCP(int fd, char* buffer, int messageSize){
    
    char* ptr;
    int nReceived, nRead;
    
    ptr = buffer;
    nReceived = 0;
    while (messageSize > 0){
        nRead = read(fd, ptr, messageSize);
        if (nRead == -1){
            logError(TRUE, "Couldn't receive message via TCP socket.");
            return -1;
        }
        else if (nRead == 0){
            break;
        }
        ptr += nRead;
        nReceived += nRead;
        messageSize -= nRead;
    }
    return nReceived;
}

/** 
 * Checks if reads end of line (\n)
 * @param fd File descriptor
 * @return 1 for success or 0 for errors
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
 * @param fd File descriptor
 * @return 1 for success or 0 for errors
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
 * @param fd File descriptor
 * @param buffer array to read word to
 * @param maxRead number of bytes to read
 * @return 1 for success or 0 for errors
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

void requestErrorTCP(userData user, serverData server, int fd){
    char buffer[5];
    strcpy(buffer,"ERR\n");
    sendTCP(fd,buffer,strlen(buffer));
}

/**
 * Process ulist request.
 * @param user User data
 * @param server User data
 * @param fd File descriptor
*/
void processULS(userData user, serverData server, int fd){

    DIR *d;
    FILE *fp;
    struct dirent *dir;

    int read, numberUsersSub;

    char buffer[40];
    char groupID[3];
    char path[10];
    char groupName[25];

    memset(groupID,0,3);
    memset(buffer,0,40);
    memset(groupName, 0, 25);

    read = receiveNSizeTCP(fd, groupID, 2);
    
    if (read != 2){
        strcpy(buffer,"ERR\n");
        logError(server.verbose, "Wrong format.");
        sendTCP(fd,buffer,strlen(buffer));
        return;
    }
    else if(!checkEndLine(fd)){
        strcpy(buffer,"ERR\n");
        logError(server.verbose, "Wrong format.");
        sendTCP(fd,buffer,strlen(buffer));
        return;
    }
    else if (!checkStringIsNumber(groupID)){
        strcpy(buffer,"ERR\n");
        logError(server.verbose, "Wrong format.");
        sendTCP(fd,buffer,strlen(buffer));
        return;
    }
    else if(!GroupExists(groupID)){
        strcpy(buffer,"RUL NOK\n");
        logError(server.verbose, "Group doesn't exist.");
        sendTCP(fd,buffer,strlen(buffer));
        return;
    }

    // Iterate over GROUPS/GID directory and send subscribed user id

    getGroupName(groupID,groupName);
    sprintf(path, "GROUPS/%s", groupID);

    d = opendir(path);
    if (d){

        sprintf(buffer,"RUL OK %s",groupName);
        sendTCP(fd,buffer,strlen(buffer));

        while ((dir = readdir(d))){   

            if(strlen(dir->d_name) == 9){

                strcpy(buffer, " ");
                sendTCP(fd,buffer,strlen(buffer));

                memset(buffer, 0, 40);
                strncpy(buffer, dir->d_name, 5);
                sendTCP(fd,buffer,strlen(buffer)); 
            }
        }

        strcpy(buffer,"\n");
        logULS(server.verbose, groupID);

        sendTCP(fd,buffer,strlen(buffer));
    }
    else{
        strcpy(buffer,"ERR\n");
        logError(server.verbose, "Failed to open GROUPS directory.");
        sendTCP(fd,buffer,strlen(buffer));
    }
    closedir(d);
}

/**
 * Process post request.
 * @param user User data
 * @param server User data
 * @param fd File descriptor
*/
void processPST(userData user, serverData server, int fd){

    FILE *fptr;
    int messageID, sent, fileSize, actuallyRead;

    char userID[6], groupID[3];
    char textSizeDigits[4], fileSizeDigits[FILESIZEMAXDIGITS+1];
    char text[TEXTSIZE+1];
    char fileName[FILENAMESIZE+1];

    char response[10];

    char buffer[FILEBUFFERSIZE];
    char singleChar[1];
    char filePath[44];

    memset(text,0,241);
    memset(fileName,0,25);

    // Reads User ID
    if(!readWord(fd,userID,5) || strlen(userID) != 5 || !checkStringIsNumber(userID)){
        strcpy(response,"ERR\n");
        logError(server.verbose, "Failed to post because of wrong format.");
        sendTCP(fd,response,strlen(response));
        return;
    }

    // Reads Group ID
    if(!readWord(fd,groupID,2) || strlen(groupID) != 2 || !checkStringIsNumber(groupID)){
        strcpy(response,"ERR\n");
        logError(server.verbose, "Failed to post because of wrong format.");
        sendTCP(fd,response,strlen(response));
        return;
    }

    if(!UserExists(userID) || !CheckUserLogin(userID) || !GroupExists(groupID)){
        strcpy(response,"RPT NOK\n");
        logError(server.verbose, "Failed to post because user/group doesnt exists or not logged in.");
        sendTCP(fd,response,strlen(response));
        return;
    }

    // Read text size
    if(!readWord(fd,textSizeDigits,3) || strlen(textSizeDigits) > 3 || !checkStringIsNumber(textSizeDigits) || atoi(textSizeDigits) > 240){
        strcpy(response,"ERR\n");
        logError(server.verbose, "Failed to post because of wrong format.");
        sendTCP(fd,response,strlen(response));
        return;
    }

    // Read text
    if(receiveNSizeTCP(fd,text,atoi(textSizeDigits)) == -1){
        strcpy(response,"RPT NOK\n");
        sendTCP(fd,response,strlen(response));
        return;
    }

    // Create Message dir, author file and text file
    messageID = CreateMessageDir(userID,groupID,text);
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
    
    // Checks if needs to read file
    if(singleChar[0] == '\n'){
        sprintf(response,"RPT %04d\n",messageID);
        logPST(server.verbose, userID, groupID, atoi(textSizeDigits), 0, NULL);
        sendTCP(fd,response,strlen(response));
        return;

    }else if(singleChar[0] != ' '){
        strcpy(response,"ERR\n");
        logError(server.verbose, "Failed to post because of wrong format.");
        sendTCP(fd,response,strlen(response));
        return;
    }

    // Read filename
    if(!readWord(fd,fileName,24)){
        strcpy(response,"ERR\n");
        sendTCP(fd,response,strlen(response));
        return;
    }

    // Read file size
    if(!readWord(fd,fileSizeDigits,10) || !checkStringIsNumber(fileSizeDigits)){
        strcpy(response,"ERR\n");
        logError(server.verbose, "Failed to post because of wrong format.");
        sendTCP(fd,response,strlen(response));
        return;
    }
    
    // Create file
    sprintf(filePath,"GROUPS/%s/MSG/%04d/%s",groupID,messageID,fileName);
    if(!(fptr = fopen(filePath, "w"))){
        strcpy(response,"RPT NOK\n");
        sendTCP(fd,response,strlen(response));
        return;
    }

    sent = 0;
    actuallyRead = 0;
    fileSize = atoi(fileSizeDigits);

    while(sent < fileSize){

        memset(buffer, 0, 512);
        // Read from socket
        if(fileSize - sent > 512){
            actuallyRead = receiveNSizeTCP(fd,buffer,512);
        }else {
            actuallyRead = receiveNSizeTCP(fd,buffer,fileSize - sent);
        }
        // Write to file
        if(fwrite(buffer, sizeof(char), actuallyRead, fptr) < actuallyRead){
            fclose(fptr);
            strcpy(response,"RPT NOK\n");
            sendTCP(fd,response,strlen(response));
            return;
        }
        sent += actuallyRead;
    }
    fclose(fptr);

    if(!checkEndLine(fd)){
        strcpy(response,"ERR\n");
        logError(server.verbose, "Failed to post because of wrong format.");
        sendTCP(fd,response,strlen(response));
        return;

    } else {
        sprintf(response,"RPT %04d\n",messageID);
        logPST(server.verbose, userID, groupID, atoi(textSizeDigits), fileSize, fileName);
        sendTCP(fd,response,strlen(response));
    }
}

/**
 * Process retrieve request.
 * @param user User data
 * @param server User data
 * @param fd File descriptor
*/
void processRTV(userData user, serverData server, int fd){

    FILE *fptr;
    int textSize, fileSize;
    int lastMessageID, messagesToRTV, firstMessageToRTV, lastMessageToRTV;

    char userID[6], groupID[3];
    char messageID[5];

    char response[10];

    char path[51];
    char fileName[25];
    char authorID[6];
    char text[241];
    char buffer[512];

    

    memset(messageID, 0, 5);

    // Reads User ID
    if(!readWord(fd,userID,5) || strlen(userID) != 5 || !checkStringIsNumber(userID)){
        strcpy(response,"ERR\n");
        logError(server.verbose, "Failed to post because of wrong format.");
        sendTCP(fd,response,strlen(response));
        return;
    }

    // Reads Group ID
    if(!readWord(fd,groupID,2) || strlen(groupID) != 2 || !checkStringIsNumber(groupID)){
        strcpy(response,"ERR\n");
        logError(server.verbose, "Failed to post because of wrong format.");
        sendTCP(fd,response,strlen(response));
        return;
    }

    if(!UserExists(userID) || !CheckUserLogin(userID) || !GroupExists(groupID)){
        strcpy(response,"RRT NOK\n");
        logError(server.verbose, "Failed to post because user/group doesnt exists or not logged in.");
        sendTCP(fd,response,strlen(response));
        return;
    }

    // Reads Message ID
    if(receiveNSizeTCP(fd,messageID,4) != 4 || strlen(messageID) != 4 || !checkStringIsNumber(messageID)){
        strcpy(response,"ERR\n");
        logError(server.verbose, "Failed to post because of wrong format.");
        sendTCP(fd,response,strlen(response));
        return;
    }

    if(!checkEndLine(fd)){
        strcpy(response,"ERR\n");
        logError(server.verbose, "Failed to post because of wrong format.");
        sendTCP(fd,response,strlen(response));
        return;
    }

    firstMessageToRTV = atoi(messageID);

    if(firstMessageToRTV == 0){
        strcpy(response,"RRT NOK\n");
        sendTCP(fd,response,strlen(response));
        return;
    }

    lastMessageID = GroupLastMessage(groupID);
    if(lastMessageID == -1){
        strcpy(response,"RRT NOK\n");
        sendTCP(fd,response,strlen(response));
        return;
    }
    else if(lastMessageID == 0 || lastMessageID < firstMessageToRTV){
        strcpy(response,"RRT EOF\n");
        logRTV(server.verbose, userID, groupID, 0);
        sendTCP(fd,response,strlen(response));
        return;
    }   

    if (lastMessageID - firstMessageToRTV + 1 >= 20)
        messagesToRTV = 20;
    else
        messagesToRTV = lastMessageID - firstMessageToRTV + 1;

    // Send status and N
    sprintf(response,"RRT OK %d",messagesToRTV);
    sendTCP(fd,response,strlen(response));
    
    lastMessageToRTV = firstMessageToRTV + messagesToRTV - 1;

    logRTV(server.verbose, userID, groupID, messagesToRTV);

    for(int currentMessageID = firstMessageToRTV; currentMessageID <= lastMessageToRTV; currentMessageID++){

        memset(authorID, 0, 6);
        memset(text, 0, 241);

        sprintf(path,"GROUPS/%s/MSG/%04d",groupID,currentMessageID);

        // Read author of message
        sprintf(path,"GROUPS/%s/MSG/%04d/A U T H O R.txt",groupID,currentMessageID);
        if(!(fptr = fopen(path, "r"))){
            logError(server.verbose, "Failed to open author file.");
            return;
        }
        if(fread(authorID, sizeof(char), 5, fptr) < 5){
            fclose(fptr);
            return;
        }
        fclose(fptr);

        // Open text file
        sprintf(path,"GROUPS/%s/MSG/%04d/T E X T.txt",groupID,currentMessageID);
        if(!(fptr = fopen(path, "r"))){
            logError(server.verbose, "Failed to open text file.");
            return;
        }

        // Get text size
        fseek(fptr,0L,SEEK_END);
        textSize = ftell(fptr);
        rewind(fptr);

        // Read text
        if(fread(text, sizeof(char), textSize, fptr) < textSize){
            fclose(fptr);
            return;
        }
        fclose(fptr);

        memset(buffer,0,272);
        sprintf(buffer," %04d %s %d %s",currentMessageID,authorID,textSize,text);
        sendTCP(fd,buffer,strlen(buffer));

        // Verify if there is a file in the message
        if(!getMessageFilePath(groupID,currentMessageID,fileName)){
            // There is no file in this message
            logRTVMessage(server.verbose, currentMessageID, textSize, 0, NULL);
            continue;
        }

        sprintf(path,"GROUPS/%s/MSG/%04d/%s",groupID,currentMessageID,fileName);
        if(!(fptr = fopen(path, "r"))){
            logError(server.verbose, "Failed to open message file.");
            return;
        }

        // Get message file size
        fseek(fptr,0L,SEEK_END);
        fileSize = ftell(fptr);
        rewind(fptr);

        memset(buffer,0,272);
        sprintf(buffer," / %s %d ",fileName,fileSize);
        sendTCP(fd,buffer,strlen(buffer));

        int read = 0;
        int actuallyRead = 0;

        while(read < fileSize){

            memset(buffer, 0, 512);
            // Read from file
            if(fileSize - read > 512){
                actuallyRead = fread(buffer,sizeof(char),512,fptr);
            }else {
                actuallyRead = fread(buffer,sizeof(char),fileSize - read,fptr);
            }

            // Send to socket
            sendTCP(fd,buffer,actuallyRead);
            read += actuallyRead;
        }

        logRTVMessage(server.verbose, currentMessageID, textSize, fileSize, fileName);

        fclose(fptr);

    }
    strcpy(buffer,"\n");
    
    sendTCP(fd,buffer,strlen(buffer));
}