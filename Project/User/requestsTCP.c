#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "log.h"
#include "structs.h"
#include "common.h"
#include "colors.h"


// Booleans
#define TRUE  1
#define FALSE 0

// Max size user can input
#define MAXINPUTSIZE 274
// Max size server can respond
// Corresponds to a ulist response with all groups
// RUL status [GName [UID ]*] -> 3+1+2+1+99*(24+1+5+1)+1 = 3077
#define MAXRESPONSESIZE 3077

#define TEXTSIZE 240
#define FILENAMESIZE 24
#define FILEBUFFERSIZE 512
#define FILESIZEMAXDIGITS 10

/**
 * Connect via TCP socket to server.
 * @param fd File descriptor of UDP socket
 * @param res Information about server 
 * @return 1 for success or 0 for errors
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
 * @param fd File descriptor of TCP socket
 * @param message Message to be sent
 * @param messageLen Message length
 * @return 1 for success or 0 for errors
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
 * Receive the whole message via TCP socket from server.
 * @param fd File descriptor of TCP socket
 * @return message from server (needs to be freed) or NULl for errors
*/
char* receiveWholeTCP(int fd){
    int nRead = 1; // This can't be initialized as 0
    char* message = calloc(MAXRESPONSESIZE,sizeof(char));
    char* ptr;

    ptr = message;

    while (nRead != 0){
        nRead = read(fd, ptr, MAXRESPONSESIZE);
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
 * Receive message with length messageSize via TCP socket from server.
 * @param fd File descriptor of UDP socket
 * @param buffer Buffer to be filled with message from the server
 * @param messageSize Size of the desired message
 * @return number of bytes read or -1 for errors
*/
int receiveNSizeTCP(int fd, char* buffer, int messageSize){
    
    char* ptr = buffer;
    int nRead,nReceived;

    ptr = buffer;
    nReceived = 0;
    while (messageSize > 0){
        nRead = read(fd, ptr, messageSize);
        if (nRead == -1){
            logError("Couldn't receive message via TCP socket.");
            return -1;
        }
        if (nRead == 0){
            break;
        }
        ptr += nRead;
        nReceived += nRead;
        messageSize -= nRead;
    }
    return nReceived;
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
        logError("Bad formatting");
        return FALSE;
    }
    return TRUE;
}

/**
 * Reads a word of max size maxRead from fd and checks for a space after
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

/**
 * Checks the existence of the downloads folder, if there isn't any, creates one
 * @return 1 for success or 0 for errors
 */
int downloadsFolder(){

    DIR* dir;
    char downloads[10];

    strcpy(downloads, "Downloads");
    if(dir = opendir("Downloads")){
        closedir(dir);
        return TRUE;
    }
    else if(ENOENT == errno){
        if(mkdir(downloads, 0700) == -1){
            return FALSE;
        }
        return TRUE;
    }
    else{
        return FALSE;
    }
}

/**
 * Fills newName with a slightly different name if there are files with the same name as this one
 * @param originalName the original file's name
 * @param newName the new name that will be attributed
 * @return 1 for success or 0 if there are too many files with the same name
 */
int attributeFileName(char* originalName, char* newName){

    char path[79];
    char identifier[5];
    char name[25];
    char extension[25];
    memset(name, 0, 25);
    memset(extension, 0, 25);

    int i = 0;

    while (originalName[i] != '.') i++;

    strncpy(name, originalName, i);
    strcpy(extension, originalName + i);
    
    sprintf(path, "Downloads/%s", originalName);

    if (access(path, F_OK) != 0){
        strcpy(newName, originalName);
        return TRUE;
    }
    else{
        for (int i = 1; i < 10; i++){
            sprintf(path, "Downloads/%s(%d)%s", name, i, extension);
            if (access(path, F_OK) != 0){
                sprintf(newName, "%s(%d)%s", name, i, extension);
                return TRUE;
            }
        }
        return FALSE;
    }
}

/**
 * Process ulist command
 * @param user User data
 * @param server Server data
 * @param input User input to be parsed
*/
void processUlist(userData *user, serverData *server, char* input){

    int msgSize;
    char *response;
    char message[8];
    char command[MAXINPUTSIZE], extra[MAXINPUTSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s\n",command, extra);

    if(!strcmp(user->ID,"")){
        logError("No user is logged in.");
        return;
    }
    else if (!strcmp(user->groupID,"")){
        logError("No group is selected.");
        return;
    }
    else if((strlen(extra) != 0) || (strlen(command) != 5 && strlen(command) != 2)){
        logError("Wrong size parameters.");
        return;
    }

    sprintf(message, "ULS %02d\n", atoi(user->groupID));

    if(!connectTCP(server, &(user->fd), user->res)) return;
    if(!sendTCP(user->fd, message, strlen(message))) return;
    response = receiveWholeTCP(user->fd);
    if(response == NULL){
        return; 
    } 

    logULS(response);

    free(response);
}

/**
 * Process post command
 * @param user User data
 * @param server Server data
 * @param input User input to be parsed
*/
void processPost(userData* user, serverData* server, char* input){

    int msgSize, sent, actuallyRead;

    FILE* fp;
    long int fsize;
    char* response; // // will get calloc'd inside readwholetcp and should be free'd
    char command[MAXINPUTSIZE], text[MAXINPUTSIZE], fileName[MAXINPUTSIZE], extra[MAXINPUTSIZE];
    char buffer[FILEBUFFERSIZE]; 

    memset(extra, 0, MAXINPUTSIZE);
    memset(text, 0, MAXINPUTSIZE);
    memset(fileName, 0, MAXINPUTSIZE);
    sscanf(input,"%s \"%[^\"]\" %s %s\n",command, text, fileName, extra);

    if(input[strlen(command) + 1] != '\"' || input[strlen(command) + 1 + strlen(text) + 1] != '\"'){
        // Checks if text is inside quotation marks
        logError("Wrong text format.");
        return;
    }
    else if(!strcmp(user->ID,"")){
        logError("No user logged in.");
        return;
    }
    else if (!strcmp(user->groupID,"")){
        logError("No group is selected.");
        return;
    }
    else if((strlen(extra) != 0) || strlen(text) == 0 || (strlen(command) != 4) || (strlen(text) > TEXTSIZE) || (strlen(fileName) > FILENAMESIZE)){
        logError("Wrong size parameters.");
        return;
    }
    else if(strlen(fileName)){
        if(fileName[strlen(fileName)-4] != '.'){
            logError("Wrong file format.");
            return;
        }
        if(!(fp = fopen(fileName, "rb"))){
            logError("File doesn't exist.");
            return;
        }
    }

    if(!connectTCP(server, &(user->fd), user->res)) return;

    if(strlen(fileName)){

        // Get file size
        fseek(fp,0L,SEEK_END);
        fsize = ftell(fp);
        rewind(fp);

        sprintf(buffer,"PST %s %02d %ld %s ",user->ID,atoi(user->groupID),strlen(text),text);

        // Send PST UID GID textSize text
        if(!sendTCP(user->fd,buffer,strlen(buffer))) return;

        sprintf(buffer,"%s %ld ",fileName,fsize);

        // Send Fname Fsize
        if(!sendTCP(user->fd,buffer,strlen(buffer))) return;

        sent = 0;
        actuallyRead = 0;

        // Send "packages" of size FILEBUFFERSIZE at a time from given file
        while(sent < fsize){
            // Updating message 
            printf("\rUploading file... %d out of %ld bytes", sent, fsize);
            fflush(stdout);

            memset(buffer, 0, FILEBUFFERSIZE);

            // Read to buffer from file
            actuallyRead = fread(buffer, sizeof(char), FILEBUFFERSIZE, fp);

            // Send buffer
            if(!sendTCP(user->fd,buffer,actuallyRead)) return;

            sent += actuallyRead;
        }
        // Message to server must end with \n
        if(!sendTCP(user->fd,"\n",1)) return;
        printf("\rUploading file... %-30s\n", "Done");
    }
    else{
        // Send PST UID GID textSize text
        sprintf(
            buffer,"PST %s %02d %ld %s\n",
            user->ID,atoi(user->groupID),strlen(text),text
        );
        if(!sendTCP(user->fd,buffer,strlen(buffer))) return;
    }

    // Receive response from server
    response = receiveWholeTCP(user->fd);
    if(response == NULL)
        return;
    
    logPST(response);
    free(response);
}

/**
 * Parse retrieve command
 * @param user User data
 * @param input User input to be parsed
 * @return message from server (needs to be freed) or NULl for errors
*/
char* parseRetrieve(userData* user, char* input){
    

}

/**
 * Process retrieve command
 * @param user User data
 * @param server Server data
 * @param input User input to be parsed
*/
void processRetrieve(userData* user, serverData* server, char* input){

    char message[35];
    char command[MAXINPUTSIZE], MID[MAXINPUTSIZE], extra[MAXINPUTSIZE];

    int numOfMessages;
    char header[10], aux[9], numOfMessagesString[9], readChar[1];

    char info[11];
    char messageID[5], userID[6], textSizeDigits[4];
    char text[TEXTSIZE + 1];

    int textSize, fileSize;
    char fileName[FILENAMESIZE + 1];
    char newFileName[FILENAMESIZE + 1];
    char fileSizeDigits[FILESIZEMAXDIGITS + 1];
    char fileBuffer[FILEBUFFERSIZE];

    FILE *downptr;
    char path[40];
    int sum, nRead;

    memset(extra, 0, MAXINPUTSIZE);

    sscanf(input,"%s %s %s\n",command,MID,extra);

    if(!strcmp(user->ID,"")){
        logError("No user logged in.");
        return;
    }
    else if(!strcmp(user->groupID,"")){
        logError("No group is selected.");
        return;
    }
    else if((strlen(extra) != 0) || (strlen(command) != 8 && strlen(command) != 1) || strlen(MID) > 4){
        logError("Wrong size parameters.");
        return;
    }

    sprintf(message, "RTV %s %02d %04d\n",user->ID,atoi(user->groupID),atoi(MID));

    // Creates / checks download folder
    if (!downloadsFolder()){
        logError("Error with the downloads folder");
        return;
    }

    // Send message to server
    if(!connectTCP(server,&(user->fd),user->res)) return;
    if(!sendTCP(user->fd,message,strlen(message))) return;
    
    memset(header, 0, 10);
    memset(numOfMessagesString, 0, 9);

    // Reads header
    if(receiveNSizeTCP(user->fd, header, 9) == -1) return;
    // Verifies if server response is valid and log it
    if(!logRTV(header)) return;

    // Parse header
    sscanf(header,"%s %s %s", aux, aux, numOfMessagesString);

    // Get number of messages
    if(header[strlen(header)-1] == ' '){
        numOfMessages = numOfMessagesString[0];
    }
    else {
        // number of message has 2 digits so we didn't read the space after
        numOfMessages = atoi(numOfMessagesString);
        read(user->fd,readChar,1);
    }

    // Reads first char of message to info
    // For loops starts reading message after the first char to info+1
    memset(info, 0, 11);
    if(receiveNSizeTCP(user->fd, info, 1) == -1) return;

    for (int i = 0; i < numOfMessages; i++){

        // Colors
        i % 2 == 0 ? colorCyan() : colorBlue();

        // Read messageID and userID of text message
        if(receiveNSizeTCP(user->fd, info + 1, 9) == -1) return;
        sscanf(info, "%s %s", messageID, userID);

        if(!skipSpace(user->fd)) return;
        
        // Read text size
        if(!readWord(user->fd,textSizeDigits, 3)){
            logError("Text size too big");
            return;
        }
        textSize = atoi(textSizeDigits);

        //Read Text
        memset(text, 0, TEXTSIZE + 1);
        if(receiveNSizeTCP(user->fd, text, textSize) == -1) return;

        printf("Message: %s\n", text);

        // Skips spaces between messages and if its \n then all messages are read
        if(receiveNSizeTCP(user->fd, readChar, 1) == -1) return;
        if (readChar[0] == '\n') return;
     
        // Next char can be a number (representing the start of the next message)
        // or can be a dash (\) which means this message has a file appended
        if(receiveNSizeTCP(user->fd, readChar, 1) == -1) return;

        // Check if there is a file or not
        if (readChar[0] != '/'){
            memset(info, 0, 11);
            info[0] = readChar[0];
            continue;
        }
        
        else if (readChar[0] == '/'){

            if(!skipSpace(user->fd)) return;

            //Read FileName            
            if(!readWord(user->fd, fileName, FILENAMESIZE)){
                logError("File name is too big");
                return;
            };

            // Read FileSize
            readWord(user->fd, fileSizeDigits, FILESIZEMAXDIGITS);
            fileSize = atoi(fileSizeDigits);
            printf("File size: %d\nFile name: %s\n", fileSize, fileName);

            // Open file
            if(!attributeFileName(fileName, newFileName)){
                logError("Error attributing file name. Please clear your downloads folder.");
                return;
            }

            if (strcmp(fileName, newFileName))
                printf("File renamed to: %s\n", newFileName);

            sprintf(path, "Downloads/%s", newFileName);
            downptr = fopen(path, "wb");

            if(downptr == NULL){
                logError("Could not open file");
                return;
            }

            sum = 0;
            nRead = 0;

            // Reads file in "packages" of size FILEBUFFERSIZE
            for (int i = 0; sum < fileSize; i++)
            {
                // Progress bar
                printf("\rDownloading file... %d out of %d bytes", sum, fileSize);
                fflush(stdout);

                memset(fileBuffer, 0, FILEBUFFERSIZE);
                if (fileSize - sum > FILEBUFFERSIZE){
                    nRead = read(user->fd, fileBuffer, FILEBUFFERSIZE);
                }
                else{
                    nRead = read(user->fd, fileBuffer, fileSize - sum);
                }
                if (nRead == -1){
                    logError("Something really wrong happened");
                    return;
                }
                sum += nRead;
                fwrite(fileBuffer, sizeof(char), nRead, downptr);
            }
            fclose(downptr); 

            printf("\rDownloading file... %-30s\n", "Done");
            printf("File successfully downloaded.\n");

            // Skips spaces between messages and if its \n then all messages are read
            if(receiveNSizeTCP(user->fd, readChar, 1) == -1) return;
            if (readChar[0] == '\n') return;

            // Reads first char of the next message (because messages are read to info + 1)
            if(receiveNSizeTCP(user->fd, readChar, 1) == -1) return;
            memset(info, 0, 11);
            info[0] = readChar[0];
        }
        colorReset();
    }
}