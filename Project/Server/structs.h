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
    char number[3];
    char name[25];
    char lastMsg[5];
} typedef Group;

#endif