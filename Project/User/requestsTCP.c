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
#define TEXTSIZE 240
#define FILESIZE 24
#define MAXBYTES 8000

char* parseUlist(userData* user, char* input){
    
    char* message;
    char command[MAXSIZE], extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s\n",command, extra);

    if(!strcmp(user->ID,"")){
        logError("No user is logged in.");
        return NULL;
    }

    else if (!strcmp(user->groupID,"")){
        logError("No group is selected.");
        return NULL;
    }

    else if((strlen(extra) != 0) || (strlen(command) != 5 && strlen(command) != 2)){
        logError("Wrong size parameters.");
        return NULL;
    }

    message = malloc(sizeof(char)*7);
    sprintf(message, "ULS %02d\n", atoi(user->groupID));

    return message;
}



/*
TODO
-do files from post come only from our dir or can come from outside
better parse to handle  <post "a b"> the spaces between the message
-test if filename is correct  (Ex:  test.txt is good but test is not)
*/

char* parsePost(userData* user, char* input){

    char *message, *buffer;
    char command[MAXSIZE], text[MAXSIZE], filename[MAXSIZE], extra[MAXSIZE];
    long int fsize;

    memset(extra,0,sizeof extra);
    memset(filename,0,sizeof extra);
    sscanf(input,"%s \"%[^\"]\" %s %s\n",command, text, filename, extra);

    if(!strcmp(user->ID,"")){
        logError("No user is logged in.");
        return NULL;
    }
    else if (!strcmp(user->groupID,"")){
        logError("No group is selected.");
        return NULL;
    }
    else if((strlen(extra) != 0) || strlen(text) == 0 || (strlen(command) != 4) || (strlen(text) > TEXTSIZE) || (strlen(filename) > FILESIZE)){
        logError("Wrong size parameters.");
        return NULL;
    }


    if(strlen(filename) > 4){
        FILE* fp = fopen(filename, "r");
        if(fp == NULL){
            logError("File not found");
            return NULL;
        }
        fseek(fp, 0L, SEEK_END);
        fsize = ftell(fp);
        if(fsize == -1){
            logError("Invalid file size.");
            fclose(fp);
            return NULL;
        }
        if(fsize >= MAXBYTES){
            logError("File too big.");
            fclose(fp);
            return NULL;
        }
        rewind(fp);

        buffer = malloc(fsize + 1);
        fread(buffer, sizeof(char), fsize, fp);
        buffer[fsize] = '\0';

        printf("file size: %ld\n",fsize);

        message = malloc(sizeof(char)*(fsize + 295));
        sprintf(message, "PST %s %02d %ld %s %s %ld %s\n", user->ID, atoi(user->groupID),
                strlen(text), text, filename, fsize, buffer);

        printf("message: %s\n",message);
        
        free(buffer);
        fclose(fp);
    }

    else{
        message = malloc(sizeof(char)*(MAXSIZE));
        sprintf(message, "PST %s %02d %ld %s\n", user->ID, atoi(user->groupID), strlen(text), text);
    }

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

    errcode=getaddrinfo(server->ipAddress,server->port,&hints,&res) ;
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
    int nRead = 1; // This can't be initialized as 0
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
    sendMessageTCP(fd,message,strlen(message));
    response = receiveMessageTCP(fd);

    if(helper != NULL){
        (*helper)(user,response);
    }

    (*logger)(response);

    free(message);
    free(response);
}

void processPost(userData* user, serverData* server, char* input){
    int fd;
    int msgSize;
    struct addrinfo *res;

    /**
     * All verifications:
     *  User logged in
     *  User belongs to a group
     *  Text message max 240 chars
     *  Check if file exists
     *  Filename max 24 alphanumeric chars(and - _ .)
     *  Fsize field at max has 10 digits
     * 
    */

    FILE* fp;
    long int fsize;
    char command[MAXSIZE], text[MAXSIZE], filename[MAXSIZE], extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    memset(filename,0,sizeof extra);
    sscanf(input,"%s \"%[^\"]\" %s %s\n",command, text, filename, extra);

    printf("DEBUG:\n");
    printf("Command:%s\nText:%s\nFilename:%s\n",command,text,filename);

    if(!strcmp(user->ID,"")){
        logError("No user logged in.");
        return;
    }
    else if (!strcmp(user->groupID,"")){
        logError("No group is selected.");
        return;
    }
    else if((strlen(extra) != 0) || strlen(text) == 0 || (strlen(command) != 4) || (strlen(text) > TEXTSIZE) || (strlen(filename) > FILESIZE)){
        logError("Wrong size parameters.");
        return;
    }
    else if(filename[strlen(filename)-4] != '.'){
        logError("Wrong file format.");
        return;
    }
    else if(!(fp = fopen(filename, "rb"))){
        logError("File doesn't exist");
        return;
    }

    fseek(fp,0L,SEEK_END);
    fsize = ftell(fp);
    rewind(fp);

    connectTCP(server,&fd,res);

    char message[295];

    sprintf(
        message,"PST %s %02d %ld %s %s %ld ",
        user->ID,atoi(user->groupID),strlen(text),text,filename, fsize
    );

    sendMessageTCP(fd,message,295);
    //printf("%s",message);

    int sent = 0;
    char buffer[500];   

    int i = 0;
    int actuallyRead = 0;
    while(sent <= fsize){
        memset(buffer,0,500);
        actuallyRead = fread(buffer,sizeof(char),500,fp);
        sendMessageTCP(fd,buffer,actuallyRead);
        //printf("%s",buffer);
        sent += actuallyRead;
        i ++;
    }
    sendMessageTCP(fd,"\n",1);
    // no fim de enviar o ficheiro falta mandar o \n ?

    printf("Sent file in %d messages\n",i);

    char* response;
    response = receiveMessageTCP(fd);
    printf("%s",response);

    
}