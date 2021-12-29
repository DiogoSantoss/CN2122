#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "log.h"
#include "structs.h"
#include "common.h"

// Booleans
#define TRUE  1
#define FALSE 0

#define MAXSIZEUDP 39 //GSR is the biggest command 
#define EXTRAMAXSIZE 3169

char* processREG(userData user, serverData server, int fd, char* request){

    char prefix[4], sufix[MAXSIZEUDP];
    char UserID[6], password[9];

    char* message = calloc(9, sizeof(char));

    sscanf(request, "%s %s %s %s", prefix, UserID, password, sufix);

    if (strlen(sufix) != 0 || strlen(UserID) != 5 || strlen(password) != 8){
        //logError("Wrong size parameters.");
        strcpy(message, "RRG NOK\n");
        return NULL;
    }
    else if (!checkStringIsNumber(UserID) || !checkStringIsAlphaNum(password)){
        //logError("Forbidden character in parameters.");
        strcpy(message, "RRG NOK\n");
        return NULL;
    }
    strcpy(message, "RRG OK\n");
    return message;
}