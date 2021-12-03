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

// Group Number
#define GN 27

// Global Variables
// Default Port
int port = 5800+GN;
// Default IP address
char* ipAddress = "guadiana";


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
    
    //TODO: PLS DOCUMENT
    // Edge cases: (ask teacher)
    // ./User -n -p  -> Port: 5806 IP Address: -p
    // ./User -p -n  -> Port: 0    IP Address: guadiana
    // if you want to learn more there is always google at hand
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
                for(int i = 0; i<strlen(optarg); i++){
                    if(!isdigit(optarg[i])){
                        fprintf(stderr, "Port number should be a number.\n");
                        exit(1);
                    }
                } 
                port = atoi(optarg);
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
    printf("Port:%d\nIP Address:%s\n",port,ipAddress);
}


int main(int argc, char *argv[]){

    parseArguments(argc,argv);

    return 1;
}