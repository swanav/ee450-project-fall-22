#ifndef COURSES_H
#define COURSES_H

#include <stdlib.h>
#include <string.h>

typedef struct __lookup_params_t {
    char category[64];
    char course_code[64];
} lookup_params_t;

typedef enum __lookup_result_t {
    LOOKUP_OK,
    LOOKUP_FAIL,
} lookup_result_t;

typedef struct __db_entry_t {
    char course_code[32];
    int credits;
    char professor[64];
    char days[32];
    char course_name[128];
    struct __db_entry_t* next;
} db_entry_t;

db_entry_t* read_courses_db(const char* filename);
void free_courses_db(db_entry_t* head);

int parse_lookup_request(char* req_string, size_t req_len, lookup_params_t* params);
lookup_result_t course_lookup(const db_entry_t* db, char* category, char* course_code, char* info_buf, size_t info_buf_len);



#endif // COURSES_H
