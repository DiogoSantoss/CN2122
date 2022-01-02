#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "log.h"
#include "structs.h"

#define TRUE 1
#define FALSE 0

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

// Create user directory in USERS
int CreateUserDir(char *UID){

    char user_dirname[20];
    int ret;

    sprintf(user_dirname,"USERS/%s",UID);
    ret=mkdir(user_dirname,0700);

    if(ret==-1)
        return FALSE;

    return TRUE;
}

// Delete user directory in USERS
int DelUserDir(char *UID){

    char user_dirname[20];

    sprintf(user_dirname,"USERS/%s",UID);

    if(rmdir(user_dirname)==0)
        return TRUE;
    else
        return FALSE;
}

// Check if user exists
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

// Create pass file in user directory
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

// Delete pass file in user directory
int DelPassFile(char *UID){

    char pathname[50];

    sprintf(pathname,"USERS/%s/%s_pass.txt",UID,UID);

    if(unlink(pathname)==0)
        return TRUE;
    else
        return FALSE;
}

// Check if password is correct
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
    fclose(fptr);

    if(!strcmp(userPassword,password))
        return TRUE;
    else
        return FALSE;
}

// Create login file in user directory
int createLoginFile(char* UID){

    FILE* fptr;
    char path[32];

    sprintf(path, "USERS/%s/%s_login.txt", UID, UID);

    if(!(fptr = fopen(path, "w"))){
        //Failed to open path
        return FALSE;
    }

    fclose(fptr);
    return TRUE;
}

// Delete login file in user directory
int DelLoginFile(char *UID){

    char pathname[50];

    sprintf(pathname,"USERS/%s/%s_login.txt",UID,UID);

    if(unlink(pathname)==0)
        return TRUE;
    else
        return FALSE;
}

// Return biggest group number
int maxGroupNumber(){

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

// Create group directory in GROUPS and MSG directory inside
int CreateGroupDir(char *GID){

    char group_dirname[20];
    int ret;

    sprintf(group_dirname,"GROUPS/%s",GID);
    ret=mkdir(group_dirname,0700);

    if(ret==-1)
        return FALSE;

    sprintf(group_dirname,"GROUPS/%s/MSG",GID);
    ret=mkdir(group_dirname,0700);

    if(ret==-1)
        return FALSE;

    return TRUE;
}

int GroupLastMessage(char *GID){

    DIR *d;
    int messageNumber ,max = 0;
    struct dirent *dir;
    char path[14];

    sprintf(path,"GROUPS/%s/MSG",GID);
    d = opendir(path);

    if(d){
        while((dir = readdir(d)) != NULL){
            if(dir->d_name[0] == '.' || strlen(dir->d_name) > 4)
                continue;

            messageNumber = atoi(dir->d_name);
            if(messageNumber > max)
                max = messageNumber;
        }

        return max;
    } 
    else
        return (-1);
    
}

//Fill GROUPSLIST struct with all groups info
int ListGroupsDir(GROUPLIST *list){
    
    DIR *d;
    struct dirent *dir;
    int i=0;
    int lastMessage;
    FILE *fp;
    char GIDname[30];

    list->no_groups = 0;
    d = opendir("GROUPS");
    if(d){
        while((dir = readdir(d)) != NULL){
            if(dir->d_name[0] == '.')
                continue;
            if(strlen(dir->d_name)>2)
                continue;

            strcpy(list->group_no[i], dir->d_name);
            sprintf(GIDname, "GROUPS/%s/%s_name.txt", dir->d_name, dir->d_name);
            
            fp = fopen(GIDname, "r");
            if(fp){
                fscanf(fp,"%24s", list->group_name[i]);
                fclose(fp);
            }
            lastMessage = GroupLastMessage(dir->d_name);
            if(lastMessage != -1)
                sprintf(list->group_lastMens[i], "%04d", lastMessage);
            else
                return(-1);

            ++i;
            if(i == 99)
                break;
        }
        list->no_groups = i;
        closedir(d);
    }
    else
        return(-1);
        
    if(list->no_groups>1){
        //SortGList(list);
    }   

    return(list->no_groups);
}

// Create group file in group directory
int CreateGroupFile(char* GID, char* Gname){

    FILE *fptr;
    char path[31];

    sprintf(path, "GROUPS/%s/%s_name.txt", GID, GID);

    if(!(fptr = fopen(path, "w"))){
        //Failed to open path
        return FALSE;
    }
  
    fwrite(Gname, sizeof(char), strlen(Gname), fptr);
    fclose(fptr);

    return TRUE;
}

// Check if group exists
int GroupExists(char* GID){

    DIR *d;
    struct dirent *dir;

    FILE *fp;

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

// Check if group name is correct
int checkGroupName(char* GID, char* GName){

    FILE* fptr;
    char path[50];
    char groupName[25];

    sprintf(path, "GROUPS/%s/%s_name.txt", GID, GID);

    if(!(fptr = fopen(path, "r"))){
        //Failed to open path
        return FALSE;
    }

    memset(groupName, 0, 25);

    fread(groupName, sizeof(char), 25, fptr);
    fclose(fptr);

    if(!strcmp(groupName, GName)){
        return TRUE;
    }
    else{
        return FALSE;
    }
}

// Add user to subscribed users of group
int SubscribeUser(char* UID, char* GID){

    FILE *fptr;
    char path[31];

    sprintf(path, "GROUPS/%s/%s.txt", GID, UID);

    if(!(fptr = fopen(path, "w"))){
        //Failed to open path
        return FALSE;
    }

    fclose(fptr);

    return TRUE;
}

// Remove user from group
int UnsubscribeUser(char* UID, char* GID){

    char pathname[50];
    sprintf(pathname,"GROUPS/%s/%s.txt",GID,UID);

    if(unlink(pathname)==0)
        // Success
        return TRUE;
    else
        // Failed to unsubcribe user
        return FALSE;
}

// Check if user is subscribe to group
int checkUserSubscribedToGroup(char* UID, char* GID){

    FILE *fptr;
    char path[31];
    DIR* dir;

    sprintf(path, "GROUPS/%s/%s.txt", GID, UID);

    if(!(fptr = fopen(path, "r"))){
        //Failed to open path
        return FALSE;
    }
    fclose(fptr);
    return TRUE;
}