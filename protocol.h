#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include "networking.h"
#include "constants.h"


#define REQUEST_RESPONSE_TYPE_OFFSET                0
#define REQUEST_RESPONSE_FLAGS_OFFSET               1
#define REQUEST_RESPONSE_PAYLOAD_LEN_OFFSET_1       2
#define REQUEST_RESPONSE_PAYLOAD_LEN_OFFSET_2       3
#define REQUEST_RESPONSE_HEADER_LEN                 4

typedef uint8_t request_type_t;
#define REQUEST_TYPE_AUTH                           0x61
#define REQUEST_TYPE_COURSES_SINGLE_LOOKUP          0x62
#define REQUEST_TYPE_COURSES_MULTI_LOOKUP           0x63
#define REQUEST_TYPE_COURSES_DETAIL_LOOKUP          0x64
#define REQUEST_TYPE_END                            0x65

typedef uint8_t response_type_t;
#define RESPONSE_TYPE_AUTH                          0x71
#define RESPONSE_TYPE_COURSES_SINGLE_LOOKUP         0x72
#define RESPONSE_TYPE_COURSES_MULTI_LOOKUP          0x73
#define RESPONSE_TYPE_COURSES_DETAIL_LOOKUP         0x74
#define RESPONSE_TYPE_COURSES_ERROR                 0x75
#define RESPONSE_TYPE_END                           0x76

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

#define CREDENTIALS_USERNAME_LEN_OFFSET 0
#define CREDENTIALS_PASSWORD_LEN_OFFSET 1

#define CREDENTIALS_HEADER_LEN 2
#define CREDENTIALS_USERNAME_LEN(b) b[CREDENTIALS_USERNAME_LEN_OFFSET]
#define CREDENTIALS_PASSWORD_LEN(b) b[CREDENTIALS_PASSWORD_LEN_OFFSET]
#define CREDENTIALS_TOTAL_LEN(b) CREDENTIALS_HEADER_LEN + CREDENTIALS_USERNAME_LEN(b) + CREDENTIALS_PASSWORD_LEN(b)

typedef uint8_t courses_lookup_category_t;
#define COURSES_LOOKUP_CATEGORY_COURSE_CODE         0x50
#define COURSES_LOOKUP_CATEGORY_CREDITS             0x51
#define COURSES_LOOKUP_CATEGORY_PROFESSOR           0x52    
#define COURSES_LOOKUP_CATEGORY_DAYS                0x53
#define COURSES_LOOKUP_CATEGORY_COURSE_NAME         0x54    
#define COURSES_LOOKUP_CATEGORY_INVALID             0x55

typedef struct __credentials_t {
    uint8_t username[CREDENTIALS_MAX_USERNAME_LEN + 1];
    uint8_t password[CREDENTIALS_MAX_PASSWORD_LEN + 1];
    uint8_t username_len;
    uint8_t password_len;
    struct __credentials_t* next;
} credentials_t;

typedef struct __course_t {
    char course_code[32];
    int credits;
    char professor[64];
    char days[32];
    char course_name[128];
    struct __course_t* next;
} course_t;

request_type_t protocol_get_request_type(const struct __message_t* message);

uint16_t protocol_get_payload_len(const struct __message_t* message);

uint8_t protocol_get_flags(const struct __message_t* message);

/**
 * @brief Encode a authentication request.
 * 
 * @ref Used by client to send credentials to serverM
 * @ref Used by serverM to encode the credentials to send to serverC
 *
 * @param credentials [in] Credentials to encode
 * @param out_dgrm [out] Datagram to encode into
 * @return err_t 
 */
err_t protocol_authentication_request_encode(const credentials_t* credentials, udp_dgram_t* out_dgrm);

/**
 * @brief Decode a authentication request
 * 
 * @ref Used by serverM to decode the credentials from client
 * @ref Used by serverC to decode the credentials from serverM
 *
 * @param in_dgrm [in] Datagram to decode from
 * @param credentials [out] Credentials to decode into
 * @return err_t 
 */
err_t protocol_authentication_request_decode(const udp_dgram_t* in_dgrm, credentials_t* credentials);

/**
 * @brief Encode a credentials response
 * 
 * @param authentication_result [in] The authentication result
 * @param out_dgrm [out] The encoded datagram
 * @return err_t 
 */
err_t protocol_authentication_response_encode(const uint8_t authentication_result, udp_dgram_t* out_dgrm);

/**
 * @brief Decodes a credentials response
 * 
 * @param in_dgrm [in] The datagram to decode
 * @param authentication_result [out] The authentication result
 * @return err_t 
 */
err_t protocol_authentication_response_decode(const struct __message_t* in_dgrm, uint8_t* authentication_result);

/**
 * @brief Encode a course information lookup request
 * 
 * @param course_code [in] The course to lookup information for
 * @param course_code_len [in] The length of the course id
 * @param category [in] The lookup category
 * @param out_dgrm [out] The encoded datagram
 * 
 * @return err_t 
 */
err_t protocol_courses_lookup_single_request_encode(const char* course_code, const uint8_t course_code_len, const courses_lookup_category_t category, struct __message_t* out_dgrm);

/**
 * @brief Decode a course information lookup request
 * 
 * @param in_dgrm [in] The datagram to decode
 * @param course_code [out] The course to lookup information for
 * @param course_code_len [out] The length of the course id
 * @param category [out] The lookup category
 * 
 * @return err_t 
 */
