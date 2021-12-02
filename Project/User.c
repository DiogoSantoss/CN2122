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
    int count = 0;

    for(int i = 1; i<argc; i++){
        if(argv[i]=="-n" || argv[i]=="-p"){
            count++;
        }
    }
    if

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