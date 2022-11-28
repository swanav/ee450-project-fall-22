#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "protocol.h"
#include "utils.h"


static uint16_t protocol_get_payload_len(const struct __message_t* message) {
    return message->data_len < REQUEST_RESPONSE_HEADER_LEN ? REQUEST_RESPONSE_INVALID_TYPE : message->data[REQUEST_RESPONSE_PAYLOAD_LEN_OFFSET_1] | (message->data[REQUEST_RESPONSE_PAYLOAD_LEN_OFFSET_2] << 8);
}

static uint8_t protocol_get_flags(const struct __message_t* message) {
    return message->data_len < REQUEST_RESPONSE_HEADER_LEN ? 0 : message->data[REQUEST_RESPONSE_FLAGS_OFFSET];
}

static void protocol_encode(struct __message_t* message, const uint8_t type, const uint8_t flags, const uint16_t payload_len, const uint8_t* payload) {
    if (sizeof(message->data) >= payload_len + REQUEST_RESPONSE_HEADER_LEN) {
        message->data[REQUEST_RESPONSE_TYPE_OFFSET] = type;
        message->data[REQUEST_RESPONSE_FLAGS_OFFSET] = flags;
        message->data[REQUEST_RESPONSE_PAYLOAD_LEN_OFFSET_1] = payload_len & 0xFF;
        message->data[REQUEST_RESPONSE_PAYLOAD_LEN_OFFSET_2] = payload_len >> 8;
        memcpy(message->data + REQUEST_RESPONSE_HEADER_LEN, payload, payload_len);
        message->data_len = REQUEST_RESPONSE_HEADER_LEN + payload_len;
    }
}

static void protocol_decode(const struct __message_t* message, request_type_t* request_type, uint8_t* flags, uint16_t *out_data_len, const uint16_t out_data_size, uint8_t* out_data) {
    if (request_type) {
        *request_type = protocol_get_request_type(message);
    }
    if (flags) {
        *flags = protocol_get_flags(message);
    }
    if (out_data_len && out_data) {
        *out_data_len = protocol_get_payload_len(message);
        if (*out_data_len > out_data_size) {
            *out_data_len = out_data_size;
        }
        memcpy(out_data, message->data + REQUEST_RESPONSE_HEADER_LEN, *out_data_len);
    }
}

request_type_t protocol_get_request_type(const struct __message_t* message) {
    return message->data_len < REQUEST_RESPONSE_HEADER_LEN ? REQUEST_RESPONSE_INVALID_TYPE : message->data[REQUEST_RESPONSE_TYPE_OFFSET];
}

err_t protocol_authentication_request_encode(const credentials_t* credentials, struct __message_t* out_dgrm) {
    if (credentials == NULL || out_dgrm == NULL) {
        return ERR_INVALID_PARAMETERS;
    }

    uint8_t credentials_buffer[sizeof(credentials_t)];
    uint8_t offset = 0;

    credentials_buffer[offset++] = credentials->username_len;
    credentials_buffer[offset++] = credentials->password_len;
    memcpy(credentials_buffer + offset, credentials->username, credentials->username_len);
    offset += credentials->username_len;
    memcpy(credentials_buffer + offset, credentials->password, credentials->password_len);
    offset += credentials->password_len;

    protocol_encode(out_dgrm, REQUEST_TYPE_AUTH, 0, offset, credentials_buffer);

    return ERR_OK;
}

err_t protocol_authentication_request_decode(const struct __message_t* in_dgrm, credentials_t* credentials) {

    if (!in_dgrm || !credentials) {
        return ERR_INVALID_PARAMETERS;
    }

    uint8_t buffer[REQUEST_RESPONSE_HEADER_LEN + sizeof(credentials_t)];
    uint16_t buffer_size = sizeof(buffer);

    request_type_t type = protocol_get_request_type(in_dgrm);
    if (type != REQUEST_TYPE_AUTH) {
        return ERR_INVALID_PARAMETERS;
    }

    protocol_decode(in_dgrm, NULL, NULL, &buffer_size, sizeof(buffer), buffer);

    credentials->username_len = CREDENTIALS_USERNAME_LEN(buffer);
    credentials->password_len = CREDENTIALS_PASSWORD_LEN(buffer);
    uint8_t offset = CREDENTIALS_HEADER_LEN;
    memcpy(credentials->username, buffer + offset, credentials->username_len);
    offset += credentials->username_len;
    memcpy(credentials->password, buffer + offset, credentials->password_len);
    return ERR_OK;
}

