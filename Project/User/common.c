#include <string.h>
#include <ctype.h>

// Booleans
#define TRUE  1
#define FALSE 0

/**
 * Check if string is a number
 * @param[in] value String to be checked
 * @param[out] IsNumber TRUE if is number else FALSE
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
 * @param[in] value String to be checked
 * @param[out] IsAlphaNumberic TRUE if is alphanumeric else FALSE
*/
int checkStringIsAlphaNum(char* value){
    int IsAlphaNumeric = TRUE;
    for(int i = 0; i<strlen(value)-1; i++){
        if(!isdigit(value[i]) && !isalpha(value[i])){
            IsAlphaNumeric = FALSE;
            break;
        }
    } 
    return IsAlphaNumeric;
}

/**
 * Check if string is valid group name
 * @param[in] value String to be checked
 * @param[out] IsNumber TRUE if is valid group name else FALSE
*/
int checkStringIsGroupName(char* value){
    int IsGroupName = TRUE;
    for(int i = 0; i<strlen(value)-1; i++){
        if(!isdigit(value[i]) && !isalpha(value[i]) && value[i] != '-' && value[i] != '_'){
            IsGroupName = FALSE;
            break;
        }
    } 
    return IsGroupName;
}

/**
 * Check if string is valid filename
 * @param[in] value String to be checked
 * @param[out] TRUE if is valaid filename else FALSE
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