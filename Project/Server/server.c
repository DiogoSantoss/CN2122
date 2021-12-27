#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#include "common.h"
#include "structs.h"

// Booleans
#define TRUE  1
#define FALSE 0

#define MAXSIZE 274
#define EXTRAMAXSIZE 3169

#define max(A,B) ((A)>=(B)?(A):(B))


void logError(char* message){
    printf("%s\n", message);
}





int runServerUDP(serverData server, int fd, struct addrinfo* res){

    int n;
    socklen_t addrlen;
    struct sockaddr_in addr;
    char* message  = calloc(EXTRAMAXSIZE,sizeof(char));
    char* response = calloc(EXTRAMAXSIZE,sizeof(char));

    n = bind(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1){
        logError("Couldn't bind server");
        return FALSE;
    }

    while (1){
        addrlen = sizeof(addr);
        memset(message, EXTRAMAXSIZE, sizeof(char));
        memset(response, EXTRAMAXSIZE, sizeof(char));
        n = recvfrom(fd,message,EXTRAMAXSIZE,0,(struct sockaddr*)&addr,&addrlen);
        printf("%d bytes received\n", n);
        if (n == -1){
            logError("Couldn't receive message via UDP socket");
            return FALSE;
        }
        printf("Received from: %s\n", message);
        strcpy(response, "RRG OK");
        n = sendto(fd, response, strlen(response), 0, res->ai_addr, res->ai_addrlen);
        if(n==-1){
            logError("Couldn't send message via UDP socket");
            return FALSE;
        }   
        printf("Sent to: %s", response);
    }

}

int serverUDP(int fd, struct addrinfo* res, char* message, int messageLen){
    int n;
    socklen_t addrlen;
    struct sockaddr_in addr;

    n = sendto(fd, message,messageLen,0,res->ai_addr,res->ai_addrlen);
    if(n==-1){
        logError("Couldn't send message via UDP socket");
        return FALSE;
    }   
    return TRUE;
}


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
    while((opt = getopt(argc, argv, "vp:")) != -1) 
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

int createServerSocketUDP(serverData *server, int* fd, struct addrinfo* res){
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

int createServerSocketTCP(serverData *server, int* fd, struct addrinfo* res){
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
    int fdUdp, fdTcp;
    int fdNew;
    int maxfd, counter;

    struct addrinfo *res;

    struct sockaddr_in addr; 
    socklen_t addrlen;


    enum {
        idle, 
        busy
    } state;


    if(!createServerSocketUDP(server,&fdUdp,res)) return;
    if(!createServerSocketTCP(server,&fdTcp,res)) return;


    state = idle;
    while(1){

        FD_ZERO(&rfds);
        FD_SET(fdTcp,&rfds);
        FD_SET(fdUdp,&rfds);

        maxfd = max(fdTcp, fdUdp) + 1;

        printf("Before here\n");

        // Which masks to use ? TIMEOUT , etc ?
        counter = select(
            maxfd, 
            &rfds, 
            (fd_set*)NULL, 
            (fd_set*)NULL, 
            (struct timeval *)NULL
        );

        printf("Here\n");

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
            char buffer[500];
            char* ptr;

            ptr = buffer;
            while (nRead != 0){
                nRead = read(fdNew, ptr, EXTRAMAXSIZE);
                if(nRead == -1){
                    logError("Couldn't receive message via TCP socket.");
                    break;
                }
                ptr += nRead;
            }
            printf("Message from TCP:\n");
            printf("%s",buffer);

            close(fdNew);
            //----------------------------------------------

        }

        if(FD_ISSET(fdUdp,&rfds)){

            printf("UDP\n");

            
            //----------------------------------------------
            int n;
            char buffer[500];
            n = recvfrom(fdUdp, buffer, 500, 0, (struct sockaddr*)&addr, &addrlen);
            if(n==-1){
                logError("Couldn't receive message via UDP socket");
                break;
            } 
            printf("Message from UDP:\n");
            printf("%d\n",addr.sin_addr.s_addr);
            printf("%d\n",addr.sin_port);
            printf("%s",buffer);
            
            n = sendto(fdUdp, buffer, strlen(buffer), 0, (struct sockaddr*)&addr, addrlen);
            if(n == -1){
                logError("Couldn't send message via UDP socket");
                break;
            }
            
            //----------------------------------------------

        }
    }

    //freeaddrinfo(res);
    close(fdTcp);
    close(fdUdp);
}



int main(int argc, char *argv[]){

    userData user;
    serverData server;
    


    initializeData(&user, &server);
    parseArguments(&server, argc, argv);
    handleRequests(&user, &server);

    /*
    connectServerUDP(&server, &fd, &res);
    runServerUDP(server, fd, res);
    */

    return 1;
};