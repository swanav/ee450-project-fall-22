#include <stdio.h>
#include <stdlib.h>

#include "log.h"


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