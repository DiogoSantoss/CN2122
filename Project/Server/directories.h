#ifndef DIRECTORIES_DOT_H
#define DIRECTORIES_DOT_H

void createDirectories();
int CreateUserDir(char *UID);
int DelUserDir(char *UID);
int UserExists(char* UID);
int CreatePassFile(char* UID, char* password);
int DelPassFile(char *UID);
int checkUserPassword(char* UID, char* password);
int createLoginFile(char* UID);
int DelLoginFile(char *UID);
int maxGroupNumber();
int CreateGroupDir(char *GID);
int ListGroupsDir(GROUPLIST *list);
int CreateGroupFile(char* UID, char* password);
int GroupExists(char* GID);
int checkGroupName(char* GID, char* GName);
int SubscribeUser(char* UID, char* GID);
int UnsubscribeUser(char* UID, char* GID);
int checkUserSubscribedToGroup(char* UID, char* GID);

#endif