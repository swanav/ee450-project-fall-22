#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#include "log.h"
#include "messages.h"

LOG_TAG(utils);

char* string_ltrim(char* s) {
    if(!s) return NULL;
    while(isspace(*s)) s++;
    return s;
}

char* string_rtrim(char* s) {
    if(!s) return NULL;
    char* back = s + strlen(s);
    // Peek at the previous character.
    // Go back if previous character is also a space
    while(back != s && isspace(back[-1])) back--;
    *back = '\0';
    return s;
}

char* string_trim(char* s) {
    if(!s) return NULL;
    return string_rtrim(string_ltrim(s)); 
}

int utils_get_word_count(char* str) {
    int wc = 0;
    while (*str) {
        if (*str == ' ' || *str == '\t') {
            wc++;
        }
        str++;
    }
    return wc + 1;
}
