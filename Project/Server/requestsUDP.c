#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <dirent.h>

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

/* Treatment of directories and files */

//Create initial USERS and GROUPS directories
void createDirectories(){
    
    char users[6];
    char groups[7]; 
    int retUsers, retGroups;

    strcpy(users, "USERS");
    strcpy(groups, "GROUPS");

    DIR* dir = opendir("USERS");
    if(dir){
        //Do nothing
        closedir(dir);
    }
    else if(ENOENT == errno){
        retUsers = mkdir(users, 0700);
        if(retUsers == -1){
            logError("Couldn't create USERS directory.");
            exit(1);
        }
    }
    else{
        logError("Directory USERS failed to open.");
        exit(1);
    }

    dir = opendir("GROUPS");
    if(dir){
        //Do nothing
        closedir(dir);
    }
    else if(ENOENT == errno){
        retGroups = mkdir(groups, 0700);
        if(retGroups == -1){
            logError("Couldn't create GROUPS directory.");
            exit(1);
        }
    }
    else{
        logError("Directory USERS failed to open.");
        exit(1);
    }
}

int CreateUserDir(char *UID){

    char user_dirname[20];
    int ret;

    sprintf(user_dirname,"USERS/%s",UID);
    ret=mkdir(user_dirname,0700);

    if(ret==-1)
        return FALSE;

    return TRUE;
}

int DelUserDir(char *UID){

    char user_dirname[20];

    sprintf(user_dirname,"USERS/%s",UID);

    if(rmdir(user_dirname)==0)
        return TRUE;
    else
        return FALSE;
}

int CreatePassFile(char* UID, char* password){

    FILE *fptr;
    char path[31];

    sprintf(path, "USERS/%s/%s_pass.txt", UID, UID);

    if(!(fptr = fopen(path, "w"))){
        //Failed to open path
        return FALSE;
    }

    fwrite(password, sizeof(char), 8, fptr);
    fclose(fptr);

    return TRUE;
}

int DelPassFile(char *UID){

    char pathname[50];

    sprintf(pathname,"USERS/%s/%s_pass.txt",UID,UID);

    if(unlink(pathname)==0)
        return TRUE;
    else
        return FALSE;
}

