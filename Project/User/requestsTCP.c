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
#define FILENAMEMAX 200

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
    else if((strlen(extra) != 0) || strlen(text) == 0 || (strlen(command) != 4) || (strlen(text) > TEXTSIZE) || !checkStringIsFileName(filename)){
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

    FILE* fp;
    long int fsize;
    char command[MAXSIZE], text[MAXSIZE], filename[MAXSIZE], extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    memset(filename,0,sizeof filename);
    sscanf(input,"%s \"%[^\"]\" %s %s\n",command, text, filename, extra);

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
    else if(strlen(filename)){
        if(filename[strlen(filename)-4] != '.'){
            logError("Wrong file format.");
            return;
        }
        else if(!(fp = fopen(filename, "rb"))){
            logError("File doesn't exist");
            return;
        }
    }

    connectTCP(server,&fd,res);

    char message[295];

    if(strlen(filename)){
        fseek(fp,0L,SEEK_END);
        fsize = ftell(fp);
        rewind(fp);

        sprintf(
            message,"PST %s %02d %ld %s %s %ld ",
            user->ID,atoi(user->groupID),strlen(text),text,filename, fsize
        );

        sendMessageTCP(fd,message,strlen(message));

        int sent = 0;
        char buffer[500];   

        int i = 0;
        int actuallyRead = 0;
        while(sent < fsize){
            memset(buffer,0,500);
            actuallyRead = fread(buffer,sizeof(char),500,fp);
            printf("%s",buffer);
            sendMessageTCP(fd,buffer,actuallyRead);
            printf("%d\n",actuallyRead);
            sent += actuallyRead;
            printf("%d\n",sent);
            i ++;
        }
        sendMessageTCP(fd,"\n",1);
        printf("Sent file in %d messages\n",i);
    }
    else{
        sprintf(
            message,"PST %s %02d %ld %s\n",
            user->ID,atoi(user->groupID),strlen(text),text
        );
        sendMessageTCP(fd,message,strlen(message));
    }

    char* response;
    response = receiveMessageTCP(fd);
    printf("%s",response);
}

