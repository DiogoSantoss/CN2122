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


// Booleans
#define TRUE 1
#define FALSE 0

// Global Variables
// Default Port
char* port = "5827";
// Default IP address
char* ipAddress = "guadiana";


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
        printf("Invalid number of arguments\n");
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
                    fprintf(stderr, "Port value should be a number.\n");
                    exit(1);
                }
                port = optarg;
                pCounter++;
                break;  
            default: 
                fprintf(stderr, "Wrong arguments\n");
                exit(1);
        }
        if(nCounter > 1 || pCounter > 1){ 
            fprintf(stderr, "Repeated arguments\n");
            exit(1);
        }
    }
    printf("Port:%s\nIP Address:%s\n",port,ipAddress);
}

void UDPconnect(int* fd, struct addrinfo *res){
    int errcode;
    struct addrinfo hints;

    *fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(*fd==-1){
        fprintf(stderr, "Couldn't create socket\n");
        exit(1);
    }

    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;

    errcode=getaddrinfo(ipAddress,port,&hints,&res) ;
    if(errcode!=0){
        fprintf(stderr, "Couldn't get server IP\n");
        exit(1);
    }
}

void TCPconnect(int* fd, struct addrinfo *res){
    int errcode;
    struct addrinfo hints;

    *fd = socket(AF_INET, SOCK_STREAM, 0);
    if(*fd==-1){
        fprintf(stderr, "Couldn't create socket\n");
        exit(1);
    }

    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_STREAM;

    errcode=getaddrinfo(ipAddress,port,&hints,&res) ;
    if(errcode!=0){
        fprintf(stderr, "Couldn't get server IP\n");
        exit(1);
    }
}

void processInputs(){

    int fd;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo *res;
    struct sockaddr_in addr;
    char buffer[128];

    UDPconnect(&fd, res);
    
}

int main(int argc, char *argv[]){

    parseArguments(argc,argv);

    processInputs();

    return 1;
}