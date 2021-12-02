#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

void parseArguments(int argc, char *argv[]){
    int count_n = 0;
    int count_p = 0;

    for(int i = 1; i<argc; i++){
        if(strcmp(argv[i],"-n")==0){
            count_n++;
        }
        if(strcmp(argv[i],"-p")==0){
            count_p++;
        }
    }
    if(count_n != 1 || count_p != 1){
        fprintf(stderr, "Invalid format\n");
        exit(1);
    }
}

int main(int argc, char *argv[]){

    /*
    arg0 -> nome
    arg1/3 -> -n / -p
    arg2/4 -> se -n inteiro, se -p char*
    */

    parseArguments(argc,argv);

    return 1;
}