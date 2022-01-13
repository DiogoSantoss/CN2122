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

#define MAXGROUPS 99

/**
 * Create initial USERS and GROUPS directories.
*/
void createDirectories(){

    DIR* dir;
    char users[6];
    char groups[7]; 

    strcpy(users, "USERS");
    strcpy(groups, "GROUPS");

    if(dir = opendir("USERS")){
        //Do nothing
        closedir(dir);
    }
    else if(ENOENT == errno){
        if(mkdir(users, 0700) == -1){
            logError(TRUE, "Couldn't create USERS directory.");
            exit(1);
        }
    }
    else{
        logError(TRUE, "Directory USERS failed to open.");
        exit(1);
    }

    if(dir = opendir("GROUPS")){
        //Do nothing
        closedir(dir);
    }
    else if(ENOENT == errno){
        if(mkdir(groups, 0700) == -1){
            logError(TRUE, "Couldn't create GROUPS directory.");
            exit(1);
        }
    }
    else{
        logError(TRUE, "Directory USERS failed to open.");
        exit(1);
    }
}

/**
 * Create user directory in USERS.
 * @param UID user ID
 * @return 1 for success or 0 for errors
*/
int CreateUserDir(char *UID){

    char user_dirname[20];

    sprintf(user_dirname,"USERS/%s",UID);
    if(mkdir(user_dirname,0700) == -1){
        logError(TRUE, "Failed to create user directory.");
        return FALSE;
    }

    return TRUE;
}


/**
 * Delete user directory in USERS.
 * @param UID user ID
 * @return 1 for success or 0 for errors
*/
int DelUserDir(char *UID){

    char user_dirname[20];

    sprintf(user_dirname,"USERS/%s",UID);

    if(rmdir(user_dirname) != 0){
        logError(TRUE, "Failed to delete user directory.");
        return FALSE;
    }

    return TRUE;
}

