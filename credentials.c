#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "credentials.h"
#include "constants.h"
#include "log.h"
#include "utils.h"

LOG_TAG(credentials);

static uint8_t shift_char(uint8_t c, int shift) {
    if (c >= 'A' && c <= 'Z') {
        c = (c - 'A' + shift) % 26 + 'A';
    } else if (c >= 'a' && c <= 'z') {
        c = (c - 'a' + shift) % 26 + 'a';
    } else if (c >= '0' && c <= '9') {
        c = (c - '0' + shift) % 10 + '0';
    }
    return c;
}

static err_t encrypt_buffer(const uint8_t* plaintext, const size_t plaintext_len, uint8_t* ciphertext_buffer, const size_t ciphertext_buffer_len, uint8_t* ciphertext_len) {
    if (ciphertext_buffer_len < plaintext_len) {
        LOG_WARN("ciphertext buffer must be equal to or larger than %d bytes", (int) plaintext_len);
        return ERR_INVALID_PARAMETERS;
    }

    for (int i = 0; i < plaintext_len; i++) {
        ciphertext_buffer[i] = shift_char(plaintext[i], CREDENTIALS_ENC_CHAR_SHIFT_COUNT);
    }

    *ciphertext_len = plaintext_len;

    return ERR_OK;
}

err_t credentials_encrypt(const credentials_t* in_credentials, credentials_t* out_credentials) {

    if (in_credentials == NULL || out_credentials == NULL) {
        return ERR_INVALID_PARAMETERS;
    }

    if (encrypt_buffer(in_credentials->username, in_credentials->username_len, out_credentials->username, sizeof(out_credentials->username), &out_credentials->username_len) != 0) {
        return ERR_INVALID_PARAMETERS;
    }

    if (encrypt_buffer(in_credentials->password, in_credentials->password_len, out_credentials->password, sizeof(out_credentials->password), &out_credentials->password_len) != 0) {
        return ERR_INVALID_PARAMETERS;
    }

    return ERR_OK;
}

err_t credentials_validate(const credentials_t* credentials_db, const credentials_t* credential) {
    if (credentials_db == NULL || credential == NULL) {
        return ERR_INVALID_PARAMETERS;
    }
    while (credentials_db != NULL) {
        if (strcmp((const char*) credentials_db->username, (const char*) credential->username) == 0) {
            if(strcmp((const char*) credentials_db->password, (const char*) credential->password) == 0) {
                return ERR_OK;
            } else {
                return ERR_CREDENTIALS_PASSWORD_MISMATCH;
            }
        }
        credentials_db = credentials_db->next;
    }
    return ERR_CREDENTIALS_USER_NOT_FOUND;
}
