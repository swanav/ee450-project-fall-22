#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include "networking.h"
#include "utils/constants.h"


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

// Create UDP or TCP packet based on given buffer
void protocol_encode(struct __message_t* message, uint8_t type, uint8_t flags, uint16_t payload_len, uint8_t* payload);

// Recreate buffer from given UDP or TCP packet
void protocol_decode(const struct __message_t* message, request_type_t* request_type, uint8_t* flags, uint16_t *out_data_len, const uint16_t out_data_size, uint8_t* out_data);

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
err_t protocol_authentication_response_decode(const udp_dgram_t* in_dgrm, uint8_t* authentication_result);


#endif // PROTOCOL_H
