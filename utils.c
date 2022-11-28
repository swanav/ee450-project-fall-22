#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

// Implemented after referring from stackoverflow.com/questions/122616
char* utils_string_ltrim(char* s) {
    if(!s) return NULL;
    while(isspace(*s)) s++;
    return s;
}

// Implemented after referring from stackoverflow.com/questions/122616
char* utils_string_rtrim(char* s) {
    if(!s) return NULL;
    char* back = s + strlen(s);
    while(back != s && isspace(back[-1])) back--;
    *back = '\0';
    return s;
}

char* utils_string_trim(char* s) {
    if(!s) return NULL;
    return utils_string_rtrim(utils_string_ltrim(s)); 
}

int utils_get_word_count(char* str) {
    int wc = 0;
    char* p = utils_string_trim(str);
    while (*p) {
        if (*p == ' ' || *p == '\t') {
            wc++;
        }
        p++;
    }
    return wc + 1;
}
