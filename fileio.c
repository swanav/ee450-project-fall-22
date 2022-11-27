#include "fileio.h"
#include "log.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LOG_TAG(fileio.c);

#define CSV_SPLIT_TOKEN ",\r\n"

static FILE* csv_open(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        LOG_ERR("Failed to open file %s", filename);
        return NULL;
    }
    return fp;
}

static void csv_close(FILE* fp) {
    fclose(fp);
}

credentials_t* fileio_credential_server_db_create(const char* filename) {

    if (filename == NULL) return NULL;

    FILE* fp = csv_open(filename);
    char line[1024];

    if (fp == NULL) return NULL;

    credentials_t* credentials = NULL;
    while (fgets(line, sizeof(line), fp)) {
        char* token = strtok(line, CSV_SPLIT_TOKEN);
        if (token == NULL) continue;

        credentials_t* ptr = malloc(sizeof(credentials_t));
        if (ptr == NULL) {
            LOG_ERR("Failed to allocate memory for credentials");
            return NULL;
        }

        bzero(ptr, sizeof(credentials_t));

        ptr->username_len = strlen(token);
        if (ptr->username_len > CREDENTIALS_MAX_USERNAME_LEN) {
            LOG_ERR("Username is too long");
            continue;
        }
        memcpy(ptr->username, token, min(ptr->username_len, CREDENTIALS_MAX_USERNAME_LEN));

        token = strtok(NULL, CSV_SPLIT_TOKEN);
        if (token == NULL) {
            continue;
        }

        ptr->password_len = strlen(token);
        if (ptr->password_len > CREDENTIALS_MAX_PASSWORD_LEN) {
            LOG_ERR("Password is too long");
            continue;
        }
        memcpy(ptr->password, token, min(ptr->password_len, CREDENTIALS_MAX_PASSWORD_LEN));

        if (credentials != NULL) {
            ptr->next = credentials;
        }

        credentials = ptr;
    }

    csv_close(fp);

    return credentials;
}

void fileio_credential_server_db_free(credentials_t* db) {
    while (db != NULL) {
        credentials_t* ptr = db;
        db = db->next;
        free(ptr);
    }
}

course_t* fileio_department_server_db_create(const char* filename) {
    course_t* head = NULL;
    course_t* tail = NULL;
    FILE* fp = csv_open(filename);
    char line[1024];

    if (fp == NULL) {
        return NULL;
    }

    while (fgets(line, sizeof(line), fp)) {
        char* token = strtok(line, CSV_SPLIT_TOKEN);
        if (token == NULL) {
            continue;
        }
        course_t* entry = (course_t*)malloc(sizeof(course_t));
        if (entry == NULL) {
            LOG_ERR("Failed to allocate memory for course_t");
            continue;
        }
        memset(entry, 0, sizeof(course_t));
        strncpy(entry->course_code, token, sizeof(entry->course_code));
        token = strtok(NULL, CSV_SPLIT_TOKEN);
        if (token == NULL) {
            continue;
        }
        entry->credits = atoi(token);
        token = strtok(NULL, CSV_SPLIT_TOKEN);
        if (token == NULL) {
            continue;
        }
        strncpy(entry->professor, token, sizeof(entry->professor));
        token = strtok(NULL, CSV_SPLIT_TOKEN);
        if (token == NULL) {
            continue;
        }
        strncpy(entry->days, token, sizeof(entry->days));
        token = strtok(NULL, CSV_SPLIT_TOKEN);
        if (token == NULL) {
            continue;
        }
        strncpy(entry->course_name, token, sizeof(entry->course_name));
        if (head == NULL) {
            head = entry;
            tail = entry;
        } else {
            tail->next = entry;
            tail = entry;
        }
    }

    fclose(fp);
    return head;
}

void fileio_department_server_db_free(course_t* head) {
    course_t* entry = head;
    while (entry != NULL) {
        course_t* next = entry->next;
        free(entry);
        entry = next;
    }
}
