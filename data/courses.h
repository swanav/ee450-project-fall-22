#ifndef COURSES_H
#define COURSES_H

#include <stdlib.h>
#include <string.h>

#include "../networking/networking.h"
#include "../utils/error.h"

typedef enum {
    COURSES_LOOKUP_CATEGORY_COURSE_CODE = 0x50,
    COURSES_LOOKUP_CATEGORY_CREDITS,
    COURSES_LOOKUP_CATEGORY_PROFESSOR,
    COURSES_LOOKUP_CATEGORY_DAYS,
    COURSES_LOOKUP_CATEGORY_COURSE_NAME,
    COURSES_LOOKUP_CATEGORY_INVALID,
} courses_lookup_category_t;

typedef struct __lookup_params_t {
    courses_lookup_category_t category;
    char course_code[64];
} courses_lookup_params_t;

typedef struct __course_t {
    char course_code[32];
    int credits;
    char professor[64];
    char days[32];
    char course_name[128];
    struct __course_t* next;
} course_t;


#define COURSE_LOOKUP_FLAGS_COURSE_CODE         (1 << 0)
#define COURSE_LOOKUP_FLAGS_CREDITS             (1 << 1)
#define COURSE_LOOKUP_FLAGS_PROFESSOR           (1 << 2)
#define COURSE_LOOKUP_FLAGS_DAYS                (1 << 3)
#define COURSE_LOOKUP_FLAGS_COURSE_NAME         (1 << 4)

#define COURSE_LOOKUP_MASK_COURSE_CODE(flags)   ((flags) & COURSE_LOOKUP_FLAGS_COURSE_CODE)
#define COURSE_LOOKUP_MASK_CREDITS(flags)       ((flags) & COURSE_LOOKUP_FLAGS_CREDITS)
#define COURSE_LOOKUP_MASK_PROFESSOR(flags)     ((flags) & COURSE_LOOKUP_FLAGS_PROFESSOR)
#define COURSE_LOOKUP_MASK_DAYS(flags)          ((flags) & COURSE_LOOKUP_FLAGS_DAYS)
#define COURSE_LOOKUP_MASK_COURSE_NAME(flags)   ((flags) & COURSE_LOOKUP_FLAGS_COURSE_NAME)


course_t* courses_init(const char* filename);
void courses_free(course_t* head);

err_t courses_lookup_encode();
err_t courses_lookup_decode(udp_dgram_t* req_dgram, courses_lookup_params_t* params);

err_t courses_summary_encode(const course_t* course, uint8_t* buffer, size_t buffer_len, size_t* encoded_len);
err_t courses_summary_decode(udp_dgram_t* req_dgram, courses_lookup_params_t* params);

course_t* courses_lookup(const course_t* db, const char* course_code);
err_t courses_lookup_info(const course_t* course, courses_lookup_category_t category, uint8_t* info_buf, size_t info_buf_size, size_t* info_len);

int courses_parse_lookup_request(char* req_string, size_t req_len, courses_lookup_params_t* params);

char* courses_category_string_from_enum(courses_lookup_category_t category);


#endif // COURSES_H
