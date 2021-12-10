#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>

#include "log.h"


// Booleans
#define TRUE  1
#define FALSE 0

//Constants
#define MAXSIZE 274


// Global Variables
// Default Port
char port[6] = "58011";
// Default IP address
char ipAddress[513] = "tejo.tecnico.ulisboa.pt";
// User ID
char userID[6] = "";
// User Password
char userPassword[9] = "";
// Group ID
char groupID[3] = "";


/**
 * Check if String is a number
 * @param[in] value String to be checked
 * @param[out] IsNumber TRUE if is number else FALSE
*/
int checkStringIsNumber(char* value){
    int IsNumber = TRUE;
    for(int i = 0; i<strlen(value); i++){
        if(!isdigit(value[i])){
            IsNumber = FALSE;
            break;
        }
    } 
    return IsNumber;
}

/**
 * Check if String is alphanumeric
 * @param[in] value String to be checked
 * @param[out] IsNumber TRUE if is alphanumeric else FALSE
*/
int checkStringIsAlphaNum(char* value){
    int IsAlphaNumeric = TRUE;
    for(int i = 0; i<strlen(value)-1; i++){
        if(!isdigit(value[i]) && !isalpha(value[i])){
            IsAlphaNumeric = FALSE;
            break;
        }
    } 
    return IsAlphaNumeric;
}

int checkStringIsGroupName(char* value){
    int IsGroupName = TRUE;
    for(int i = 0; i<strlen(value)-1; i++){
        if(!isdigit(value[i]) && !isalpha(value[i]) && value[i] != '-' && value[i] != '_'){
            IsGroupName = FALSE;
            break;
        }
    } 
    return IsGroupName;
}


/**
 * Parse command-line arguments
 * Only two available arguments:
 *  -n DSIP where DSIP is the IP address of the machine where the server runs
 *  -p DSport where DSport is the well-known port where server accepts requests
 * @param[in] argc Number of elements in argv
 * @param[in] argv Array of command-line arguments
*/
void parseArguments(int argc, char *argv[]){

    char opt;
    int nCounter = 0;
    int pCounter = 0;

    if(argc%2 == 0){
        logError("Invalid number of command-line arguments.");
        return;
    }
    
    // the getopt function parses command-line arguments
    // and returns the flag , eg: "-n" -> opt = 'n'
    // also has external variables such as optarg that stores
    // the flag argument, eg: "-n guadiana" -> optarg = "guadiana"
    while((opt = getopt(argc, argv, "n:p:")) != -1) 
    { 
        switch(opt) 
        { 
            case 'n': 
                if(optarg[0] != '-'){
                    strcpy(ipAddress,optarg);
                }
                nCounter++;
                break;
            case 'p':
                if(!checkStringIsNumber(optarg)){
                    logError("Port value should be a positive integer.");
                    exit(1);
                }
                strcpy(port,optarg);
                pCounter++;
                break;  
            default: 
                logError("Wrong arguments.");
                exit(1);
        }
        if(nCounter > 1 || pCounter > 1){ 
            logError("Repeated command-line arguments.");
            exit(1);
        }
    }
    printf("DEBUG: Port:%s IP Address:%s\n",port,ipAddress);
}

/**
 * Connect via UDP socket to server.
 * @param[in] fd File descriptor of UDP socket
 * @param[in] res Information about server 
*/
void UDPconnect(int* fd, struct addrinfo** res){
    int errcode;
    struct addrinfo hints;

    *fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(*fd==-1){
        logError("Couldn't create UDP socket.");
        exit(1);
    }

    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;

    errcode=getaddrinfo(ipAddress,port,&hints,res);
    if(errcode!=0){
        logError("Couldn't get server info.");
        exit(1);
    }
}

