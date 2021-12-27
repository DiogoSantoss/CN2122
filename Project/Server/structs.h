#ifndef STRUCTS_DOT_H
#define STRUCTS_DOT_H

struct serverData {
    int verbose;
    char port[6];
} typedef serverData;

struct userData {
    char port[6];
    char ipAddress[513];
} typedef userData;

#endif