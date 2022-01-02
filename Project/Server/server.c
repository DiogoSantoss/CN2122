#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>

#include "common.h"
#include "log.h"
#include "structs.h"
#include "requestsUDP.h"

// Booleans
#define TRUE  1
#define FALSE 0

#define MAXSIZEUDP 39 //GSR is the biggest command 
#define EXTRAMAXSIZE 3169

#define max(A,B) ((A)>=(B)?(A):(B))

/**
 * Initialize data about user and server
 * @param[in] user User data
 * @param[in] server Server data
*/
void initializeData(userData *user, serverData *server){
    strcpy(user->port,"");
    strcpy(user->ipAddress,"");

    server->verbose = FALSE;
    strcpy(server->port,"58011");
}

/**
 * Parse command-line arguments
 * Only two available arguments:
 *  -p PORT where PORT is the port number of the machine where the server runs
 *  -v which activates verbose mode
 * @param[in] argc Number of elements in argv
 * @param[in] argv Array of command-line arguments
*/
void parseArguments(serverData *server, int argc, char *argv[]){

    char opt;
    int vCounter = 0;
    int pCounter = 0;
    
    // the getopt function parses command-line arguments
    // and returns the flag , eg: "-p" -> opt = 'p'
    // also has external variables such as optarg that stores
    // the flag argument, eg: "-p 58011" -> optarg = "58011"
    while((opt = getopt(argc, argv, "v:p:")) != -1) 
    { 
        switch(opt) 
        { 
            case 'p':
                if(!checkStringIsNumber(optarg)){
                    logError("Port value should be a positive integer.");
                    exit(1);
                }
                strcpy(server->port,optarg);
                pCounter++;
                break;  
            case 'v':
                server->verbose = TRUE;
                vCounter++;
                break;
            default: 
                logError("Wrong arguments.");
                exit(1);
        }
        if(pCounter > 1 || vCounter > 1){ 
            logError("Repeated command-line arguments.");
            exit(1);
        }
    }
}

int createSocketUDP(serverData *server, int* fd, struct addrinfo* res){
    int errcode;
    struct addrinfo hints;

    *fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(*fd==-1){
        logError("Couldn't create UDP socket.");
        return FALSE;
    }

    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
    hints.ai_flags=AI_PASSIVE;

    errcode=getaddrinfo(NULL,server->port,&hints,&res);
    if(errcode!=0){
        logError("Couldn't get address info.");
        return FALSE;
    }

    if(bind(*fd,res->ai_addr,res->ai_addrlen) == -1){
        logError("Couldn't bind UDP socket to server address.");
        return FALSE;
    }

    return TRUE;
}

int createSocketTCP(serverData *server, int* fd, struct addrinfo* res){
    int errcode;
    struct addrinfo hints;

    *fd = socket(AF_INET, SOCK_STREAM, 0);
    if(*fd==-1){
        logError("Couldn't create TCP socket.");
        return FALSE;
    }

    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_PASSIVE;

    errcode=getaddrinfo(NULL,server->port,&hints,&res);
    if(errcode!=0){
        logError("Couldn't get address info.");
        return FALSE;
    }

    if(bind(*fd,res->ai_addr,res->ai_addrlen) == -1){
        logError("Couldn't bind TCP socket to server address.");
        return FALSE;
    }

    // IS 5 ENOUGH ?
    if(listen(*fd, 5) == -1){
        logError("Couldn't prepare socket to accept connections.");
        return FALSE;
    }

    return TRUE;
}