err_t protocol_courses_lookup_single_request_decode(const struct __message_t* in_dgrm, char* course_code, uint8_t* course_code_len, courses_lookup_category_t* category);

/**
 * @brief Encode a course information lookup response
 * 
 * @param course_code [in] The course to lookup information for
 * @param course_code_len [in] The length of the course id
 * @param category [in] The lookup category
 * @param information [in] The rqeuested information to send
 * @param information_len [in] The length of the information
 * @param out_dgrm [out] The encoded datagram
 * 
 * @return err_t 
 */
err_t protocol_courses_lookup_single_response_encode(const char* course_code, const uint8_t course_code_len, const courses_lookup_category_t category, const uint8_t* information, const uint8_t information_len, struct __message_t* out_dgrm);

/**
 * @brief Decode a course information lookup response
 * 
 * @param in_dgrm [in] The datagram to decode
 * @param course_code [out] The course to lookup information for
 * @param course_code_len [out] The length of the course id
 * @param category [out] The lookup category
 * @param information [out] The information
 * @param information_len [out] The length of the information
 * 
 * @return err_t 
 */
err_t protocol_courses_lookup_single_response_decode(const udp_dgram_t* in_dgrm, char* course_code, uint8_t* course_code_len, courses_lookup_category_t* category, uint8_t* information, uint8_t* information_len);

/**
 * @brief Encode a course detail lookup request
 * 
 * @param course_code [in] The course to lookup information for
 * @param course_code_len [in] The length of the course id
 * @param out_dgrm [out] The encoded datagram
 * 
 * @return err_t 
 */
err_t protocol_courses_lookup_detail_request_encode(const uint8_t* course_code, const uint8_t course_code_len, udp_dgram_t* out_dgrm);

/**
 * @brief Decode a course detail lookup request
 * 
 * @param in_dgrm [in] The datagram to decode
 * @param course_code [out] The course to lookup information for
 * @param course_code_len [out] The length of the course id
 * 
 * @return err_t 
 */
err_t protocol_courses_lookup_detail_request_decode(const udp_dgram_t* in_dgrm, uint8_t* course_code, uint8_t* course_code_len);

/**
 * @brief Encode a course detail lookup response
 * 
 * @param course [in] The course to encode
 * @param out_dgrm [out] The encoded datagram
 * 
 * @return err_t 
 */
err_t protocol_courses_lookup_detail_response_encode(const course_t* course, udp_dgram_t* out_dgrm);

/**
 * @brief Decode a course detail lookup response
*/
err_t protocol_courses_lookup_detail_response_decode(const udp_dgram_t* in_dgrm, course_t* course);

/**
 * @brief Encode a course list lookup request
 * 
 * @param course_count [in] The number of courses to lookup
 * @param course_codes_buffer [in] The buffer with courses to lookup
 * @param course_codes_buffer_len [in] The length of the course codes buffer
 * @param out_dgrm [out] The encoded datagram
 * 
 * @return err_t 
 */
err_t protocol_courses_lookup_multiple_request_encode(const uint8_t course_count, const uint8_t* course_codes_buffer, const uint8_t course_codes_buffer_len, tcp_sgmnt_t* out_sgmnt);

typedef void (*single_course_code_handler_t)(const uint8_t idx, const char* course_code, const uint8_t course_code_len);

/**
 * @brief Decode a course list lookup request
 * 
 * @param in_dgrm [in] The datagram to decode
 * @param course_count [out] The number of courses to lookup
 * @param handler [callback] Callback function called for each course code
 * 
 * @return err_t 
 */
err_t protocol_courses_lookup_multiple_request_decode(const udp_dgram_t* in_dgrm, uint8_t* course_count, single_course_code_handler_t handler);

/**
 * @brief Encode a course list lookup response
 * 
 * @param courses [in] The courses to encode
 * @param out_dgrm [out] The encoded datagram
 * 
 * @return err_t 
 */
err_t protocol_courses_lookup_multiple_response_encode(const course_t* courses, udp_dgram_t* out_dgrm);

/**
 * @brief Decode a course list lookup response
 * 
 * @param in_dgrm [in] The datagram to decode
 * @param courses [out] The courses linked list
 * 
 * @note The caller needs to call protocol_courses_lookup_multiple_response_decode_dealloc() to avoid memory leaks
 * 
 * @return err_t 
 */
err_t protocol_courses_lookup_multiple_response_decode(const udp_dgram_t* in_dgrm, course_t** courses);

/**
 * @brief Deallocate the memory used by a course list lookup response
 * 
 * @param course [in] The courses linked list
 */
void protocol_courses_lookup_multiple_response_decode_dealloc(course_t* course);

/**
 * @brief Encode a course lookup error
 * 
 * @param error_code [in] The error code
 * @param out_dgrm [out] The encoded datagram
 * 
 * @return err_t 
 */
err_t protocol_courses_error_encode(const err_t error_code, udp_dgram_t* out_dgrm);

/**
 * @brief Decode a course lookup error
 * 
 * @param in_dgrm [in] The datagram to decode
 * @param error_code [out] The error code
 */
err_t protocol_courses_error_decode(const udp_dgram_t* in_dgrm, err_t* error_code);

#endif // PROTOCOL_H