err_t protocol_authentication_response_encode(const uint8_t authentication_result, struct __message_t* out_dgrm) {
    if (out_dgrm == NULL) {
        return ERR_INVALID_PARAMETERS;
    }
    protocol_encode(out_dgrm, RESPONSE_TYPE_AUTH, authentication_result, 0, NULL);
    return ERR_OK;
}

err_t protocol_authentication_response_decode(const struct __message_t* in_dgrm, uint8_t* authentication_result) {
    if (in_dgrm == NULL || authentication_result == NULL) {
        return ERR_INVALID_PARAMETERS;
    }

    uint8_t buffer[REQUEST_RESPONSE_HEADER_LEN];

    request_type_t type = protocol_get_request_type(in_dgrm);
    if (type != RESPONSE_TYPE_AUTH) {
        return ERR_INVALID_PARAMETERS;
    }

    protocol_decode(in_dgrm, NULL, authentication_result, NULL, 0, buffer);

    return ERR_OK;
}

err_t protocol_courses_lookup_single_request_encode(const char* course_code, const uint8_t course_code_len, const courses_lookup_category_t category, struct __message_t* out_dgrm) {
    if (course_code == NULL || course_code_len == 0 || category < COURSES_LOOKUP_CATEGORY_COURSE_CODE || category > COURSES_LOOKUP_CATEGORY_INVALID || out_dgrm == NULL) {
        return ERR_INVALID_PARAMETERS;
    }
    protocol_encode(out_dgrm, REQUEST_TYPE_COURSES_SINGLE_LOOKUP, category, course_code_len, (uint8_t*) course_code);
    return ERR_OK;
}

err_t protocol_courses_lookup_single_request_decode(const struct __message_t* in_dgrm, char* course_code, uint8_t* course_code_len, courses_lookup_category_t* category) {
    if (in_dgrm == NULL || course_code == NULL) {
        return ERR_INVALID_PARAMETERS;
    }

    if (protocol_get_request_type(in_dgrm) != REQUEST_TYPE_COURSES_SINGLE_LOOKUP) {
        return ERR_INVALID_PARAMETERS;
    }

    uint8_t flags = 0;
    uint16_t olen = 0;
    uint16_t size = course_code_len ? *course_code_len : 0;
    protocol_decode(in_dgrm, NULL, &flags, &olen, size, (uint8_t*) course_code);

    if (course_code_len) {
        *course_code_len = olen;
    }
    if (category) {
        *category = flags;
    }
    return ERR_OK;
}

err_t protocol_courses_lookup_single_response_encode(const char* course_code, const uint8_t course_code_len, const courses_lookup_category_t category, const uint8_t* information, const uint8_t information_len, struct __message_t* out_dgrm) {
    if (course_code == NULL || course_code_len == 0 || category < COURSES_LOOKUP_CATEGORY_COURSE_CODE || category > COURSES_LOOKUP_CATEGORY_INVALID || information == NULL || information_len == 0 || out_dgrm == NULL) {
        return ERR_INVALID_PARAMETERS;
    }

    uint8_t* buffer = malloc(sizeof(course_code_len) + course_code_len + sizeof(information_len) + information_len);
    if (buffer) {
        uint8_t offset = 0;

        buffer[offset++] = course_code_len;
        memcpy(buffer + offset, course_code, course_code_len);
        offset += course_code_len;
        buffer[offset++] = information_len;
        memcpy(buffer + offset, information, information_len);
        offset += information_len;

        protocol_encode(out_dgrm, RESPONSE_TYPE_COURSES_SINGLE_LOOKUP, category, offset, (uint8_t*) buffer);
        free(buffer);
        return ERR_OK;
    }
    return ERR_OUT_OF_MEMORY;
}

