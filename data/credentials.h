#ifndef CREDENTIALS_H
#define CREDENTIALS_H

#include <stdint.h>

#include "../utils/error.h"

#define CREDENTIALS_MAX_USERNAME_LEN 32
#define CREDENTIALS_MAX_PASSWORD_LEN 32

typedef struct __credentials_t {
    uint8_t username[CREDENTIALS_MAX_USERNAME_LEN];
    uint8_t password[CREDENTIALS_MAX_PASSWORD_LEN];
    uint8_t username_len;
    uint8_t password_len;
    struct __credentials_t* next;
} credentials_t;


err_t credentials_encode(const credentials_t* in_credentials, uint8_t* out_buffer, const size_t out_buffer_size, uint8_t* out_buffer_len);
err_t credentials_decode(credentials_t* out_credentials, const uint8_t* in_buffer, const uint8_t in_buffer_len);
err_t credentials_encrypt(const credentials_t* in_credentials, credentials_t* out_credentials);
err_t credentials_validate(const credentials_t* credentials_db, const credentials_t* credential);

credentials_t* credentials_init(const char* filename);
void credentials_free(credentials_t* credentials);

void credentials_print(const credentials_t* credentials);


#endif //CREDENTIALS_H
