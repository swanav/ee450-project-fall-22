#include "courses.h"
#include <log.h>
#include <stdio.h>
#include <utils.h>


db_entry_t* read_courses_db(const char* filename) {
    db_entry_t* head = NULL;
    db_entry_t* tail = NULL;
    FILE* fp = csv_open(filename);
    char line[1024];

    if (fp == NULL) {
        return NULL;
    }

    while (fgets(line, sizeof(line), fp)) {
        char* token = strtok(line, ",");
        if (token == NULL) {
            continue;
        }
        db_entry_t* entry = (db_entry_t*)malloc(sizeof(db_entry_t));
        if (entry == NULL) {
            LOGEM("Failed to allocate memory for db_entry_t");
            continue;
        }
        memset(entry, 0, sizeof(db_entry_t));
        strncpy(entry->course_code, token, sizeof(entry->course_code));
        token = strtok(NULL, ",");
        if (token == NULL) {
            continue;
        }
        entry->credits = atoi(token);
        token = strtok(NULL, ",");
        if (token == NULL) {
            continue;
        }
        strncpy(entry->professor, token, sizeof(entry->professor));
        token = strtok(NULL, ",");
        if (token == NULL) {
            continue;
        }
        strncpy(entry->days, token, sizeof(entry->days));
        token = strtok(NULL, ",");
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

void free_courses_db(db_entry_t* head) {
    db_entry_t* entry = head;
    while (entry != NULL) {
        db_entry_t* next = entry->next;
        free(entry);
        entry = next;
    }
}

int parse_lookup_request(char* req_string, size_t req_len, lookup_params_t* params) {
    char* token = strtok(req_string, ",");
    if (token == NULL) {
        return -1;
    }
    strncpy(params->course_code, token, sizeof(params->course_code));

    token = strtok(NULL, ",");
    if (token == NULL) {
        return -1;
    }

    strncpy(params->category, token, sizeof(params->category));

    return 0;
}

lookup_result_t course_lookup(const db_entry_t* db, char* category, char* course_code, char* info_buf, size_t info_buf_len) {
    db_entry_t* ptr = (db_entry_t*) db;
    while(ptr) {
        if (strcmp(ptr->course_code, course_code) == 0) {
            if (strcmp(category, "Credits") == 0) {
                snprintf(info_buf, info_buf_len, "%d", ptr->credits);
                return LOOKUP_OK;
            } else if (strcmp(category, "Professor") == 0) {
                strncpy(info_buf, ptr->professor, info_buf_len);
                return LOOKUP_OK;
            } else if (strcmp(category, "Days") == 0) {
                strncpy(info_buf, ptr->days, info_buf_len);
                return LOOKUP_OK;
            } else if (strcmp(category, "CourseName") == 0) {
                strncpy(info_buf, ptr->course_name, info_buf_len);
                return LOOKUP_OK;
            }
        }
        ptr =  ptr->next;
    }
    return LOOKUP_FAIL;
}
