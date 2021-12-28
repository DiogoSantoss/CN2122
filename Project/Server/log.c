#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "colors.h"
#include "common.h"

//Boolean
#define TRUE 1
#define FALSE 0

//Constants
#define MAXSIZE 274
#define EXTRAMAXSIZE 3268

void logError(char* message){
    colorRed();
    printf("%s\n", message);
    colorReset();
}