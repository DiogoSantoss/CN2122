#ifndef STRUCTS_DOT_H
#define STRUCTS_DOT_H

struct serverData {
    int verbose;
    char port[6];
} typedef serverData;

struct userData {
    int fd;
    socklen_t addrlen;
    struct sockaddr_in* addr;
} typedef userData;

struct Group {
    char groupNumber[3];
    char groupName[25];
    char groupLastMsg[5];
} typedef Group;

#endif