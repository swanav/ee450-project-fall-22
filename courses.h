#ifndef COURSES_H
#define COURSES_H

#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "networking.h"
#include "protocol.h"

course_t* courses_lookup(const course_t* db, const char* course_code);
err_t courses_lookup_info(const course_t* course, courses_lookup_category_t category, uint8_t* info_buf, size_t info_buf_size, size_t* info_len);

char* courses_category_string_from_enum(courses_lookup_category_t category);
courses_lookup_category_t courses_lookup_category_from_string(const char* category);

#endif // COURSES_H