err_t protocol_courses_lookup_single_response_decode(const struct __message_t* in_dgrm, char* course_code, uint8_t* course_code_len, courses_lookup_category_t* category, uint8_t* information, uint8_t* information_len) {
    if (in_dgrm == NULL || course_code == NULL || course_code_len == NULL || category == NULL || information == NULL || information_len == NULL) {
        return ERR_INVALID_PARAMETERS;
    }

    if (protocol_get_request_type(in_dgrm) != RESPONSE_TYPE_COURSES_SINGLE_LOOKUP) {
        return ERR_INVALID_PARAMETERS;
    }

    uint8_t flags = 0;
    uint16_t olen = 0;
    uint8_t buffer[128];
    protocol_decode(in_dgrm, NULL, &flags, &olen, sizeof(buffer), buffer);

    uint8_t offset = 0;
    *course_code_len = buffer[offset++];
    memcpy(course_code, buffer + offset, *course_code_len);
    offset += *course_code_len;
    *information_len = buffer[offset++];
    memcpy(information, buffer + offset, *information_len);
    offset += *information_len;

    *category = flags;
    return ERR_OK;
}

err_t protocol_courses_lookup_detail_request_encode(const uint8_t* course_code, const uint8_t course_code_len, struct __message_t* out_dgrm) {
    if (course_code == NULL || out_dgrm == NULL) {
        return ERR_INVALID_PARAMETERS;
    }
    protocol_encode(out_dgrm, REQUEST_TYPE_COURSES_DETAIL_LOOKUP, 0, course_code_len, course_code);
    return ERR_OK;
}

err_t protocol_courses_lookup_detail_request_decode(const struct __message_t* in_dgrm, uint8_t* course_code, uint8_t* course_code_len) {
    if (in_dgrm == NULL || course_code == NULL) {
        return ERR_INVALID_PARAMETERS;
    }

    if (protocol_get_request_type(in_dgrm) != REQUEST_TYPE_COURSES_DETAIL_LOOKUP) {
        return ERR_INVALID_PARAMETERS;
    }

    uint16_t size = course_code_len ? *course_code_len : 0;
    uint16_t olen = 0;
    protocol_decode(in_dgrm, NULL, NULL, &olen, size, course_code);
    course_code[olen] = '\0';

    *course_code_len = olen;

    return ERR_OK;
}

err_t protocol_courses_lookup_detail_response_encode(const course_t* course, struct __message_t* out_dgrm) {
    if (course == NULL || out_dgrm == NULL) {
        return ERR_INVALID_PARAMETERS;
    }

    uint8_t buffer[256];
    size_t offset = 0;
    
    buffer[offset++] = strlen(course->course_code);
    memcpy(buffer + offset, course->course_code, strlen(course->course_code));
    offset += strlen(course->course_code);

    buffer[offset++] = strlen(course->course_name);
    memcpy(buffer + offset, course->course_name, strlen(course->course_name));
    offset += strlen(course->course_name);

    buffer[offset++] = strlen(course->professor);
    memcpy(buffer + offset, course->professor, strlen(course->professor));
    offset += strlen(course->professor);

    buffer[offset++] = strlen(course->days);
    memcpy(buffer + offset, course->days, strlen(course->days));
    offset += strlen(course->days);

    buffer[offset++] = 1;
    buffer[offset++] = course->credits;

    protocol_encode(out_dgrm, RESPONSE_TYPE_COURSES_DETAIL_LOOKUP, 0, offset, buffer);

    return ERR_OK;
}

