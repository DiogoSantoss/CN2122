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
char port[5] = "58011";
// Default IP address
char ipAddress[512] = "tejo.tecnico.ulisboa.pt";// input can be IP or Name
// User ID
char UserID[5] = "";
// Group ID
char GroupID[2] = "";


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
 * Check if String is a number
 * @param[in] value String to be checked
 * @param[out] IsNumber TRUE if is number else FALSE
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
    
    // Edge cases: (ask teacher)
    // ./User -n -p  -> Port: 5806 IP Address: -p
    // ./User -p -n  -> Port: 0    IP Address: guadiana

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

char* UDPreceiveMessage(int fd){
    int n;
    socklen_t addrlen;
    struct sockaddr_in addr;
    char* buffer = malloc(sizeof(char)*128);

    addrlen = sizeof(addr);
    n = recvfrom(fd,buffer,128,0,(struct sockaddr*)&addr,&addrlen);
    if(n==-1){
        logError("Couldn't receive message via UDP socket");
        exit(1);
    } 

    //printf("DEBUG SERVER MESSAGE:%s",buffer);
    return buffer;
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

    if(strcmp(UserID,"")){
        logLOG("A user is already logged in.");
        return NULL;

    } else {
        strcpy(UserID,UID);
    }

    message = malloc(sizeof(char)*18);
    sprintf(message,"LOG %s %s\n",UID,pass);

    return message;
}

void helperLogin(char* response){
    if(!strcmp(response,"RLO NOK\n")){
        strcpy(UserID,"");
    }
}

char* parseLogout(char* input){

    char* message;
    char command[MAXSIZE],UID[MAXSIZE],pass[MAXSIZE],extra[MAXSIZE];

    memset(extra,0,sizeof extra);
    sscanf(input,"%s %s %s %s\n",command,UID,pass,extra);

    if((strlen(extra) != 0) || (strlen(input) != 22) || (strlen(UID) != 5) || (strlen(pass) != 8)){
        logOUT("Wrong size parameters.");
        return NULL;

    } else if(!checkStringIsNumber(UID) || !checkStringIsAlphaNum(pass)){
        logOUT("Forbidden character in parameters.");
        return NULL;
    }

    if(!strcmp(UserID,"")){
        logOUT("No user is logged in.");
        return NULL;

    } else if (strcmp(UserID,UID)){
        logOUT("This user is not logged in.");
        return NULL;
    }

    message = malloc(sizeof(char)*18);
    sprintf(message,"OUT %s %s\n",UID,pass);

    return message;
}

void helperLogout(char* response){
    if(!strcmp(response,"ROU OK\n")){
        strcpy(UserID,"");
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

    if(!strcmp(UserID,"")){
        logOUT("No user is logged in.");
        return;
    }

    printf("Current UID:%s\n",UserID);
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

    if(!strcmp(GroupID,"")){
        logOUT("No group is selected.");
        return;
    }

    printf("Current UID:%s\n",GroupID);
}

void processRequest(char* input, int size, char* (*parser)(char*), void (*logger)(char*), void(*helper)(char*)){

    int fd;
    int msgSize;
    struct addrinfo *res;
    char *message, *response;

    message = (*parser)(input);
    if(message == NULL) return;

    UDPconnect(&fd,&res);
    UDPsendMessage(fd,res,message,size);
    response = UDPreceiveMessage(fd);

    if(helper != NULL){
        (*helper)(response);
    }

    (*logger)(response);

    free(message);
    free(response);
}

void handleRequests(){

    // Max input size is defined by the biggest post message possible
    // 4+1+240+2+1+24+1+1 = MAXSIZE
    char input[MAXSIZE],command[MAXSIZE],extra[MAXSIZE];

    while(TRUE){

        fgets(input, MAXSIZE, stdin);
        sscanf(input,"%s %s\n",command,extra);

        if(!strcmp(command,"reg")){
            processRequest(input, 19, parseRegister, logREG, NULL);
            
        } else if(!strcmp(command,"unregister") || !strcmp(command,"unr")){
            processRequest(input, 19, parseUnregister, logUNR, NULL);
            
        } else if(!strcmp(command,"login")){
            processRequest(input, 19, parseLogin, logLOG, helperLogin);

        } else if(!strcmp(command,"logout")){
            processRequest(input, 19, parseLogout, logOUT, helperLogout);
            
        } else if(!strcmp(command,"showuid") || !strcmp(command,"su")){
            processShowUID(input);

        } else if(!strcmp(command,"exit")){
            break;

        } else if(!strcmp(command,"groups") || !strcmp(command,"gl")){
            //processGroups();

        } else if(!strcmp(command,"subscribe") || !strcmp(command,"s")){
            //processSubscribe();

        } else if(!strcmp(command,"unsubscribe") || !strcmp(command,"u")){
            //processUnsubscribe();

        } else if(!strcmp(command,"my_groups") || !strcmp(command,"mgl")){
            //processMyGroups();

        } else if(!strcmp(command,"select") || !strcmp(command,"sag")){
            //processSelect();
        
        } else if(!strcmp(command,"showgid") || !strcmp(command,"sg")){
            processShowGID(input);

        } else if(!strcmp(command,"ulist") || !strcmp(command,"ul")){
            //processUList();

        } else if(!strcmp(command,"post")){
            //processPost();

        } else if(!strcmp(command,"retrieve") || !strcmp(command,"r")){
            //processRetrieve();

        } else {
            printf("ERROR\n");
        }
    }
}


int main(int argc, char *argv[]){

    parseArguments(argc,argv);
    handleRequests();
    //disconnect() gracefully;

    return 1;
}