void handleRequests(userData* user, serverData* server){

    fd_set rfds;
    int fdUdp, fdTcp; // File descriptors for udp and tcp connections
    int fdNew;        // File descriptor for tcp client connection
    int maxfd, counter;

    struct addrinfo *res;

    struct sockaddr_in addr; 
    socklen_t addrlen;

    //enum {
    //    idle, 
    //    busy
    //} state;

    if(!createSocketUDP(server,&fdUdp,res)) return;
    if(!createSocketTCP(server,&fdTcp,res)) return;

    //state = idle;
    while(1){

        FD_ZERO(&rfds);
        FD_SET(fdTcp,&rfds);
        FD_SET(fdUdp,&rfds);

        maxfd = max(fdTcp, fdUdp) + 1;

        // Which masks to use ? TIMEOUT , etc ?
        counter = select(
            maxfd, 
            &rfds, 
            (fd_set*)NULL, 
            (fd_set*)NULL, 
            (struct timeval *)NULL
        );

        if(counter == -1){
            logError("Select error.");
            break;
        }

        if(FD_ISSET(fdTcp,&rfds)){

            printf("TCP\n");

            addrlen=sizeof(addr);
            if((fdNew=accept(fdTcp,(struct sockaddr*)&addr,&addrlen))==-1){
                logError("Couldn't accept connection.");
                break;
            }

            //----------------------------------------------
            int nRead = -1;
            char request[500];
            char* ptr;

            ptr = request;
            while (nRead != 0){
                nRead = read(fdNew, ptr, EXTRAMAXSIZE);
                if(nRead == -1){
                    logError("Couldn't receive message via TCP socket.");
                    break;
                }
                ptr += nRead;
            }
            printf("Message from TCP:\n");
            printf("%s",request);

            close(fdNew);
            //----------------------------------------------

        }

        if(FD_ISSET(fdUdp,&rfds)){

            // RECEIVE UDP MESSAGE

            ssize_t n;
            char request[MAXSIZEUDP],command[MAXSIZEUDP],extra[MAXSIZEUDP];
            char* response;
            memset(request, 0, MAXSIZEUDP);
            addrlen=sizeof(addr);
            n = recvfrom(fdUdp, request, MAXSIZEUDP, 0, (struct sockaddr*)&addr, &addrlen);
            if(n==-1){
                logError("Couldn't receive message via UDP socket");
                break;
            } 
            printf("Message from UDP:\n");
            printf("%d\n",addr.sin_addr.s_addr);
            printf("%d\n",addr.sin_port);
            printf("Message:%s",request);

            if(request[strlen(request)-1] != '\n'){
                logError("Client message doesn't end with \\n.");
                //sendErrorMessage();
                continue;
            }

            sscanf(request,"%s %s",command,extra);

            if(!strcmp(command,"REG")){
                response = processREG(*user, *server, request);

            } else if(!strcmp(command,"UNR")){
                response = processURN(*user, *server, request);

            } else if(!strcmp(command,"LOG")){
                response = processLOG(*user, *server, request);

            } else if(!strcmp(command,"OUT")){
                response = processOUT(*user, *server, request);

            } else if(!strcmp(command,"GLS")){
                response = processGLS(*user, *server, request);

            } else if(!strcmp(command,"GSR")){
                response = processGSR(*user, *server, request);

            } else if(!strcmp(command,"GUR")){
                response = processGUR(*user, *server, request);
                
            } else if(!strcmp(command,"GLM")){

            } else{
                logError("Command not found.");
                //sendErrorMessage();
            }

            // STRCMPS

            //REG UID pass\n
            //UNR UID pass\n
            //LOG UID pass\n
            //OUT UID pass\n
            //GLS\n
            //GSR UID GID GName\n
            //GUR UID GID\n
            //GLM UID\n

            printf("Response:%s",response);
            printf("-------------------------\n");
            //int manuel = strlen(response);
            
            n = sendto(fdUdp, response, strlen(response), 0, (struct sockaddr*)&addr, addrlen);
            if(n == -1){
                printf("sendto: Error %s (%d)\n", strerror(errno), errno);
                logError("Couldn't send message via UDP socket");
                break;
            }
            free(response);
        }
    }

    //freeaddrinfo(res);
    close(fdTcp);
    close(fdUdp);
}



int main(int argc, char *argv[]){

    userData user;
    serverData server;
    
    createDirectories();
    initializeData(&user, &server);
    parseArguments(&server, argc, argv);
    handleRequests(&user, &server);

    return 1;
};