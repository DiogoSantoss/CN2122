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
int CheckUserLogin(char* UID);
int maxGroupNumber();
int GroupLastMessage(char *GID);
int CreateGroupDir(char* GID);
int ListGroupsDir(Group* list);
int CreateGroupFile(char* UID, char* password);
int GroupExists(char* GID);
int checkGroupName(char* GID, char* GName);
int getGroupName(char* GID, char* GName);
int SubscribeUser(char* UID, char* GID);
int UnsubscribeUser(char* UID, char* GID);
int checkUserSubscribedToGroup(char* UID, char* GID);
int DelUserFromGroups(char* UID);
int CreateMessageDir(char* UID, char* GID, char* message);
int getMessageFilePath(char* GID, int MID, char* fileName);

#endif