err_t protocol_courses_lookup_detail_response_decode(const struct __message_t* in_dgrm, course_t* course) {
    if (in_dgrm == NULL || course == NULL) {
        return ERR_INVALID_PARAMETERS;
    }

    uint8_t buffer[128];
    uint16_t buffer_len = 0;

    protocol_decode(in_dgrm, NULL, NULL, &buffer_len, sizeof(buffer), buffer);

    uint8_t len = 0;
    uint8_t offset = 0;

    len = buffer[offset++];
    memcpy(course->course_code, buffer + offset, len);
    course->course_code[len] = '\0';
    offset += len;

    len = buffer[offset++];
    memcpy(course->course_name, buffer + offset, len);
    course->course_name[len] = '\0';
    offset += len;

    len = buffer[offset++];
    memcpy(course->professor, buffer + offset, len);
    course->professor[len] = '\0';
    offset += len;

    len = buffer[offset++];
    memcpy(course->days, buffer + offset, len);
    course->days[len] = '\0';
    offset += len;

    course->credits = buffer[++offset];

    return ERR_OK;
}

err_t protocol_courses_lookup_multiple_request_encode(const uint8_t course_count, const uint8_t* course_codes_buffer, const uint8_t course_codes_buffer_len, struct __message_t* out_sgmnt) {
    if (course_codes_buffer == NULL || out_sgmnt == NULL) {
        return ERR_INVALID_PARAMETERS;
    }

    uint8_t buffer[128];
    uint8_t buffer_len = 0;

    const uint8_t* ptr = course_codes_buffer;
    buffer[0] = course_count;

    buffer_len += 1;
    for (uint8_t i = 0; i < course_count; i++) {
        int length = utils_get_word_length((char*) ptr);
        buffer[buffer_len] = length;
        buffer_len += 1;
        memcpy(buffer + buffer_len, ptr, length);
        buffer_len += length;
        ptr += length;
        ptr += 1; // skip space
    }

    protocol_encode(out_sgmnt, REQUEST_TYPE_COURSES_MULTI_LOOKUP, 0, buffer_len, buffer);
    return ERR_OK;
}

err_t protocol_courses_lookup_multiple_request_decode(const struct __message_t* in_dgrm, uint8_t* course_count, single_course_code_handler_t handler) {
    if (in_dgrm == NULL || handler == NULL) {
        return ERR_INVALID_PARAMETERS;
    }

    if (protocol_get_request_type(in_dgrm) != REQUEST_TYPE_COURSES_MULTI_LOOKUP) {
        return ERR_INVALID_PARAMETERS;
    }

    uint8_t buffer[128];
    uint16_t buffer_len = 0;

    protocol_decode(in_dgrm, NULL, NULL, &buffer_len, sizeof(buffer), buffer);

    uint8_t offset = 0;
    *course_count = buffer[offset++];
    for (uint8_t i = 0; i < *course_count; i++) {
        uint8_t len = buffer[offset++];
        handler(i, (char*) buffer + offset, len);
        offset += len;
    }

    return ERR_OK;
}

static uint8_t course_details_encode(course_t* course, uint8_t* buffer, uint8_t buffer_size) {
    uint8_t buffer_offset = 1;
    uint8_t course_code_len = strlen(course->course_code);
    uint8_t course_name_len = strlen(course->course_name);
    uint8_t professor_len = strlen(course->professor);
    uint8_t days_len = strlen(course->days);

    memcpy(buffer + buffer_offset, &course_code_len, 1);
    buffer_offset += 1;
    memcpy(buffer + buffer_offset, course->course_code, course_code_len);
    buffer_offset += course_code_len;

    memcpy(buffer + buffer_offset, &course_name_len, 1);
    buffer_offset += 1;
    memcpy(buffer + buffer_offset, course->course_name, course_name_len);
    buffer_offset += course_name_len;

    memcpy(buffer + buffer_offset, &professor_len, 1);
    buffer_offset += 1;
    memcpy(buffer + buffer_offset, course->professor, professor_len);
    buffer_offset += professor_len;

    memcpy(buffer + buffer_offset, &days_len, 1);
    buffer_offset += 1;
    memcpy(buffer + buffer_offset, course->days, days_len);
    buffer_offset += days_len;

    memcpy(buffer + buffer_offset, &(course->credits), 1);
    buffer_offset += 1;

    buffer[0] = buffer_offset;
    return buffer_offset;
}

