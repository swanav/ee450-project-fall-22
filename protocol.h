#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include "networking.h"

#define DEPARTMENT_PREFIX_EE                        "EE"
#define DEPARTMENT_PREFIX_CS                        "CS"
#define DEPARTMENT_PREFIX_LEN                       2

#define REQUEST_RESPONSE_TYPE_OFFSET                0
#define REQUEST_RESPONSE_FLAGS_OFFSET               1
#define REQUEST_RESPONSE_PAYLOAD_LEN_OFFSET_1       2
#define REQUEST_RESPONSE_PAYLOAD_LEN_OFFSET_2       3
#define REQUEST_RESPONSE_HEADER_LEN                 4

typedef uint8_t request_type_t;
#define REQUEST_TYPE_AUTH                           0x61
#define REQUEST_TYPE_COURSES_LOOKUP_INFO            0x62
#define REQUEST_TYPE_COURSES_MULTI_LOOKUP           0x63
#define REQUEST_TYPE_COURSES_DETAIL_LOOKUP          0x64
#define REQUEST_TYPE_END                            0x65

typedef uint8_t response_type_t;
#define RESPONSE_TYPE_AUTH                          0x71
#define RESPONSE_TYPE_COURSES_LOOKUP_INFO           0x72
#define RESPONSE_TYPE_COURSES_MULTI_LOOKUP          0x73
#define RESPONSE_TYPE_COURSES_DETAIL_LOOKUP         0x74
#define RESPONSE_TYPE_END                           0x75

#define REQUEST_RESPONSE_INVALID_TYPE               0x00

typedef uint8_t auth_flags_t;
#define AUTH_FLAGS_SUCCESS                          (1 << 4)
#define AUTH_FLAGS_FAILURE                          (1 << 5)
#define AUTH_FLAGS_USER_NOT_FOUND                   (1 << 0)
#define AUTH_FLAGS_PASSWORD_MISMATCH                (1 << 1)

#define AUTH_MASK_SUCCESS(x)                        (x & AUTH_FLAGS_SUCCESS)
#define AUTH_MASK_FAILURE(x)                        (x & AUTH_FLAGS_FAILURE)
#define AUTH_MASK_USER_NOT_FOUND(x)                 (x & AUTH_FLAGS_USER_NOT_FOUND)
#define AUTH_MASK_PASSWORD_MISMATCH(x)              (x & AUTH_FLAGS_PASSWORD_MISMATCH)


typedef uint8_t courses_lookup_category_t;
#define COURSES_LOOKUP_CATEGORY_COURSE_CODE         0x50
#define COURSES_LOOKUP_CATEGORY_CREDITS             0x51
#define COURSES_LOOKUP_CATEGORY_PROFESSOR           0x52    
#define COURSES_LOOKUP_CATEGORY_DAYS                0x53
#define COURSES_LOOKUP_CATEGORY_COURSE_NAME         0x54    
#define COURSES_LOOKUP_CATEGORY_INVALID             0x55

#define COURSE_LOOKUP_FLAGS_COURSE_CODE             (1 << 0)
#define COURSE_LOOKUP_FLAGS_CREDITS                 (1 << 1)
#define COURSE_LOOKUP_FLAGS_PROFESSOR               (1 << 2)
#define COURSE_LOOKUP_FLAGS_DAYS                    (1 << 3)
#define COURSE_LOOKUP_FLAGS_COURSE_NAME             (1 << 4)
#define COURSE_LOOKUP_FLAGS_INVALID                 (1 << 7)

#define COURSE_LOOKUP_MASK_COURSE_CODE(x)           (x & COURSE_LOOKUP_FLAGS_COURSE_CODE)
#define COURSE_LOOKUP_MASK_CREDITS(x)               (x & COURSE_LOOKUP_FLAGS_CREDITS)
#define COURSE_LOOKUP_MASK_PROFESSOR(x)             (x & COURSE_LOOKUP_FLAGS_PROFESSOR)
#define COURSE_LOOKUP_MASK_DAYS(x)                  (x & COURSE_LOOKUP_FLAGS_DAYS)
#define COURSE_LOOKUP_MASK_COURSE_NAME(x)           (x & COURSE_LOOKUP_FLAGS_COURSE_NAME)
#define COURSE_LOOKUP_MASK_INVALID(x)               (x & COURSE_LOOKUP_FLAGS_INVALID)



// Create UDP or TCP packet based on given buffer
void protocol_encode(struct __message_t* message, uint8_t type, uint8_t flags, uint16_t payload_len, uint8_t* payload);

// Recreate buffer from given UDP or TCP packet
void protocol_decode(struct __message_t* message, request_type_t* request_type, uint8_t* flags, uint16_t *out_data_len, const uint16_t out_data_size, uint8_t* out_data);

request_type_t protocol_get_request_type(struct __message_t* message);

uint16_t protocol_get_payload_len(struct __message_t* message);

uint8_t protocol_get_flags(struct __message_t* message);

#endif // PROTOCOL_H
