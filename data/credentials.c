#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "credentials.h"
#include "log.h"
#include "utils.h"

#define CREDENTIALS_USERNAME_LEN_OFFSET 0
#define CREDENTIALS_PASSWORD_LEN_OFFSET 1

#define CREDENTIALS_HEADER_LEN 2
#define CREDENTIALS_USERNAME_LEN(b) b[CREDENTIALS_USERNAME_LEN_OFFSET]
#define CREDENTIALS_PASSWORD_LEN(b) b[CREDENTIALS_PASSWORD_LEN_OFFSET]
#define CREDENTIALS_TOTAL_LEN(b) CREDENTIALS_HEADER_LEN + CREDENTIALS_USERNAME_LEN(b) + CREDENTIALS_PASSWORD_LEN(b)

#define BUFFER_ENCRYPTION_CHAR_SHIFT_COUNT 4

// Used by client to send credentials to serverM
// Used by serverM to encode the credentials to send to serverC
err_t credentials_encode(const credentials_t* in_credentials, uint8_t* out_buffer, const size_t out_buffer_size, uint8_t* out_buffer_len) {
    if (in_credentials == NULL || out_buffer == NULL || out_buffer_len == NULL || out_buffer_size < CREDENTIALS_HEADER_LEN + in_credentials->username_len + in_credentials->password_len) {
        return ERR_INVALID_PARAMETERS;
    }

    uint8_t offset = 0;
    out_buffer[offset++] = in_credentials->username_len;
    out_buffer[offset++] = in_credentials->password_len;
    memcpy(out_buffer + offset, in_credentials->username, in_credentials->username_len);
    offset += in_credentials->username_len;
    memcpy(out_buffer + offset, in_credentials->password, in_credentials->password_len);

    *out_buffer_len = offset + in_credentials->password_len;

    return ERR_OK;
}

// Used by serverM to decode the credentials from client
// Used by serverC to decode the credentials from serverM
err_t credentials_decode(credentials_t* out_credentials, const uint8_t* in_buffer, const uint8_t in_buffer_len) {

    if (out_credentials == NULL || (in_buffer_len < CREDENTIALS_HEADER_LEN) || (in_buffer_len < CREDENTIALS_TOTAL_LEN(in_buffer))) {
        return ERR_INVALID_PARAMETERS;
    }

    out_credentials->username_len = CREDENTIALS_USERNAME_LEN(in_buffer);
    out_credentials->password_len = CREDENTIALS_PASSWORD_LEN(in_buffer);
    uint8_t offset = CREDENTIALS_HEADER_LEN;
    memcpy(out_credentials->username, in_buffer + offset, out_credentials->username_len);
    offset += out_credentials->username_len;
    memcpy(out_credentials->password, in_buffer + offset, out_credentials->password_len);
    return ERR_OK;
}

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
        LOGW("ciphertext buffer must be equal to or larger than %d bytes", (int) plaintext_len);
        return ERR_INVALID_PARAMETERS;
    }

    for (int i = 0; i < plaintext_len; i++) {
        ciphertext_buffer[i] = shift_char(plaintext[i], BUFFER_ENCRYPTION_CHAR_SHIFT_COUNT);
    }

    *ciphertext_len = plaintext_len;

    return ERR_OK;
}


// Used by serverM to encrypt the credentials
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

uint8_t credentials_len(const credentials_t* credentials) {
    return CREDENTIALS_HEADER_LEN + credentials->username_len + credentials->password_len;
}



credentials_t* credentials_init(const char* filename) {

    credentials_t* credentials = NULL;

    if (filename == NULL) {
        return NULL;
    }

    FILE* fp = csv_open(filename);
    char line[1024];

    if (fp == NULL) return NULL;

    while (fgets(line, sizeof(line), fp)) {
        char* token = strtok(line, ",\r\n");
        if (token == NULL) {
            continue;
        }

        credentials_t* ptr = malloc(sizeof(credentials_t));
        if (ptr == NULL) {
            LOGEM("Failed to allocate memory for credentials");
            return NULL;
        }
        bzero(ptr, sizeof(credentials_t));

        ptr->username_len = strlen(token);
        if (ptr->username_len > CREDENTIALS_MAX_USERNAME_LEN) {
            LOGEM("Username is too long");
            return NULL;
        }
        memcpy(ptr->username, token, ptr->username_len);

        token = strtok(NULL, ",\r\n");
        if (token == NULL) {
            continue;
        }

        ptr->password_len = strlen(token);
        if (ptr->password_len > CREDENTIALS_MAX_PASSWORD_LEN) {
            LOGEM("Password is too long");
            return NULL;
        }
        memcpy(ptr->password, token, ptr->password_len);

        if (credentials != NULL) {
            ptr->next = credentials;
        }

        credentials = ptr;
    }

    csv_close(fp);

    return credentials;
}

void credentials_free(credentials_t* credentials) {
    while (credentials != NULL) {
        credentials_t* ptr = credentials;
        credentials = credentials->next;
        free(ptr);
    }
}

void credentials_print(const credentials_t* credentials) {
    while (credentials != NULL) {
        LOGV("Username: %s, Password: %s", credentials->username, credentials->password);
        credentials = credentials->next;
    }
}

err_t credentials_validate(const credentials_t* credentials_db, const credentials_t* credential) {
    if (credentials_db == NULL || credential == NULL) {
        return ERR_INVALID_PARAMETERS;
    }
    while (credentials_db != NULL) {
        if (strcmp((const char*) credentials_db->username, (const char*) credential->username) == 0) {
            if(strcmp((const char*) credentials_db->password, (const char*) credential->password) == 0) {
                return ERR_CREDENTIALS_OK;
            } else {
                return ERR_CREDENTIALS_PASSWORD_MISMATCH;
            }
        }
        credentials_db = credentials_db->next;
    }
    return ERR_CREDENTIALS_USER_NOT_FOUND;
}
