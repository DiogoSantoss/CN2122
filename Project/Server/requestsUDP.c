#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>

#include "log.h"
#include "structs.h"
#include "common.h"

#include <sys/stat.h>
#include <sys/types.h>

int mkdir(const char *pathname, mode_t mode);

// Booleans
#define TRUE  1
#define FALSE 0

#define MAXSIZEUDP 39 //GSR is the biggest command 
#define EXTRAMAXSIZE 3169

int CreateUserDir(char *UID){

    char user_dirname[20];
    int ret;

    sprintf(user_dirname,"Users/%s",UID);
    ret=mkdir(user_dirname,0700);

    if(ret==-1)
        return FALSE;

    return TRUE;
}

int CreateUserPassword(char* UID, char* password){

    FILE *fptr;
    char path[31];
    sprintf(path, "Users/%s/%s_password.txt", UID, UID);

    if(!(fptr = fopen(path, "w"))){
        //Failed to open path
        return FALSE;
    }

    fwrite(password, sizeof(char), 8, fptr);
    fclose(fptr);

    return TRUE;
}

int UserExists(char* UID){

    char path[31];
    DIR* dir;
    sprintf(path, "Users/%s", UID);
    if(dir = opendir(path)){
        // User exists
        return TRUE;
    }
    else if (ENOENT == errno) {
        //Directory doesnt exist.
        return FALSE;
    } 
    else {
        //Unknown error
        return FALSE;
    }
}


char* processREG(userData user, serverData server, int fd, char* request){

    char prefix[4], sufix[MAXSIZEUDP];
    char UserID[6], password[9];

    char* message = calloc(9, sizeof(char));

    sscanf(request, "%s %s %s %s", prefix, UserID, password, sufix);

    if (strlen(sufix) != 0 || strlen(UserID) != 5 || strlen(password) != 8){
        //Wrong size parameters
        strcpy(message, "RRG NOK\n");
        return NULL;
    }
    else if (!checkStringIsNumber(UserID) || !checkStringIsAlphaNum(password)){
        //Forbidden character in parameters
        strcpy(message, "RRG NOK\n");
        return NULL;
    }
    else if (UserExists(UserID)){
        // User exists
        strcpy(message, "RRG DUP");
    }

    if (!CreateUserDir(UserID)) strcpy(message, "ERR");
    if (!CreateUserPassword(UserID, password)) strcpy(message, "ERR");

    strcpy(message, "RRG OK\n");
    return message;
}