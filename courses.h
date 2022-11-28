#ifndef COURSES_H
#define COURSES_H

#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "networking.h"
#include "protocol.h"

err_t courses_lookup_multiple_request_encode(struct __message_t* out_msg, uint8_t course_count, uint8_t* buffer, uint8_t buffer_len);
err_t courses_lookup_multiple_request_decode(struct __message_t* out_msg, uint8_t* course_count, uint8_t* buffer, uint8_t buffer_len);

err_t courses_lookup_multiple_response_encode(struct __message_t* out_msg, course_t* courses);
course_t* courses_lookup_multiple_response_decode(struct __message_t* in_msg);

course_t* courses_lookup(const course_t* db, const char* course_code);
err_t courses_lookup_info(const course_t* course, courses_lookup_category_t category, uint8_t* info_buf, size_t info_buf_size, size_t* info_len);

char* courses_category_string_from_enum(courses_lookup_category_t category);
courses_lookup_category_t courses_lookup_category_from_string(const char* category);

#endif // COURSES_H
