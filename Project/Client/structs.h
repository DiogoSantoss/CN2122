#ifndef STRUCTS_DOT_H
#define STRUCTS_DOT_H

struct userData {
    int fd;
    struct addrinfo *res;
    
    char ID[6];
    char password[9];
    char groupID[3];
} typedef userData;

struct serverData {
    char port[6];
    char ipAddress[513];
} typedef serverData;

#endif