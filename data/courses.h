#ifndef COURSES_H
#define COURSES_H

#include <stdlib.h>
#include <string.h>

#include "../networking/networking.h"
#include "../utils/error.h"
#include "../protocol.h"

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

course_t* courses_init(const char* filename);
void courses_free(course_t* head);

err_t courses_lookup_info_request_encode(courses_lookup_params_t* params, struct __message_t* out_msg);
err_t courses_lookup_info_request_decode(struct __message_t* msg, courses_lookup_params_t* params);

err_t courses_lookup_info_response_encode(struct __message_t* out_msg, courses_lookup_params_t* params, uint8_t* info_buffer, uint8_t info_buffer_len);
err_t courses_lookup_info_response_decode(struct __message_t* msg, courses_lookup_params_t* params, uint8_t* info_buffer, uint8_t info_buffer_size, uint8_t* info_len);

err_t courses_lookup_info_response_encode_error(struct __message_t* out_msg, courses_lookup_params_t* params, uint8_t error_code);
err_t courses_lookup_info_response_decode_error(struct __message_t* msg, courses_lookup_params_t* params, uint8_t* error_code);

err_t courses_lookup_encode(courses_lookup_params_t* params, uint8_t* buffer, uint8_t buffer_size, uint8_t* out_len);
err_t courses_lookup_decode(udp_dgram_t* req_dgram, courses_lookup_params_t* params);

err_t courses_summary_encode(const course_t* course, uint8_t* buffer, size_t buffer_len, size_t* encoded_len);
err_t courses_summary_decode(udp_dgram_t* req_dgram, courses_lookup_params_t* params);

course_t* courses_lookup(const course_t* db, const char* course_code);
err_t courses_lookup_info(const course_t* course, courses_lookup_category_t category, uint8_t* info_buf, size_t info_buf_size, size_t* info_len);

int courses_parse_lookup_request(char* req_string, size_t req_len, courses_lookup_params_t* params);

char* courses_category_string_from_enum(courses_lookup_category_t category);
courses_lookup_category_t courses_lookup_category_from_string(const char* category);

char* course_lookup_info_get_department(const char* course_code);


#endif // COURSES_H
