#include "database.h"

#include <stdio.h>
#include <string.h>

#include "log.h"
#include "protocol.h"
#include "utils.h"

LOG_TAG(database);

#if defined(SERVER_CS) || defined(SERVER_EE)
course_t* database_courses_lookup(const course_t* db, const char* course_code) {
    const course_t* entry = db;
    while (entry != NULL) {
        if (strcasecmp(entry->course_code, course_code) == 0) {
            return (course_t*)entry;
        }
        entry = entry->next;
    }
    return NULL;
}

err_t database_courses_lookup_info(const course_t* course, courses_lookup_category_t category, uint8_t* info_buf, size_t info_buf_size, size_t* info_len) {
    if (course == NULL || info_buf == NULL || info_len == NULL || category >= COURSES_LOOKUP_CATEGORY_INVALID) {
        return ERR_INVALID_PARAMETERS;
    }

    switch (category) {
        case COURSES_LOOKUP_CATEGORY_COURSE_CODE:
            strncpy((char*)info_buf, course->course_code, info_buf_size);
            *info_len = strlen(course->course_code);
            break;
        case COURSES_LOOKUP_CATEGORY_CREDITS:
            snprintf((char*)info_buf, info_buf_size, "%d", course->credits);
            *info_len = strlen((char*)info_buf);
            break;
        case COURSES_LOOKUP_CATEGORY_PROFESSOR:
            strncpy((char*)info_buf, course->professor, info_buf_size);
            *info_len = strlen(course->professor);
            break;
        case COURSES_LOOKUP_CATEGORY_DAYS:
            strncpy((char*)info_buf, course->days, info_buf_size);
            *info_len = strlen(course->days);
            break;
        case COURSES_LOOKUP_CATEGORY_COURSE_NAME:
            strncpy((char*)info_buf, course->course_name, info_buf_size);
            *info_len = strlen(course->course_name);
            break;
        default:
            return ERR_INVALID_PARAMETERS;
    }
    return ERR_OK;
}

#endif // SERVER_CS || SERVER_EE

#if defined(CLIENT)
courses_lookup_category_t database_courses_lookup_category_from_string(const char* category) {
    if ((strcasecmp(category, "CourseCode") == 0) || (strcasecmp(category, "Course Code") == 0)) {
        return COURSES_LOOKUP_CATEGORY_COURSE_CODE;
    } else if ((strcasecmp(category, "Credits") == 0) || (strcasecmp(category, "Credit") == 0)) {
        return COURSES_LOOKUP_CATEGORY_CREDITS;
    } else if (strcasecmp(category, "Professor") == 0) {
        return COURSES_LOOKUP_CATEGORY_PROFESSOR;
    } else if (strcasecmp(category, "Days") == 0) {
        return COURSES_LOOKUP_CATEGORY_DAYS;
    } else if ((strcasecmp(category, "CourseName") == 0) || (strcasecmp(category, "Course Name") == 0)) {
        return COURSES_LOOKUP_CATEGORY_COURSE_NAME;
    } else {
        return COURSES_LOOKUP_CATEGORY_INVALID;
    }
}
#endif // CLIENT

#if defined(CLIENT) || defined(SERVER_M) || defined(SERVER_CS) || defined(SERVER_EE)
char* database_courses_category_string_from_enum(courses_lookup_category_t category) {
    switch (category) {
        case COURSES_LOOKUP_CATEGORY_COURSE_CODE:
            return "Course Code";
        case COURSES_LOOKUP_CATEGORY_CREDITS:
            return "Credits";
        case COURSES_LOOKUP_CATEGORY_PROFESSOR:
            return "Professor";
        case COURSES_LOOKUP_CATEGORY_DAYS:
            return "Days";
        case COURSES_LOOKUP_CATEGORY_COURSE_NAME:
            return "Course Name";
        default:
            return "Invalid";
    }
}
#endif // CLIENT || SERVER_M || SERVER_CS || SERVER_EE

#if defined(SERVER_M)
// Inspired from https://stackoverflow.com/questions/5224990/shift-a-letter-down-the-alphabet
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

err_t database_credentials_encrypt(const credentials_t* in_credentials, credentials_t* out_credentials) {

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
#endif // SERVER_M

#if defined(SERVER_C)
err_t database_credentials_validate(const credentials_t* credentials_db, const credentials_t* credential) {
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
#endif // SERVER_C
