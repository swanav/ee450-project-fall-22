#ifndef COURSES_H
#define COURSES_H

#include <stdlib.h>
#include <string.h>

#include "../networking.h"
#include "../error.h"
#include "../protocol.h"

typedef struct __lookup_params_t {
    courses_lookup_category_t category;
    char course_code[64];
} courses_lookup_params_t;

err_t courses_lookup_multiple_request_encode(struct __message_t* out_msg, uint8_t course_count, uint8_t* buffer, uint8_t buffer_len);
err_t courses_lookup_multiple_request_decode(struct __message_t* out_msg, uint8_t* course_count, uint8_t* buffer, uint8_t buffer_len);

err_t courses_lookup_multiple_response_encode(struct __message_t* out_msg, course_t* courses);
course_t* courses_lookup_multiple_response_decode(struct __message_t* in_msg);

err_t courses_details_response_encode(const course_t* course, struct __message_t* out_msg);
err_t courses_details_response_decode(udp_dgram_t* req_dgram, course_t* course);

err_t courses_details_request_encode(uint8_t * course_code, uint8_t course_code_len, struct __message_t* out_msg);
err_t courses_details_request_decode(struct __message_t* in_msg, uint8_t* course_code, uint8_t course_code_size);


course_t* courses_lookup(const course_t* db, const char* course_code);
err_t courses_lookup_info(const course_t* course, courses_lookup_category_t category, uint8_t* info_buf, size_t info_buf_size, size_t* info_len);

int courses_parse_lookup_request(char* req_string, size_t req_len, courses_lookup_params_t* params);

char* courses_category_string_from_enum(courses_lookup_category_t category);
courses_lookup_category_t courses_lookup_category_from_string(const char* category);

char* course_lookup_info_get_department(const char* course_code);


#endif // COURSES_H