int UserExists(char* UID){

    char path[31];
    DIR* dir;

    sprintf(path, "USERS/%s", UID);

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

int checkUserPassword(char* UID, char* password){

    FILE *fptr;
    char path[31];
    char userPassword[9]; // 8+1

    sprintf(path, "USERS/%s/%s_pass.txt", UID, UID);

    if(!(fptr = fopen(path, "r"))){
        //Failed to open path
        return FALSE;
    }

    fread(userPassword, sizeof(char), 9, fptr);
    printf("userPassword:%s\n",userPassword);
    printf("password:%s\n",password);
    fclose(fptr);

    if(!strcmp(userPassword,password))
        return TRUE;
    else
        return FALSE;
}
/*
int ListGroupsDir(GROUPLIST *list){

    DIR *d;
    struct dirent *dir;
    int i=0;
    FILE *fp;
    char GIDname[30];

    list->no_groups=0;

    d = opendir("GROUPS");
    if (d){
        while ((dir = readdir(d)) != NULL){

            if(dir->d_name[0]=='.')
                continue;
            if(strlen(dir->d_name)>2)
                continue;

            strcpy(list->group_no[i], dir->d_name);
            sprintf(GIDname,"GROUPS/%s/%s_name.txt",dir->d_name,dir->d_name);

            fp=fopen(GIDname,"r");

            if(fp){
                fscanf(fp,"%24s",list->group_name[i]);
                fclose(fp);
            }
            ++i;

            if(i==99)
                break;
        }

        list->no_groups=i;
        closedir(d);

    } else
        return(-1);

    if(list->no_groups > 1)
        SortGList(list);

    return(list->no_groups);
}
*/


/* Treatment of Requests */

/**
 * Process register request.
 * @param[in] user User data
 * @param[in] request Client input to be parsed
 * @param[out] message Formarted message to respond to client
*/
char* processREG(userData user, serverData server, char* request){

    char prefix[4], sufix[MAXSIZEUDP];
    char UserID[6], password[9];

    memset(sufix, 0, MAXSIZEUDP);
    char* message = calloc(9, sizeof(char));

    sscanf(request, "%s %s %s %s", prefix, UserID, password, sufix);

    printf("Request: %s", request);

    if (strlen(sufix) != 0 || strlen(UserID) != 5 || strlen(password) != 8){
        // Wrong size parameters
        //printf("sizesufix: %d\nsizeUID: %d\nsizepassword: %d\n", strlen(sufix), strlen(UserID), strlen(password));
        //printf("sufix: %s\n", sufix);
        strcpy(message, "RRG NOK\n");
        return message;
    }
    else if (!checkStringIsNumber(UserID) || !checkStringIsAlphaNum(password)){
        // Forbidden character in parameters
        strcpy(message, "RRG NOK\n");
        return message;
    }
    else if (UserExists(UserID)){
        // User exists
        strcpy(message, "RRG DUP\n");
        return message;
    }
    else{
        // Everything ok
        strcpy(message, "RRG OK\n");

        if (!CreateUserDir(UserID)) strcpy(message, "ERR\n");
        if (!CreatePassFile(UserID, password)) strcpy(message, "ERR\n");
    }

    return message;
}

/**
 * Process unregister request.
 * @param[in] user User data
 * @param[in] request Client input to be parsed
 * @param[out] message Formarted message to respond to client
*/
char* processURN(userData user, serverData server, char* request){

    char prefix[4], sufix[MAXSIZEUDP];
    char UserID[6], password[9];

    memset(sufix, 0, MAXSIZEUDP);
    char* message = calloc(9, sizeof(char));

    sscanf(request, "%s %s %s %s", prefix, UserID, password, sufix);

    if (strlen(sufix) != 0 || strlen(UserID) != 5 || strlen(password) != 8){
        // Wrong size parameters
        strcpy(message, "RUN NOK\n");
        return message;
    }
    else if (!checkStringIsNumber(UserID) || !checkStringIsAlphaNum(password)){
        // Forbidden character in parameters
        strcpy(message, "RUN NOK\n");
        return message;
    }
    else if (!UserExists(UserID) || !checkUserPassword(UserID,password)){
        // User doesn't exists or wrong password
        strcpy(message, "RUN NOK\n");
        return message;
    }
    else{
        // Everything ok
        strcpy(message, "RUN OK\n");

        if (!DelPassFile(UserID)) strcpy(message, "ERR\n");
        if (!DelUserDir(UserID)) strcpy(message, "ERR\n");
    }

    return message;
}

/**
 * Process login request.
 * @param[in] user User data
 * @param[in] request Client input to be parsed
 * @param[out] message Formarted message to respond to client
*/
char* processLOG(userData user, serverData server, char* request){

    char prefix[4], sufix[MAXSIZEUDP];
    char UserID[6], password[9];

    memset(sufix, 0, MAXSIZEUDP);
    char* message = calloc(9, sizeof(char));

    sscanf(request, "%s %s %s %s", prefix, UserID, password, sufix);

    if (strlen(sufix) != 0 || strlen(UserID) != 5 || strlen(password) != 8){
        // Wrong size parameters
        strcpy(message, "RLO NOK\n");
        return message;
    }
    else if (!checkStringIsNumber(UserID) || !checkStringIsAlphaNum(password)){
        // Forbidden character in parameters
        strcpy(message, "RLO NOK\n");
        return message;
    }
    else if (!UserExists(UserID) || !checkUserPassword(UserID,password)){
        // User doesn't exists or wrong password
        strcpy(message, "RLO NOK\n");
        return message;
    }
    else{
        // Everything ok
        strcpy(message, "RLO OK\n");
    }

    return message;
}

/**
 * Process logout request.
 * @param[in] user User data
 * @param[in] request Client input to be parsed
 * @param[out] message Formarted message to respond to client
*/
char* processOUT(userData user, serverData server, char* request){

    char prefix[4], sufix[MAXSIZEUDP];
    char UserID[6], password[9];

    memset(sufix, 0, MAXSIZEUDP);
    char* message = calloc(9, sizeof(char));

    sscanf(request, "%s %s %s %s", prefix, UserID, password, sufix);

    if (strlen(sufix) != 0 || strlen(UserID) != 5 || strlen(password) != 8){
        // Wrong size parameters
        strcpy(message, "ROU NOK\n");
        return message;
    }
    else if (!checkStringIsNumber(UserID) || !checkStringIsAlphaNum(password)){
        // Forbidden character in parameters
        strcpy(message, "ROU NOK\n");
        return message;
    }
    else if (!UserExists(UserID) || !checkUserPassword(UserID,password)){
        // User doesn't exists or wrong password
        strcpy(message, "ROU NOK\n");
        return message;
    }
    else{
        // Everything ok
        strcpy(message, "ROU OK\n");
    }

    return message;
}

/**
 * Process groups request.
 * @param[in] user User data
 * @param[in] request Client input to be parsed
 * @param[out] message Formarted message to respond to client
*/
char* processGLS(userData user, serverData server, char* request){



    
}