/**
 * Check if user exists.
 * @param UID user ID
 * @return 1 for success or 0 for errors
*/
int UserExists(char* UID){

    DIR* dir;
    char path[31];

    sprintf(path, "USERS/%s", UID);

    if(dir = opendir(path)){
        // User exists
        closedir(dir);
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


/**
 * Create pass file in user directory.
 * @param UID user ID
 * @param password user password
 * @return 1 for success or 0 for errors
*/
int CreatePassFile(char* UID, char* password){

    FILE *fptr;
    char path[31];

    sprintf(path, "USERS/%s/%s_pass.txt", UID, UID);

    if(!(fptr = fopen(path, "w"))){
        logError(TRUE, "Failed to create user password file.");
        return FALSE;
    }

    if(fwrite(password, sizeof(char), 8, fptr) < 8){
        fclose(fptr);
        return FALSE;
    }
    fclose(fptr);

    return TRUE;
}

/**
 * Delete pass file in user directory.
 * @param UID user ID
 * @return 1 for success or 0 for errors
*/
int DelPassFile(char *UID){

    char pathname[50];

    sprintf(pathname,"USERS/%s/%s_pass.txt",UID,UID);

    if(unlink(pathname) == 0){
        return TRUE;

    } else{
        logError(TRUE, "Failed to delete user password file.");
        return FALSE;
    }

}

/**
 * Check if password is correct.
 * @param UID user ID
 * @param password user password
 * @return 1 for success or 0 for errors
*/
int checkUserPassword(char* UID, char* password){

    FILE *fptr;
    char path[31];
    char userPassword[9];

    memset(userPassword, 0, 9);

    sprintf(path, "USERS/%s/%s_pass.txt", UID, UID);

    if(!(fptr = fopen(path, "r"))){
        logError(TRUE, "Failed to open user password file.");
        return FALSE;
    }
    
    if(fread(userPassword, sizeof(char), 8, fptr) < 8){
        fclose(fptr);
        return FALSE;
    }
    fclose(fptr);

    if(!strcmp(userPassword,password))
        return TRUE;
    else
        return FALSE;
}

/**
 * Create login file in user directory.
 * @param UID user ID
 * @return 1 for success or 0 for errors
*/
int createLoginFile(char* UID){

    FILE* fptr;
    char path[32];

    sprintf(path, "USERS/%s/%s_login.txt", UID, UID);

    if(!(fptr = fopen(path, "w"))){
        logError(TRUE, "Failed to create user login file.");
        return FALSE;
    }

    fclose(fptr);
    return TRUE;
}

/**
 * Delete login file in user directory.
 * @param UID user ID
 * @return 1 for success or 0 for errors
*/
int DelLoginFile(char *UID){

    char pathname[50];

    sprintf(pathname,"USERS/%s/%s_login.txt",UID,UID);

    if(unlink(pathname) == 0){
        return TRUE;

    } else {
        logError(TRUE, "Failed to delete user login file.");
        return FALSE;
    }

}
 
/**
 * Verify if user is logged in.
 * @param UID user ID
 * @return 1 for success or 0 for errors
*/
int CheckUserLogin(char* UID){

    FILE* fptr;
    char path[32];

    sprintf(path, "USERS/%s/%s_login.txt", UID, UID);

    if(!(fptr = fopen(path, "r"))){
        return FALSE;
    }

    fclose(fptr);
    return TRUE;
}

/**
 * Create group directory in GROUPS and MSG directory inside.
 * @param GID group ID
 * @return 1 for success or 0 for errors
*/
int CreateGroupDir(char *GID){

    char group_dirname[20];

    sprintf(group_dirname,"GROUPS/%s",GID);

    if(mkdir(group_dirname,0700) == -1){
        logError(TRUE, "Failed to create group directory.");
        return FALSE;
    }

    sprintf(group_dirname,"GROUPS/%s/MSG",GID);

    if(mkdir(group_dirname,0700) == -1){
        logError(TRUE, "Failed to create message directory.");
        return FALSE;
    }

    return TRUE;
}

/**
 * Return biggest group number.
 * @return max group number for success or -1 for errors
*/
int maxGroupNumber(){

    DIR *d;
    int groupMax = 0;
    struct dirent *dir;

    if (d = opendir("GROUPS"))
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
        closedir(d);
        return groupMax;
    }
    else{   
        logError(TRUE, "Failed to open groups directory.");
        closedir(d);
        return -1;
    }
}

/**
 * Get group last messsage ID.
 * @param GID group ID
 * @return message ID for success or -1 for errors
*/
int GroupLastMessage(char *GID){

    DIR* d;
    struct dirent *dir;
    int messageNumber ,max = 0;
    char path[14];
    
    sprintf(path,"GROUPS/%s/MSG",GID);

    if(d = opendir(path)){
        while((dir = readdir(d)) != NULL){
            if(dir->d_name[0] == '.' || strlen(dir->d_name) > 4)
                continue;

            messageNumber = atoi(dir->d_name);
            if(messageNumber > max)
                max = messageNumber;
        }
        closedir(d);
        return max;
    } 
    else{
        logError(TRUE, "Failed to open message directory.");
        closedir(d);
        return -1;
    }
    
}

/**
 * Auxiliary function for qsort
 * Compares two group IDs
 * @param name1 group ID
 * @param name2 group ID
 * @return integer 
*/
int compName(const void* name1, const void* name2){

    Group* group1 = (Group*) name1;
    Group* group2 = (Group*) name2;

    return (atoi(group1->number)-atoi(group2->number));
}

/**
 * Quick sort function to sort list of groups by group ID
 * @param list list of groups
 * @param numberGroups number of groups in that list
*/
void SortGList(Group* list, int numberGroups){
    qsort(list,numberGroups,sizeof(Group),compName);
}

/**
 * Fill Group struct with all groups info
 * @param list list of groups
 * @return 1 for success or 0 for errors 
*/
int ListGroupsDir(Group* list){
    
    DIR *d;
    FILE *fp;
    struct dirent *dir;
    int lastMessage, numberGroups, i;
    char GIDname[30];
    
    if(d = opendir("GROUPS")){
        i = 0; numberGroups = 0;
        while((dir = readdir(d)) != NULL){
            if(dir->d_name[0] == '.')
                continue;
            if(strlen(dir->d_name)>2)
                continue;

            strcpy(list[i].number, dir->d_name);
            sprintf(GIDname, "GROUPS/%s/%s_name.txt", list[i].number, list[i].number);
            
            fp = fopen(GIDname, "r");
            if(fp){
                fscanf(fp,"%24s", list[i].name);
                fclose(fp);
            }
            lastMessage = GroupLastMessage(dir->d_name);
            if(lastMessage != -1)
                sprintf(list[i].lastMsg, "%04d", lastMessage);
            else
                return(-1);

            i++;
            if(i == MAXGROUPS)
                break;
        }
        numberGroups = i;
        closedir(d);
    }
    else{
        closedir(d);
        logError(TRUE, "Failed to open groups directory.");
        return -1;
    }
        
    if(numberGroups>1){
        SortGList(list,numberGroups);
    }   

    return(numberGroups);
}

/**
 * Create group file in group directory.
 * @param GID group ID
 * @param Gname group name
 * @return 1 for success or 0 for errors
*/
int CreateGroupFile(char* GID, char* Gname){

    FILE *fptr;
    char path[31];

    sprintf(path, "GROUPS/%s/%s_name.txt", GID, GID);

    if(!(fptr = fopen(path, "w"))){
        logError(TRUE, "Failed to create group name file");
        return FALSE;
    }
  
    if(fwrite(Gname, sizeof(char), strlen(Gname), fptr) < strlen(Gname)){
        fclose(fptr);
        return FALSE;
    }
    fclose(fptr);

    return TRUE;
}

/**
 * Check if group exists.
 * @param GID group ID
 * @return 1 for success or 0 for errors
*/
int GroupExists(char* GID){

    DIR *d;
    FILE *fp;
    struct dirent *dir;

    d = opendir("GROUPS");
    if (d){
        while ((dir = readdir(d))){
            if(strcmp(GID, dir->d_name) == 0)
            return TRUE;
        }
        return FALSE;
    }
    else{
        logError(TRUE, "Failed to open groups directory.");
        return FALSE;
    }
}

/**
 * Check if group name is correct.
 * @param GID group ID
 * @param Gname group name
 * @return 1 for success or 0 for errors
*/
int checkGroupName(char* GID, char* GName){

    FILE* fptr;
    char path[50];
    char groupName[25];

    memset(groupName, 0, 25);

    sprintf(path, "GROUPS/%s/%s_name.txt", GID, GID);

    if(!(fptr = fopen(path, "r"))){
        logError(TRUE, "Failed to open group name file.");
        return FALSE;
    }

    if(fread(groupName, sizeof(char), 24, fptr) == 0){
        fclose(fptr);
        return FALSE;
    }
    fclose(fptr);

    if(!strcmp(groupName, GName)){
        return TRUE;
    }
    else{
        return FALSE;
    }
}

/**
 * Fills GName with the group name that corresponds to the specified GID.
 * @param GID group ID
 * @param Gname group name
 * @return 1 for success or 0 for errors
*/
int getGroupName(char* GID, char* GName){

    FILE* fptr;
    char path[50];

    sprintf(path, "GROUPS/%s/%s_name.txt", GID, GID);

    if(!(fptr = fopen(path, "r"))){
        logError(TRUE, "Failed to open group name file.");
        return FALSE;
    }

    if(fread(GName, sizeof(char), 24, fptr) == 0){
        fclose(fptr);
        return FALSE;
    }
    fclose(fptr);

    return TRUE;
}

/**
 * Add user to subscribed users of group.
 * @param UID user ID
 * @param GID group ID
 * @return 1 for success or 0 for errors
*/
int SubscribeUser(char* UID, char* GID){

    FILE *fptr;
    char path[31];

    sprintf(path, "GROUPS/%s/%s.txt", GID, UID);

    if(!(fptr = fopen(path, "w"))){
        logError(TRUE, "Failed to create subscription file.");
        return FALSE;
    }

    fclose(fptr);

    return TRUE;
}

/**
 * Remove user from group.
 * @param UID user ID
 * @param GID group ID
 * @return 1 for success or 0 for errors
*/
int UnsubscribeUser(char* UID, char* GID){

    char pathname[50];
    sprintf(pathname,"GROUPS/%s/%s.txt",GID,UID);

    if(unlink(pathname) == 0){
        return TRUE;

    } else{
        logError(TRUE, "Failed to remove subscription file.");
        return FALSE;
    }

}

/**
 * Check if user is subscribe to group.
 * @param UID user ID
 * @param GID group ID
 * @return 1 for success or 0 for errors
*/
int checkUserSubscribedToGroup(char* UID, char* GID){

    DIR* dir;
    FILE *fptr;
    char path[31];
    
    sprintf(path, "GROUPS/%s/%s.txt", GID, UID);

    if(!(fptr = fopen(path, "r"))){
        return FALSE;
    }
    fclose(fptr);
    return TRUE;
}

/**
 * Deletes user from subscribed groups.
 * @param UID user ID
 * @return 1 for success or 0 for errors
*/
int DelUserFromGroups(char* UID){

    char GID[3];
    int maxGroup = maxGroupNumber();

    for(int i = 1; i <= maxGroup; i++){
        sprintf(GID,"%02d",i);
        if(checkUserSubscribedToGroup(UID,GID)){
            UnsubscribeUser(UID,GID);
        }
    }

    return TRUE;
}

/**
 * Check if user is subscribe to group.
 * @param UID user ID
 * @param GID group ID
 * @param message message to be copied to file
 * @return 1 for success or 0 for errors
*/
int CreateMessageDir(char* UID, char* GID, char* message){

    FILE *fptr;
    
    int ret;
    int messageNumber;
    char group_dirname[19];
    char pathAuthor[35];
    char pathText[35];

    messageNumber = GroupLastMessage(GID);
    if(messageNumber == -1)
        return -1;

    // Next message
    messageNumber ++;

    // New message directory
    sprintf(group_dirname,"GROUPS/%s/MSG/%04d",GID,messageNumber);
    ret=mkdir(group_dirname,0700);
    if(ret==-1)
        return -1;

    // Create author file
    sprintf(pathAuthor, "%s/A U T H O R.txt",group_dirname);
    if(!(fptr = fopen(pathAuthor, "w"))){
        unlink(group_dirname);
        return -1;
    }
        
    if(fwrite(UID, sizeof(char), strlen(UID), fptr) < strlen(UID)){
        fclose(fptr);
        return FALSE;
    }
    fclose(fptr);

    // Create text file
    sprintf(pathText, "%s/T E X T.txt",group_dirname);
    if(!(fptr = fopen(pathText, "w"))){
        unlink(pathAuthor);
        unlink(group_dirname);
        return -1;
    }  

    if(fwrite(message, sizeof(char), strlen(message), fptr) < strlen(message)){
        fclose(fptr);
        return FALSE;
    }
    fclose(fptr);

    return messageNumber;
}

// Fills path with the path of the file on message
int getMessageFilePath(char* GID, int MID, char* fileName){
    
    DIR* d;
    char path[19];

    sprintf(path,"GROUPS/%s/MSG/%04d", GID, MID);
    d = opendir(path);
    if (d){
        
        while (1){
            struct dirent *dir;
            if(!(dir = readdir(d))){
                break;
            }

            if(dir->d_name[0] == '.')
                continue;

            if(strcmp("A U T H O R.txt",dir->d_name) && strcmp("T E X T.txt",dir->d_name)){
                strcpy(fileName,dir->d_name);
                closedir(d);
                return TRUE;
            }
        }
        closedir(d);
        return FALSE;
    }
    else{
        logError(TRUE, "Failed to open message directory.");
        closedir(d);
        return FALSE;
    }  
}