err_t protocol_courses_lookup_multiple_response_encode(const course_t* courses, struct __message_t* out_dgrm) {
    if (out_dgrm == NULL || courses == NULL) {
        return ERR_INVALID_PARAMETERS;
    }
    
    uint8_t buffer[1016];
    uint16_t buffer_offset = 0;

    course_t* course_ptr = (course_t*) courses;
    uint8_t course_count = 0;
    while(course_ptr) {
        buffer_offset += course_details_encode(course_ptr, buffer + buffer_offset, sizeof(buffer) - buffer_offset);
        course_count++;
        course_ptr = course_ptr->next;
    }

    protocol_encode(out_dgrm, RESPONSE_TYPE_COURSES_MULTI_LOOKUP, course_count, buffer_offset, buffer);

    return ERR_OK;

}

static course_t* course_details_decode(uint8_t* buffer, uint8_t buffer_size, uint16_t* bytes_read) {
    course_t* course = malloc(sizeof(course_t));
    bzero(course, sizeof(course_t));

    uint8_t buffer_offset = 1;
    uint8_t course_code_len = buffer[buffer_offset++];
    memcpy(course->course_code, buffer + buffer_offset, course_code_len);
    buffer_offset += course_code_len;

    uint8_t course_name_len = buffer[buffer_offset++];
    memcpy(course->course_name, buffer + buffer_offset, course_name_len);
    buffer_offset += course_name_len;

    uint8_t professor_len = buffer[buffer_offset++];
    memcpy(course->professor, buffer + buffer_offset, professor_len);
    buffer_offset += professor_len;

    uint8_t days_len = buffer[buffer_offset++];
    memcpy(course->days, buffer + buffer_offset, days_len);
    buffer_offset += days_len;

    memcpy(&(course->credits), buffer + buffer_offset, 1);
    buffer_offset += 1;

    *bytes_read += buffer[0];

    course->next = NULL;

    return course;
}

err_t protocol_courses_lookup_multiple_response_decode(const struct __message_t* in_dgrm, course_t** courses_ret) {
    if (in_dgrm == NULL || courses_ret == NULL) {
        return ERR_INVALID_PARAMETERS;
    }

    if (protocol_get_request_type(in_dgrm) != RESPONSE_TYPE_COURSES_MULTI_LOOKUP) {
        return ERR_INVALID_PARAMETERS;
    }

    uint8_t buffer[sizeof(in_dgrm->data) - REQUEST_RESPONSE_HEADER_LEN] = {0};
    uint16_t buffer_len = sizeof(buffer);

    protocol_decode(in_dgrm, NULL, NULL, &buffer_len, sizeof(buffer), buffer);

    uint8_t course_count = protocol_get_flags(in_dgrm);
    uint16_t buffer_offset = 0;

    for (int i = 0; i < course_count; i++) {
        course_t* course = course_details_decode(buffer + buffer_offset, buffer[buffer_offset], &buffer_offset);
        if (*courses_ret == NULL) {
            *courses_ret = course;
        } else {
            course_t* course_ptr = *courses_ret;
            while(course_ptr->next) {
                course_ptr = course_ptr->next;
            }
            course_ptr->next = course;
            course->next = NULL;
        }
    }

    return ERR_OK;
}

void protocol_courses_lookup_multiple_response_decode_dealloc(course_t* course) {
    while(course != NULL) {
        course_t* next = course->next;
        free(course);
        course = next;
    }
}

err_t protocol_courses_error_encode(const err_t error_code, struct __message_t* out_dgrm) {
    protocol_encode(out_dgrm, RESPONSE_TYPE_COURSES_ERROR, error_code, 0, NULL);
    return ERR_OK;
}

err_t protocol_courses_error_decode(const struct __message_t* in_dgrm, err_t* error_code) {
    if (in_dgrm == NULL || error_code == NULL) {
        return ERR_INVALID_PARAMETERS;
    }

    if (protocol_get_flags(in_dgrm) != RESPONSE_TYPE_COURSES_ERROR) {
        return ERR_INVALID_PARAMETERS;
    }

    protocol_decode(in_dgrm, NULL, error_code, NULL, 0, NULL);
    return ERR_OK;
}
