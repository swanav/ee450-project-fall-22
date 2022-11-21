#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#include "log.h"
#include "messages.h"


FILE* csv_open(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        LOGE("Failed to open file %s", filename);
        return NULL;
    }
    return fp;
}

void csv_close(FILE* fp) {
    fclose(fp);
}

// read csv file
void read_csv(const char *filename, int *n, int *d) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        LOGE("Failed to open file %s", filename);
        exit(1);
    }
    char line[1024];
    int i = 0;
    while (fgets(line, 1024, fp)) {
        LOGD("line %d: %s", i, line);
        // char *tmp = strdup(line);
        // int j = 0;
        // const char *tok;
        // for (tok = strtok(line, ","); tok && *tok; j++, tok = strtok(NULL, ",\n")) {
        //     data[i][j] = atof(tok);
        // }
        // if (i == 0) {
        //     *d = j;
        // }
        // free(tmp);
        i++;
    }
    *n = i;
    fclose(fp);
}

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
