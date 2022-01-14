#include <string.h>
#include <ctype.h>

// Booleans
#define TRUE  1
#define FALSE 0

/**
 * Check if string is a number
 * @param value String to be checked
 * @return 1 if is number else 0
*/
int checkStringIsNumber(char* value){
    int IsNumber = TRUE;
    for(int i = 0; i<strlen(value); i++){
        if(!isdigit(value[i])){
            IsNumber = FALSE;
            break;
        }
    } 
    return IsNumber;
}

/**
 * Check if string is alphanumeric
 * @param value String to be checked
 * @return 1 if is alphanumeric else 0
*/
int checkStringIsAlphaNum(char* value){
    int IsAlphaNumeric = TRUE;
    for(int i = 0; i<strlen(value); i++){
        if(!isdigit(value[i]) && !isalpha(value[i])){
            IsAlphaNumeric = FALSE;
            break;
        }
    } 
    return IsAlphaNumeric;
}

/**
 * Check if string is valid group name
 * @param value String to be checked
 * @return 1 if is valid group name else 0
*/
int checkStringIsGroupName(char* value){
    int IsGroupName = TRUE;
    for(int i = 0; i<strlen(value); i++){
        if(!isdigit(value[i]) && !isalpha(value[i]) && value[i] != '-' && value[i] != '_'){
            IsGroupName = FALSE;
            break;
        }
    } 
    return IsGroupName;
}

/**
 * Check if string is valid filename
 * @param value String to be checked
 * @return 1 is valaid filename else 0
*/
int checkStringIsFileName(char* value){
    if(!checkStringIsAlphaNum(value)){
        return FALSE;
    } else if(strlen(value) > 24){
        return FALSE;
    } else if(value[strlen(value)-4] == '.'){
        return FALSE;
    }
    return TRUE;
}