#include <stdlib.h>
#include <string.h>

#include "protocol.h"

void protocol_encode(struct __message_t* message, const uint8_t type, const uint8_t flags, const uint16_t payload_len, const uint8_t* payload) {
    if (sizeof(message->data) >= payload_len + REQUEST_RESPONSE_HEADER_LEN) {
        message->data[REQUEST_RESPONSE_TYPE_OFFSET] = type;
        message->data[REQUEST_RESPONSE_FLAGS_OFFSET] = flags;
        message->data[REQUEST_RESPONSE_PAYLOAD_LEN_OFFSET_1] = payload_len & 0xFF;
        message->data[REQUEST_RESPONSE_PAYLOAD_LEN_OFFSET_2] = payload_len >> 8;
        memcpy(message->data + REQUEST_RESPONSE_HEADER_LEN, payload, payload_len);
        message->data_len = REQUEST_RESPONSE_HEADER_LEN + payload_len;
    }
}

void protocol_decode(const struct __message_t* message, request_type_t* request_type, uint8_t* flags, uint16_t *out_data_len, const uint16_t out_data_size, uint8_t* out_data) {
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

uint16_t protocol_get_payload_len(const struct __message_t* message) {
    return message->data_len < REQUEST_RESPONSE_HEADER_LEN ? REQUEST_RESPONSE_INVALID_TYPE : message->data[REQUEST_RESPONSE_PAYLOAD_LEN_OFFSET_1] | (message->data[REQUEST_RESPONSE_PAYLOAD_LEN_OFFSET_2] << 8);
}

uint8_t protocol_get_flags(const struct __message_t* message) {
    return message->data_len < REQUEST_RESPONSE_HEADER_LEN ? 0 : message->data[REQUEST_RESPONSE_FLAGS_OFFSET];
}

err_t protocol_authentication_request_encode(const credentials_t* credentials, udp_dgram_t* out_dgrm) {
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

err_t protocol_authentication_request_decode(const udp_dgram_t* in_dgrm, credentials_t* credentials) {

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

err_t protocol_authentication_response_encode(const uint8_t authentication_result, udp_dgram_t* out_dgrm) {
    if (out_dgrm == NULL) {
        return ERR_INVALID_PARAMETERS;
    }
    protocol_encode(out_dgrm, RESPONSE_TYPE_AUTH, authentication_result, 0, NULL);
    return ERR_OK;
}

err_t protocol_authentication_response_decode(const udp_dgram_t* in_dgrm, uint8_t* authentication_result) {
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

err_t protocol_courses_lookup_single_request_encode(const char* course_code, const uint8_t course_code_len, const courses_lookup_category_t category, udp_dgram_t* out_dgrm) {
    if (course_code == NULL || course_code_len == 0 || category < COURSES_LOOKUP_CATEGORY_COURSE_CODE || category > COURSES_LOOKUP_CATEGORY_INVALID || out_dgrm == NULL) {
        return ERR_INVALID_PARAMETERS;
    }
    protocol_encode(out_dgrm, REQUEST_TYPE_COURSES_SINGLE_LOOKUP, category, course_code_len, (uint8_t*) course_code);
    return ERR_OK;
}

err_t protocol_courses_lookup_single_request_decode(const udp_dgram_t* in_dgrm, char* course_code, uint8_t* course_code_len, courses_lookup_category_t* category) {
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

err_t protocol_courses_lookup_single_response_encode(const char* course_code, const uint8_t course_code_len, const courses_lookup_category_t category, const uint8_t* information, const uint8_t information_len, udp_dgram_t* out_dgrm) {
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

err_t protocol_courses_lookup_single_response_decode(const udp_dgram_t* in_dgrm, char* course_code, uint8_t* course_code_len, courses_lookup_category_t* category, uint8_t* information, uint8_t* information_len) {
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

err_t protocol_courses_lookup_detail_request_encode(const uint8_t* course_code, const uint8_t course_code_len, udp_dgram_t* out_dgrm) {
    if (course_code == NULL || out_dgrm == NULL) {
        return ERR_INVALID_PARAMETERS;
    }
    protocol_encode(out_dgrm, REQUEST_TYPE_COURSES_DETAIL_LOOKUP, 0, course_code_len, course_code);
    return ERR_OK;
}

err_t protocol_courses_lookup_detail_request_decode(const udp_dgram_t* in_dgrm, uint8_t* course_code, uint8_t* course_code_len) {
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

err_t protocol_courses_lookup_detail_response_encode(const course_t* course, udp_dgram_t* out_dgrm) {
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

err_t protocol_courses_lookup_detail_response_decode(const udp_dgram_t* in_dgrm, course_t* course) {
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

err_t protocol_courses_error_encode(const err_t error_code, udp_dgram_t* out_dgrm) {
    protocol_encode(out_dgrm, RESPONSE_TYPE_COURSES_ERROR, error_code, 0, NULL);
    return ERR_OK;
}

err_t protocol_courses_error_decode(const udp_dgram_t* in_dgrm, err_t* error_code) {
    if (in_dgrm == NULL || error_code == NULL) {
        return ERR_INVALID_PARAMETERS;
    }

    if (protocol_get_flags(in_dgrm) != RESPONSE_TYPE_COURSES_ERROR) {
        return ERR_INVALID_PARAMETERS;
    }

    protocol_decode(in_dgrm, NULL, error_code, NULL, 0, NULL);
    return ERR_OK;
}