void processRetrieve(userData* user, serverData* server, char* input){

    int fd;
    int msgSize;
    struct addrinfo *res;

    char command[MAXSIZE], MID[MAXSIZE], extra[MAXSIZE];
    memset(extra,0,sizeof extra);

    sscanf(input,"%s %s %s\n",command,MID,extra);

    if(!strcmp(user->ID,"")){
        logError("No user logged in.");
        return;
    }
    else if(!strcmp(user->groupID,"")){
        logError("No group is selected.");
        return;
    }
    else if((strlen(extra) != 0) || (strlen(command) != 8 && strlen(command) != 1) || strlen(MID)>4){
        logError("Wrong size parameters.");
        return;
    }

    connectTCP(server,&fd,res);

    char message[14];

    sprintf(message, "RTV %s %02d %04d\n",user->ID,atoi(user->groupID),atoi(MID));

    sendMessageTCP(fd,message,strlen(message));

    //very cool

    // RRT OK 10 0020 12345 11 hello world 0021 12345 4 helo \ image.jpg 252 asfwfeofewmgoewgmwe  

    char header[10];
    char commando[9],status[9],numberOfMessages[9];
    int amountOfMessages;
    //Named space, but wont always be a space
    char space[1];

    // Reads header (ex: 'RTT OK 10')
    memset(header, NULL, 10);
    read(fd,header,9);
    sscanf(header,"%s %s %s",commando,status,numberOfMessages);
    printf("HEADER: %s\n",header);

    // verify command and status

    if(numberOfMessages[1] == ' '){
        amountOfMessages = numberOfMessages[0];
    }
    else {
        amountOfMessages = atoi(numberOfMessages);
        read(fd,space,1);
    }

    printf("Ammount of messages: %d\n", amountOfMessages);

    char info[11];
    char MessID[5], UserID[6], TSizeString[4];
    char text[240];

    memset(info, NULL, 11);
    read(fd,info,1);

    for (int i = 0; i < amountOfMessages; i++)
    {
        memset(text, NULL, 240);
        read(fd, info + 1, 9);
        sscanf(info, "%s %s", MessID, UserID);

        //TODO - Check if it really is a space
        read(fd, space, 1);

        memset(TSizeString, NULL, 4);
        //Check Text Size
        for (int i = 0; i < 3; i++)
        {
            read(fd, space, 1);
            if(space[0] == ' '){ 
                if (i == 0){
                    printf("Error no 1\n");
                    logError("Bad Formatting");
                    exit(1);
                }
                break;
            }
            TSizeString[i] = space[0];
        }
        if(space[0] =! ' '){ 
            printf("Error no 2\n");
            logError("Bad formatting");
            exit(1);
        }

        int TSize = atoi(TSizeString);

        //Read Text
        read(fd, text, TSize);

        //TODO - Check if it really is a space
        read(fd, space, 1);

        //Named space, but we hope it wont be a space
        read(fd, space, 1);

        printf("Space: %c\n", space[0]);

        printf("Message: %s", text);

        // Check if there is a file or not
        if (space[0] != '/'){
            memset(info, NULL, 11);
            info[0] = space[0];
            printf("Message number %s without file\n\n", MessID);
            continue;
        }
        
        else if (space[0] == '/'){
            //TODO - Check if it really is a space
            read(fd, space, 1);

            char fileName[FILENAMEMAX + 1];
            memset(fileName, NULL, FILENAMEMAX + 1);

            //Read FileName
            //TODO - Check if filename sent by the server is not too big
            for (int i = 0; i < FILENAMEMAX; i++)
            {
                read(fd, space, 1);
                if (space[0] == ' ')
                {
                    break;
                }
                fileName[i] = space[0];
            }

            //Read FileSize
            char fileSizeString[11];
            memset(fileSizeString, NULL, 11);

            for (int i = 0; i < 10; i++)
            {
                read(fd, space, 1);
                if (space[0] == ' ')
                {
                    break;
                }
                //TODO - Check if numerical
                fileSizeString[i] = space[0];
            }

            int fileSize = atoi(fileSizeString);

            printf("File size: %d\n", fileSize);

            //Read File
            char fileBuffer[512];

            printf("File name: %s\n", fileName);

            FILE *downptr;
            downptr = fopen(fileName, "wb");
            //TODO - Check if file was oppened correctly

            int sum = 0;
            int rd = 0;

            for (int i = 0; sum < fileSize; i++)
            {
                memset(fileBuffer, NULL, 512);
                if (fileSize - sum > 512)
                {
                    rd = read(fd, fileBuffer, 512);
                }
                else
                {
                    rd = read(fd, fileBuffer, fileSize - sum);
                }
                if (rd == -1){
                    logError("Something really wrong happened");
                    exit(1);
                }
                sum += rd;
                printf("Sum: %d\n", sum);
                fwrite(fileBuffer, sizeof(char), rd, downptr);
            }
            //actuallyRead = fread(buffer,sizeof(char),500,fp);


            fclose(downptr); 

            //TODO - Check if it really is a space
            read(fd, space, 1);

            read(fd, space, 1);

            memset(info, NULL, 11);
            info[0] = space[0];

            printf("Message number %s with file\n", MessID);
        }

        else{
            printf("Error no 3\n");
            logError("Bad formatting");
            exit(1);
        }

    }


/*
    char* ptr; 
    char buffer[300];
    char filename[300],fileSize[300];
    int bytesInFile;
    char currentChar[1];
    int numberOfSpaces;

    for(int i = 0; i<amountOfMessages; i++){

        memset(buffer,0,300);
        numberOfSpaces = 0;
        ptr = buffer;
        // 
        while(numberOfSpaces != 3){
            read(fd,ptr,1);
            if(*ptr == ' '){
                numberOfSpaces++;
            }
            ptr += 1;
        }
        pread(fd,currentChar,1,strlen(buffer)+1);
        printf("%c",currentChar[0]);
        if(currentChar[0] != '\\'){
            
            printf("MESSAGE WITH NO FILE: %s\n",buffer);
            break;
        }
        else{
            memset(buffer,0,300);
            numberOfSpaces = 0;
            ptr = buffer;
            while(numberOfSpaces != 3){
                read(fd,ptr,1);
                if(*ptr == ' '){
                    numberOfSpaces++;
                }
                ptr += 1;
            }
            sscanf(buffer," %s %s ",filename,fileSize);
            bytesInFile = atoi(fileSize);
            printf("MESSAGE WITH FILE SIZE OF: %s",bytesInFile);
            // read file
        }
        
    }
    */
}
