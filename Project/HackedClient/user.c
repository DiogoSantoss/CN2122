#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#include "log.h"
#include "colors.h"
#include "common.h"
#include "structs.h"
#include "requestsUDP.h"
#include "requestsTCP.h"

// Constants
#define MAXSIZE 274

/**
 * Initialize data about user and server
 * @param[in] user User data
 * @param[in] server Server data
*/
void initializeData(userData *user, serverData *server){
    strcpy(user->ID,"");
    strcpy(user->password,"");
    strcpy(user->groupID,"");
    user->res = NULL;

    strcpy(server->ipAddress,"tejo.tecnico.ulisboa.pt");
    strcpy(server->port,"58011");
}

/**
 * Parse command-line arguments
 * Only two available arguments:
 *  -n DSIP where DSIP is the IP address of the machine where the server runs
 *  -p DSport where DSport is the well-known port where server accepts requests
 * @param[in] argc Number of elements in argv
 * @param[in] argv Array of command-line arguments
*/
void parseArguments(serverData *server, int argc, char *argv[]){

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
                    strcpy(server->ipAddress,optarg);
                }
                nCounter++;
                break;
            case 'p':
                if(!checkStringIsNumber(optarg)){
                    logError("Port value should be a positive integer.");
                    exit(1);
                }
                strcpy(server->port,optarg);
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
}

/**
 * Main loop where user's commands are processed
 * @param[in] user User data
 * @param[in] server Server data
*/
void handleRequests(userData *user, serverData *server){

    char input[MAXSIZE],command[MAXSIZE],extra[MAXSIZE];

    while(1){

        fgets(input, MAXSIZE, stdin);

        int msgSize;
        char *response;

        if(!connectUDP(server, &(user->fd), &(user->res))) return;
        if(!sendMessageUDP(user->fd, user->res, input, strlen(input))) return;
        response = receiveMessageUDP(user->fd);
        if(response == NULL) return;

        colorYellow();
        printf("%s", response);
        colorReset();
        
        free(response);
    }
}

int main(int argc, char *argv[]){
    
    userData user;
    serverData server;

    initializeData(&user, &server);
    parseArguments(&server, argc, argv);
    handleRequests(&user, &server);

    return 1;
}