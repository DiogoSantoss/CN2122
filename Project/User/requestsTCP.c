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
#define FILENAMESIZE 24
#define MAXBYTES 8000
#define FILEBUFFERSIZE 512

/**
 * Receive the whole message via TCP socket from server.
 * @param[in] fd File descriptor of TCP socket
 * @param[out] message Message from server
*/
char* receiveWholeTCP(int fd){
    int nRead = 1; // This can't be initialized as 0
    char* message = calloc(EXTRAMAXSIZE,sizeof(char));
    char* ptr;

    ptr = message;

    while (nRead != 0){
        nRead = read(fd, ptr, EXTRAMAXSIZE);
        if(nRead == -1){
            logError("Couldn't receive message via TCP socket.");
            free(message);
            return NULL;
        }
        ptr += nRead;
    }
     
    return message;
}
/**
 * Receive messageSize message via TCP socket from server.
 * @param[in] fd File descriptor of UDP socket
 * @param[in] buffer Buffer to be filled with message from the server
 * @param[in] messageSize Size of the desired message
*/
int receiveNSizeTCP(int fd, char* buffer, int messageSize){
    
    char* ptr = buffer;
    int sum = 0;
    int nRead = 0;

    while (messageSize > sum){
        nRead = read(fd, ptr, messageSize);
        if (nRead == -1){
            logError("Couldn't receive message via TCP socket.");
            return FALSE;
        }
        if (nRead == 0){
            break;
        }
        ptr += nRead;
        sum += nRead;
        messageSize -= nRead;
    }
    return TRUE;
}

/**
 * Connect via TCP socket to server.
 * @param[in] fd File descriptor of UDP socket
 * @param[in] res Information about server 
*/
int connectTCP(serverData *server, int* fd, struct addrinfo* res){
    int errcode,n;
    struct addrinfo hints;

    *fd = socket(AF_INET, SOCK_STREAM, 0);
    if(*fd==-1){
        logError("Couldn't create TCP socket.");
        return FALSE;
    }

    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_STREAM;

    errcode=getaddrinfo(server->ipAddress,server->port,&hints,&res) ;
    if(errcode!=0){
        logError("Couldn't get server info.");
        return FALSE;
    }

    n = connect(*fd,res->ai_addr,res->ai_addrlen);
    if(n==-1){
        logError("Couldn't connect to server.");
        return FALSE;
    }
    
    return TRUE;
}