/**
 * Send message via UDP socket to server.
 * @param[in] fd File descriptor of UDP socket
 * @param[in] res Information about server 
 * @param[in] message Message to be sent
 * @param[in] messageLen Message length
*/
void UDPsendMessage(int fd, struct addrinfo* res, char* message, int messageLen){
    int n;
    socklen_t addrlen;
    struct sockaddr_in addr;

    //printf("DEBUG USER MESSAGE:%s",message);
    n = sendto(fd, message,messageLen,0,res->ai_addr,res->ai_addrlen);
    if(n==-1){
        logError("Couldn't send message via UDP socket");
        exit(1);
    }   
}

/**
 * Receive message via UDP socket from server.
 * @param[in] fd File descriptor of UDP socket
 * @param[out] message Message from server
*/
char* UDPreceiveMessage(int fd){
    int n;
    socklen_t addrlen;
    struct sockaddr_in addr;
    char* message = malloc(sizeof(char)*128);

    addrlen = sizeof(addr);
    n = recvfrom(fd,message,128,0,(struct sockaddr*)&addr,&addrlen);
    if(n==-1){
        logError("Couldn't receive message via UDP socket");
        exit(1);
    } 
    return message;
}

/**
 * Connect via TCP socket to server.
 * @param[in] fd File descriptor of UDP socket
 * @param[in] res Information about server 
*/
void TCPconnect(int* fd, struct addrinfo* res){
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

    errcode=getaddrinfo(ipAddress,port,&hints,&res) ;
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
void TCPsendMessage(int fd, char* message, int messageLen){
    int n;

    n = write(fd, message,messageLen);
    if(n==-1){
        logError("Couldn't send message via TCP socket");
        exit(1);
    }
}

/**
 * Parse register command.
 * @param[in] input User input to be parsed
 * @param[out] message Formarted message to send to server
*/
char* parseRegister(char* input){

    char* message;
    char command[MAXSIZE],UID[MAXSIZE],pass[MAXSIZE],extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s %s %s\n",command,UID,pass,extra);

    if((strlen(extra) != 0) || (strlen(input) != 19) || (strlen(UID) != 5) || (strlen(pass) != 8)){
        logREG("Wrong size parameters.");
        return NULL;

    } else if(!checkStringIsNumber(UID) || !checkStringIsAlphaNum(pass)){
        logREG("Forbidden character in parameters.");
        return NULL;
    }

    message = malloc(sizeof(char)*18);
    sprintf(message,"REG %s %s\n",UID,pass);

    return message;
}

char* parseUnregister(char* input){

    char* message;
    char command[MAXSIZE],UID[MAXSIZE],pass[MAXSIZE],extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s %s %s\n",command,UID,pass,extra);

    if((strlen(extra) != 0) || ((strlen(input) != 19) && (strlen(input) != 26)) || (strlen(UID) != 5) || (strlen(pass) != 8)){
        logREG("Wrong size parameters.");
        return NULL;

    } else if(!checkStringIsNumber(UID) || !checkStringIsAlphaNum(pass)){
        logREG("Forbidden character in parameters.");
        return NULL;
    }

    message = malloc(sizeof(char)*18);
    sprintf(message,"UNR %s %s\n",UID,pass);

    return message;
} 

char* parseLogin(char* input){

    char* message;
    char command[MAXSIZE],UID[MAXSIZE],pass[MAXSIZE],extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s %s %s\n",command,UID,pass,extra);

    if((strlen(extra) != 0) || (strlen(input) != 21) || (strlen(UID) != 5) || (strlen(pass) != 8)){
        logLOG("Wrong size parameters.");
        return NULL;

    } else if(!checkStringIsNumber(UID) || !checkStringIsAlphaNum(pass)){
        logLOG("Forbidden character in parameters.");
        return NULL;
    }

    if(strcmp(userID,"")){
        logLOG("A user is already logged in.");
        return NULL;

    } else {
        strcpy(userID,UID);
        strcpy(userPassword,pass);
    }

    message = malloc(sizeof(char)*18);
    sprintf(message,"LOG %s %s\n",UID,pass);
    
    return message;
}

void helperLogin(char* response){
    if(!strcmp(response,"RLO NOK\n")){
        strcpy(userID,"");
    }
}

char* parseLogout(char* input){

    char* message;
    char command[MAXSIZE],extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s\n",command,extra);

    if((strlen(extra) != 0) || (strlen(input) != 7)){
        logOUT("Wrong size parameters.");
        return NULL;

    }

    if(!strcmp(userID,"")){
        logOUT("No user is logged in.");
        return NULL;
    }

    message = malloc(sizeof(char)*18);
    sprintf(message,"OUT %s %s\n",userID,userPassword);
    printf("%ld\n", strlen(userPassword));

    return message;
}

void helperLogout(char* response){
    if(!strcmp(response,"ROU OK\n")){
        strcpy(userID,"");
        strcpy(userPassword,"");
    }
}

void processShowUID(char* input){

    char* message;
    char command[MAXSIZE],extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s\n",command,extra);

    if(strlen(extra) != 0){
        logOUT("Wrong size parameters.");
        return;
    }

    if(!strcmp(userID,"")){
        logOUT("No user is logged in.");
        return;
    }

    printf("Current UID:%s\n",userID);
}

char* parseSubscribe(char* input){

    char* message;
    char command[MAXSIZE], GID[MAXSIZE], GName[MAXSIZE], extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s %s %s\n", command, GID, GName, extra);

    if((strlen(extra) != 0) || ((strlen(command) != 9) && (strlen(command) != 1)) || (strlen(GID) > 2) || (strlen(GName) > 25)){
        logGSR("Wrong size parameters.");
        return NULL;

    } else if(!checkStringIsNumber(GID) || !checkStringIsGroupName(GName)){
        logGSR("Forbidden character in parameters.");
        return NULL;
    }

    if(!strcmp(userID,"")){
        logGSR("No user is logged in.");
        return NULL;
    }

    message = malloc(sizeof(char)*37);
    sprintf(message,"GSR %s %02d %s\n", userID, atoi(GID), GName);
    printf("%s",message);

    return message;
}

char* parseUnsubscribe(char* input){

    char* message;
    char command[MAXSIZE], GID[MAXSIZE], extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s %s\n", command, GID, extra);

    if((strlen(extra) != 0) || ((strlen(command) != 11) && (strlen(command) != 1)) || (strlen(GID) > 2)){
        logGSR("Wrong size parameters.");
        return NULL;

    } else if(!checkStringIsNumber(GID)){
        logGSR("Forbidden character in parameters.");
        return NULL;
    }

    if(!strcmp(userID,"")){
        logGSR("No user is logged in.");
        return NULL;
    }

    message = malloc(sizeof(char)*37);
    sprintf(message,"GUR %s %02d\n", userID, atoi(GID));

    return message;
}

char* parseMyGroups(char* input){

    char* message;
    char command[MAXSIZE],extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s\n",command,extra);

    if((strlen(extra) != 0) || ((strlen(input) != 4) && (strlen(input) != 10))){
        logOUT("Wrong size parameters.");
        return NULL;
    }

    if(!strcmp(userID,"")){
        logGSR("No user is logged in.");
        return NULL;
    }

    message = malloc(sizeof(char)*10);
    sprintf(message,"GLM %s\n", userID);

    return message;
}

void processSelect(char* input){

    char* message;
    char command[MAXSIZE], GID[MAXSIZE], extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s %s\n", command, GID, extra);

    if((strlen(extra) != 0) || ((strlen(command) != 6) && (strlen(command) != 3)) || (strlen(GID) > 2)){
        logOUT("Wrong size parameters.");
        return;

    } else if(!checkStringIsNumber(GID)){
        logOUT("Forbidden character in parameters.");
        return;
    } else if(!strcmp(GID, "0") || !strcmp(GID, "00")){
        logOUT("Group 0 doesn't exist");
        return;
    }

    if(!strcmp(userID,"")){
        logOUT("No user is logged in.");
        return;
    }

    strcpy(groupID, GID);
    printf("Current group selected: %s\n", groupID);
}

void processShowGID(char* input){

    char* message;
    char command[MAXSIZE],extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s\n",command,extra);

    if(strlen(extra) != 0){
        logOUT("Wrong size parameters.");
        return;
    }

    if(!strcmp(userID,"")){
        logOUT("No user is logged in.");
        return;
    }

    if(!strcmp(groupID,"")){
        logOUT("No group is selected.");
        return;
    }

    printf("Current GID:%s\n",groupID);
}


/**
 * Generic function to proccess commands that access the server via UDP protocol.
 * This function receives the user input and a set of function specific for each
 * command.
 * @param[in] input User input
 * @param[in] parser Function to parse the command
 * @param[in] logger Function to log the messages related to the command
 * @param[in] helper "Optional" function when processRequest needs to do additional tasks
 * 
*/
void processRequest(char* input, char* (*parser)(char*), void (*logger)(char*), void(*helper)(char*)){

    int fd;
    int msgSize;
    struct addrinfo *res;
    char *message, *response;

    message = (*parser)(input);
    if(message == NULL) return;

    printf("MESSAGE SENT:%s",message);

    UDPconnect(&fd,&res);
    UDPsendMessage(fd,res,message,strlen(message));
    response = UDPreceiveMessage(fd);

    printf("RESPONSE GIVEN:%s",response);

    if(helper != NULL){
        (*helper)(response);
    }

    (*logger)(response);

    free(message);
    free(response);
}

// Main Loop
void handleRequests(){

    char input[MAXSIZE],command[MAXSIZE],extra[MAXSIZE];

    while(TRUE){

        fgets(input, MAXSIZE, stdin);
        sscanf(input,"%s %s\n",command,extra);

        if(!strcmp(command,"reg")){
            processRequest(input, parseRegister, logREG, NULL);
            
        } else if(!strcmp(command,"unregister") || !strcmp(command,"unr")){
            processRequest(input, parseUnregister, logUNR, NULL);
            
        } else if(!strcmp(command,"login")){
            processRequest(input, parseLogin, logLOG, helperLogin);

        } else if(!strcmp(command,"logout")){
            processRequest(input, parseLogout, logOUT, helperLogout);
            
        } else if(!strcmp(command,"showuid") || !strcmp(command,"su")){
            processShowUID(input);

        } else if(!strcmp(command,"exit")){
            break;

        } else if(!strcmp(command,"groups") || !strcmp(command,"gl")){
            //processGroups();

        } else if(!strcmp(command,"subscribe") || !strcmp(command,"s")){
            processRequest(input, parseSubscribe, logGSR, NULL);

        } else if(!strcmp(command,"unsubscribe") || !strcmp(command,"u")){
            processRequest(input, parseUnsubscribe, logGUR, NULL);

        } else if(!strcmp(command,"my_groups") || !strcmp(command,"mgl")){
            processRequest(input, parseMyGroups, logGLM, NULL);

        } else if(!strcmp(command,"select") || !strcmp(command,"sag")){
            processSelect(input);
        
        } else if(!strcmp(command,"showgid") || !strcmp(command,"sg")){
            processShowGID(input);

        } else if(!strcmp(command,"ulist") || !strcmp(command,"ul")){
            //processUList();

        } else if(!strcmp(command,"post")){
            //processPost();

        } else if(!strcmp(command,"retrieve") || !strcmp(command,"r")){
            //processRetrieve();

        } else {
            logError("Command not found.");
        }
    }
}

// Main
int main(int argc, char *argv[]){
    
    parseArguments(argc,argv);
    handleRequests();
    //disconnect() gracefully;

    return 1;
}