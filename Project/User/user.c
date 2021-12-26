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
        sscanf(input,"%s %s\n",command,extra);

        if(!strcmp(command,"reg")){
            processRequestUDP(user, server, input, parseRegister, logREG, NULL);
            
        } else if(!strcmp(command,"unregister") || !strcmp(command,"unr")){
            processRequestUDP(user, server, input, parseUnregister, logUNR, NULL);
            
        } else if(!strcmp(command,"login")){
            processRequestUDP(user, server, input, parseLogin, logLOG, helperLogin);

        } else if(!strcmp(command,"logout")){
            processRequestUDP(user, server, input, parseLogout, logOUT, helperLogout);
            
        } else if(!strcmp(command,"showuid") || !strcmp(command,"su")){
            processShowUID(user, input);

        } else if(!strcmp(command,"exit")){
            break;

        } else if(!strcmp(command,"groups") || !strcmp(command,"gl")){
            processRequestUDP(user, server, input, parseGroups, logGLS, NULL);

        } else if(!strcmp(command,"subscribe") || !strcmp(command,"s")){
            processRequestUDP(user, server, input, parseSubscribe, logGSR, NULL);

        } else if(!strcmp(command,"unsubscribe") || !strcmp(command,"u")){
            processRequestUDP(user, server, input, parseUnsubscribe, logGUR, NULL);

        } else if(!strcmp(command,"my_groups") || !strcmp(command,"mgl")){
            processRequestUDP(user, server, input, parseMyGroups, logGLS, NULL);

        } else if(!strcmp(command,"select") || !strcmp(command,"sag")){
            processSelect(user, input);
        
        } else if(!strcmp(command,"showgid") || !strcmp(command,"sg")){
            processShowGID(user, input);

        } else if(!strcmp(command,"ulist") || !strcmp(command,"ul")){
            processUlist(user, server, input, parseUlist, logULS, NULL);

        } else if(!strcmp(command,"post")){
            processPost(user, server, input);

        } else if(!strcmp(command,"retrieve") || !strcmp(command,"r")){
            processRetrieve(user, server, input);

        } else {
            logError("Command not found.");
        }

        if(user->res != NULL){
            freeaddrinfo(user->res);
            close(user->fd);
            user->res = NULL;
        }
        colorReset();
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