/**
 * Send message via TCP socket to server.
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

//-------------------------------------------------------------------------------

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

//--------------------------------------------------------------------------------------------

void processPost(userData* user, serverData* server, char* input){

    int fd;
    int msgSize;
    struct addrinfo *res;

    FILE* fp;
    long int fsize;
    char command[MAXSIZE], text[MAXSIZE], filename[MAXSIZE], extra[MAXSIZE];

    //TODO - This message size is sus
    char message[MAXSIZE * 4];

    //----------------------------VERIFY USER INPUT-----------------------------

    memset(extra,0,sizeof extra);
    memset(filename,0,sizeof filename);
    sscanf(input,"%s \"%[^\"]\" %s %s\n",command, text, filename, extra);


    if(!strcmp(user->ID,"")){
        logError("No user logged in.");
        return;
    }
    else if (!strcmp(user->groupID,"")){
        logError("No group is selected.");
        return;
    }
    else if((strlen(extra) != 0) || strlen(text) == 0 || (strlen(command) != 4) || (strlen(text) > TEXTSIZE) || (strlen(filename) > FILENAMESIZE)){
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

    //----------------------------SEND USER INPUT-----------------------------

    if(!connectTCP(server,&fd,res)) return;

    if(strlen(filename)){

        // Get file size
        fseek(fp,0L,SEEK_END);
        fsize = ftell(fp);
        rewind(fp);

        sprintf(
            message,"PST %s %02d %ld %s %s %ld ",
            user->ID,atoi(user->groupID),strlen(text),text,filename,fsize
        );

        // Send PST UID GID Tsize text Fname Fsize
        if(!sendTCP(fd,message,strlen(message))) return;

        // Send data from file

        int sent = 0;
        char buffer[FILEBUFFERSIZE];   
        int actuallyRead = 0;

        // Send "packages" of size FILEBUFFERSIZE at a time
        while(sent < fsize){
            memset(buffer, 0, FILEBUFFERSIZE);
            // Read "package" from file
            actuallyRead = fread(buffer, sizeof(char), FILEBUFFERSIZE, fp);
            // Send "package"
            if(!sendTCP(fd,buffer,actuallyRead)) return;
            sent += actuallyRead;
        }
        // Message to server must end with \n
        if(!sendTCP(fd,"\n",1)) return;
    }
    else{
        // Send PST UID GID Tsize text
        sprintf(
            message,"PST %s %02d %ld %s\n",
            user->ID,atoi(user->groupID),strlen(text),text
        );
        if(!sendTCP(fd,message,strlen(message))) return;
    }

    char* response;
    response = receiveWholeTCP(fd);
    logPST(response);

    free(response);
}

//--------------------------------------------------------------------------------------------

void processRetrieve(userData* user, serverData* server, char* input){

    int fd;
    int msgSize;
    struct addrinfo *res;

    char command[MAXSIZE], MID[MAXSIZE], extra[MAXSIZE];

    //----------------------------VERIFY USER INPUT-----------------------------

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

    //----------------------------SEND USER INPUT-----------------------------

    if(!connectTCP(server,&fd,res)) return;

    char message[35];

    sprintf(message, "RTV %s %02d %04d\n",user->ID,atoi(user->groupID),atoi(MID));

    if(!sendTCP(fd,message,strlen(message))) return;

    //----------------------------RECEIVE SERVER INPUT-----------------------------

    // ERR
    // RRT NOK
    // RRT EOF (N=0)
    // RRT OK 10 0020 12345 11 hello world 0021 12345 4 helo \ image.jpg 252 asfwfeofewmgoewgmwe


    char header[10];
    char serverCommand[9],serverStatus[9],numberOfMessages[9];
    int amountOfMessages;
    char space[1]; //Named space, but wont always be a space

    // Reads header (ex: 'RRT OK 10', 'RRT NOK', 'RRT EOF', 'ERR')
    memset(header, 0, 10);
    memset(numberOfMessages, 0, 9);

    if(!receiveNSizeTCP(fd, header, 9)) return;

    if(!logRTV(header)) return;

    sscanf(header,"%s %s %s",serverCommand,serverStatus,numberOfMessages);

    if(numberOfMessages[1] == ' '){
        amountOfMessages = numberOfMessages[0];
    }
    else {
        amountOfMessages = atoi(numberOfMessages);
        read(fd,space,1);
    }

    printf("Amount of messages: %d\n", amountOfMessages);

    char info[11];
    char MessID[5], UserID[6], TSizeString[4];
    char text[240];

    memset(info, 0, 11);
    if(!receiveNSizeTCP(fd, info, 1)) return;

    for (int i = 0; i < amountOfMessages; i++)
    {
        i % 2 == 0 ? cyan() : white();
        memset(text, 0, 240);
        if(!receiveNSizeTCP(fd, info + 1, 9)) return;
        sscanf(info, "%s %s", MessID, UserID);

        //TODO - Check if it really is a space
        if(!receiveNSizeTCP(fd, space, 1)) return;

        memset(TSizeString, 0, 4);
        //Check Text Size
        for (int i = 0; i < 3; i++)
        {
            if(!receiveNSizeTCP(fd, space, 1)) return;
            if(space[0] == ' '){ 
                if (i == 0){
                    logError("Bad Formatting");
                    return;
                }
                break;
            }
            TSizeString[i] = space[0];
        }
        if(space[0] =! ' '){ 
            logError("Bad formatting");
            return;
        }

        int TSize = atoi(TSizeString);

        //Read Text
        if(!receiveNSizeTCP(fd, text, TSize)) return;

        //TODO - Check if it really is a space
        if(!receiveNSizeTCP(fd, space, 1)) return;

        //Named space, but we hope it wont be a space
        if(!receiveNSizeTCP(fd, space, 1)) return;

        printf("Message: %s", text);

        // Check if there is a file or not
        if (space[0] != '/'){
            memset(info, 0, 11);
            info[0] = space[0];
            continue;
        }
        
        else if (space[0] == '/'){
            printf("Downloading file...\n");
            //TODO - Check if it really is a space
            if(!receiveNSizeTCP(fd, space, 1)) return;

            char fileName[FILENAMESIZE + 1];
            memset(fileName, 0, FILENAMESIZE + 1);

            //Read FileName
            //TODO - Check if filename sent by the server is not too big
            for (int i = 0; i < FILENAMESIZE; i++)
            {
                read(fd, space, 1);
                if (space[0] == ' '){
                    break;
                }
                fileName[i] = space[0];
            }

            //Read FileSize
            char fileSizeString[11];
            memset(fileSizeString, 0, 11);

            for (int i = 0; i < 10; i++)
            {
                if(!receiveNSizeTCP(fd, space, 1)) return;
                if (space[0] == ' ') break;

                //TODO - Check if numerical
                fileSizeString[i] = space[0];
            }

            int fileSize = atoi(fileSizeString);

            printf("File size: %d\nFile name: %s\n", fileSize, fileName);

            //Read File
            char fileBuffer[FILEBUFFERSIZE];

            FILE *downptr;
            downptr = fopen(fileName, "wb");
            //TODO - Check if file was oppened correctly

            int sum = 0;
            int rd = 0;

            for (int i = 0; sum < fileSize; i++)
            {
                memset(fileBuffer, 0, FILEBUFFERSIZE);
                if (fileSize - sum > FILEBUFFERSIZE){
                    rd = read(fd, fileBuffer, FILEBUFFERSIZE);
                }
                else{
                    rd = read(fd, fileBuffer, fileSize - sum);
                }
                if (rd == -1){
                    logError("Something really wrong happened");
                    return;
                }
                sum += rd;
                fwrite(fileBuffer, sizeof(char), rd, downptr);
            }

            fclose(downptr); 

            //TODO - Check if it really is a space
            if(!receiveNSizeTCP(fd, space, 1)) return;

            if(!receiveNSizeTCP(fd, space, 1)) return;

            memset(info, 0, 11);
            info[0] = space[0];
        }
        else{
            logError("Bad formatting");
            return;
        }
    }
    reset(); //color reset
}

//--------------------------------------------------------------------------------------------



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

    if(!connectTCP(server,&fd,res)) return;
    if(!sendTCP(fd,message,strlen(message))) return;
    response = receiveWholeTCP(fd);

    if(helper != NULL){
        (*helper)(user,response);
    }

    (*logger)(response);

    free(message);
    free(response);
}