#ifndef CREDENTIALS_H
#define CREDENTIALS_H

#include <stdint.h>

#define CREDENTIALS_MAX_USERNAME_LEN 32
#define CREDENTIALS_MAX_PASSWORD_LEN 32

typedef struct __credentials_t {
    uint8_t username[CREDENTIALS_MAX_USERNAME_LEN];
    uint8_t password[CREDENTIALS_MAX_PASSWORD_LEN];
    uint8_t username_len;
    uint8_t password_len;
    struct __credentials_t* next;
} credentials_t;

typedef int8_t err_t;

#define ERR_OK                              0x00
#define ERR_INVALID_PARAMETERS              0x01

#define ERR_REQ_BASE                        0x10
#define ERR_REQ_INVALID                     (ERR_REQ_BASE | ERR_INVALID_PARAMETERS)

#define ERR_RESP_BASE                       0x20
#define ERR_RESP_INVALID                    (ERR_RESP_BASE | ERR_INVALID_PARAMETERS)

#define ERR_CREDENTIALS_BASE                0x30
#define ERR_CREDENTIALS_OK                  (ERR_CREDENTIALS_BASE | 0x01)
#define ERR_CREDENTIALS_USER_NOT_FOUND      (ERR_CREDENTIALS_BASE | 0x02)
#define ERR_CREDENTIALS_PASSWORD_MISMATCH   (ERR_CREDENTIALS_BASE | 0x03)
#define ERR_CREDENTIALS_INVALID_REQUEST     (ERR_CREDENTIALS_BASE | 0x04)
#define ERR_CREDENTIALS_INVALID_PARAMETERS  (ERR_CREDENTIALS_BASE | 0x05)


err_t credentials_encode(const credentials_t* in_credentials, uint8_t* out_buffer, const uint8_t out_buffer_size, uint8_t* out_buffer_len);
err_t credentials_decode(credentials_t* out_credentials, const uint8_t* in_buffer, const uint8_t in_buffer_len);
err_t credentials_encrypt(const credentials_t* in_credentials, credentials_t* out_credentials);
err_t credentials_validate(const credentials_t* credentials_db, const credentials_t* credential);

credentials_t* credentials_init(const char* filename);
void credentials_free(credentials_t* credentials);

void credentials_print(const credentials_t* credentials);


#endif //CREDENTIALS_H
