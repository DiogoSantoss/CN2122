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

int DelLoginFile(char *UID){

    char pathname[50];

    sprintf(pathname,"USERS/%s/%s_login.txt",UID,UID);

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

int GroupExists(char* GID){

    DIR *d;
    struct dirent *dir;

    FILE *fp;

    int groupMax = 0;

    d = opendir("GROUPS");
    if (d)
    {
        while ((dir = readdir(d)))
        {
            if(strcmp(GID, dir->d_name) == 0)
            return TRUE;
        }
        return FALSE;
    }
    else
        return FALSE;

}

int latestGroup(){

    DIR *d;
    struct dirent *dir;

    FILE *fp;

    int groupMax = 0;

    d = opendir("GROUPS");
    if (d)
    {
        while ((dir = readdir(d)))
        {
            if (dir->d_name[0]=='.')
                continue;
            if(strlen(dir->d_name)>2)
                continue;
            if (atoi(dir->d_name) > groupMax)
                groupMax = atoi(dir->d_name);
        }
        return groupMax;
    }
    else
        return -1;
}

int CreateGroupDir(char *GID){

    char group_dirname[20];
    int ret;

    sprintf(group_dirname,"GROUPS/%s",GID);
    ret=mkdir(group_dirname,0700);

    if(ret==-1)
        return FALSE;

    return TRUE;
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

int checkLoginFile(char* UID){
    FILE* fptr;
    char path[32];

    sprintf(path, "USERS/%s/%s_login.txt", UID, UID);

    if(!(fptr = fopen(path, "r"))){
        if(!(fptr = fopen(path, "w"))){
            //Failed to open path
            return FALSE;
        }
        fclose(fptr);
        return TRUE;
    }

    fclose(fptr);
    return TRUE;
}

int CreateGroupFile(char* UID, char* password){

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

int SubscribeUser(char* UID, char* GID){

    FILE *fptr;
    char path[31];

    sprintf(path, "GROUPS/%s/%s.txt", GID, UID);

    printf("PATH: %s\n", path);

    if(!(fptr = fopen(path, "w"))){
        //Failed to open path
        return FALSE;
    }

    fwrite(UID, sizeof(char), 5, fptr);
    fclose(fptr);

    return TRUE;
}

int UserExistsInGroup(char* UID, char* GID){

    FILE *fptr;
    char path[31];
    DIR* dir;

    sprintf(path, "GROUPS/%s/%s.txt", GID, UID);

    if(dir = opendir(path)){
        // User exists in group
        return TRUE;
    }
    else if (ENOENT == errno) {
        // User doesnt exist in group
        return FALSE;
    } 
    else {
        //Unknown error
        return FALSE;
    }
}

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

        if(!checkLoginFile(UserID)) strcpy(message, "ERR\n");
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

        if (!DelLoginFile(UserID)) strcpy(message, "ERR\n");
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

char* processGSR(userData user, serverData server, char* request){

    char prefix[4], sufix[MAXSIZEUDP];
    char UserID[6], GroupID[3], GroupName[25];

    memset(sufix, 0, MAXSIZEUDP);
    char* message = calloc(13, sizeof(char));

    sscanf(request, "%s %s %s %s %s", prefix, UserID, GroupID, GroupName, sufix);


    int groupMax = latestGroup();

    if (strlen(sufix) == 0){

        if (strlen(UserID) != 5 || !checkStringIsAlphaNum(UserID) || !UserExists(UserID)){
            strcpy(message, "RGS E_USR\n");
            return message;
        }
        else if (strlen(GroupName) > 24 || !checkStringIsAlphaNum(GroupName)){
            strcpy(message, "RGS E_GNAME\n");
            return message;
        }
        // Existing group is subscribed
        else if (strlen(GroupID) == 2 && checkStringIsNumber(GroupID)){
            if (strcmp(GroupID, "00") == 0){
                if (groupMax >= 99){
                    strcpy(message, "RGS E_FULL\n");
                    return message;
                }
                char newGroup[3];
                sprintf(newGroup, "%02d", groupMax + 1);
                CreateGroupDir(newGroup);
                if(!SubscribeUser(UserID, newGroup)){
                    strcpy(message, "RGS NOK\n");
                    return message;
                }

                sprintf(message, "RGS NEW %s\n", newGroup);
                return message;
            }
            else{
                // TODO - check if user is already subscribed to that group? (not sure) - use funcition UserExistsInGroup
                // In the tejo server, this is not verified, the user just happens to subscribe again to the group
                if(!SubscribeUser(UserID, GroupID)){
                    strcpy(message, "RGS NOK\n");
                    return message;
                }
                strcpy(message, "RGS OK\n");
                return message;
            }
        }
        // New group is created
        else{
            strcpy(message, "RGS NOK\n");
            return message;
        }
    }
    else if (!checkStringIsNumber(UserID) || !checkStringIsAlphaNum(GroupID) || !checkStringIsAlphaNum(GroupName)){
        // Forbidden character in parameters
        strcpy(message, "ROU NOK\n");
        return message;
    }
}