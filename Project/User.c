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

// Global Variables
// Default Port
char* port = "58027";
// Default IP address
char* ipAddress = "guadiana.tecnico.ulisboa.pt";


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
                    ipAddress = optarg;
                }
                nCounter++;
                break;
            case 'p':
                if(!checkStringIsNumber(optarg)){
                    logError("Port value should be a positive integer.");
                    exit(1);
                }
                port = optarg;
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

    n = sendto(fd, message,messageLen,0,res->ai_addr,res->ai_addrlen);
    if(n==-1){
        logError("Couldn't send message via UDP socket");
        exit(1);
    }   
    
    char buffer[128];
    addrlen = sizeof(addr);
    n = recvfrom(fd,buffer,128,0,(struct sockaddr*)&addr,&addrlen);
    if(n==-1) exit(1);
    printf("%s\n", buffer);
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

void processRegister(){
    //parse()
    //connect()
    //send()
    //disconnect()
}

void handleRequests(){

    // Max input size is defined by the biggest post message possible
    // 4+1+240+2+1+24+1+1 = 274
    char buffer[274];
    char* p;

    while(1){
        fgets(buffer,274,stdin);
        p = strtok(buffer," ");

        if(!strcmp(p,"reg")){
            processRegister();
            
        } else if(!strcmp(p,"unregister") || !strcmp(p,"unr")){
            processUnregister();
            
        } else if(!strcmp(p,"login")){
            processLogin();

        } else if(!strcmp(p,"logout")){
            processLogout();
            
        } else if(!strcmp(p,"showuid") || !strcmp(p,"su")){
            processShowUID();

        } else if(!strcmp(p,"exit")){
            break;

        } else if(!strcmp(p,"groups") || !strcmp(p,"gl")){
            processGroups();

        } else if(!strcmp(p,"subscribe") || !strcmp(p,"s")){
            processSubscribe();

        } else if(!strcmp(p,"unsubscribe") || !strcmp(p,"u")){
            processUnsubscribe();

        } else if(!strcmp(p,"my_groups") || !strcmp(p,"mgl")){
            processMyGroups();

        } else if(!strcmp(p,"select") || !strcmp(p,"sag")){
            processSelect();
        
        } else if(!strcmp(p,"showgid") || !strcmp(p,"sg")){
            processShowGID();

        } else if(!strcmp(p,"ulist") || !strcmp(p,"ul")){
            processUList();

        } else if(!strcmp(p,"post")){
            processPost();

        } else if(!strcmp(p,"retrieve") || !strcmp(p,"r")){
            processRetrieve();

        } else {
            printf("ERR\n");
        }
    }
}


int main(int argc, char *argv[]){

    parseArguments(argc,argv);
    handleRequests();
    //disconnect() gracefully;

    return 1;
}