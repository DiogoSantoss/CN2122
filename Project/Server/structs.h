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

struct GROUPLIST {
    int no_groups;
    char group_no[99][3];
    char group_name[99][25];
    char group_lastMens[99][5];
} typedef GROUPLIST;

#endif