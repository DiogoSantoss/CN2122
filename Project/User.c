#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

// Group Number
#define GN 27

// Default Port and IP address if none is given
int port = 5800+GN;
char* ipAddress = "guadiana";

// TODO: DOCUMENT
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
                if(optarg[0] != '-'){
                    port = atoi(optarg);
                }
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