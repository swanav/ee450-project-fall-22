#include "courses.h"
#include <log.h>
#include <stdio.h>
#include <utils.h>
#include "../protocol.h"

LOG_TAG(courses);

#define SPLIT_TOKEN ",\r\n"

course_t* courses_init(const char* filename) {
    course_t* head = NULL;
    course_t* tail = NULL;
    FILE* fp = csv_open(filename);
    char line[1024];

    if (fp == NULL) {
        return NULL;
    }

    while (fgets(line, sizeof(line), fp)) {
        char* token = strtok(line, SPLIT_TOKEN);
        if (token == NULL) {
            continue;
        }
        course_t* entry = (course_t*)malloc(sizeof(course_t));
        if (entry == NULL) {
            LOG_ERR("Failed to allocate memory for course_t");
            continue;
        }
        memset(entry, 0, sizeof(course_t));
        strncpy(entry->course_code, token, sizeof(entry->course_code));
        token = strtok(NULL, SPLIT_TOKEN);
        if (token == NULL) {
            continue;
        }
        entry->credits = atoi(token);
        token = strtok(NULL, SPLIT_TOKEN);
        if (token == NULL) {
            continue;
        }
        strncpy(entry->professor, token, sizeof(entry->professor));
        token = strtok(NULL, SPLIT_TOKEN);
        if (token == NULL) {
            continue;
        }
        strncpy(entry->days, token, sizeof(entry->days));
        token = strtok(NULL, SPLIT_TOKEN);
        if (token == NULL) {
            continue;
        }
        strncpy(entry->course_name, token, sizeof(entry->course_name));
        if (head == NULL) {
            head = entry;
            tail = entry;
        } else {
            tail->next = entry;
            tail = entry;
        }
    }

    fclose(fp);
    return head;
}

void courses_free(course_t* head) {
    course_t* entry = head;
    while (entry != NULL) {
        course_t* next = entry->next;
        free(entry);
        entry = next;
    }
}

void courses_print(course_t* course) {
    LOG_WARN("%s %s %s %s %d", course->course_code, course->course_name, course->professor, course->days, course->credits);
}

void courses_printall(course_t* course) {
    while (course != NULL) {
        courses_print(course);
        course = course->next;
    }
}


// int courses_parse_lookup_request(char* req_string, size_t req_len, courses_lookup_params_t* params) {
//     char* token = strtok(req_string, SPLIT_TOKEN);
//     if (token == NULL) {
//         return -1;
//     }
//     strncpy(params->course_code, token, sizeof(params->course_code));

//     token = strtok(NULL, SPLIT_TOKEN);
//     if (token == NULL) {
//         return -1;
//     }

//     strncpy(params->category, token, sizeof(params->category));

//     return 0;
// }

course_t* courses_lookup(const course_t* db, const char* course_code) {
    const course_t* entry = db;
    while (entry != NULL) {
        if (strcasecmp(entry->course_code, course_code) == 0) {
            return (course_t*)entry;
        }
        entry = entry->next;
    }
    return NULL;
}

courses_lookup_category_t courses_lookup_category_from_string(const char* category) {
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

err_t courses_lookup_info(const course_t* course, courses_lookup_category_t category, uint8_t* info_buf, size_t info_buf_size, size_t* info_len) {
    if (course == NULL || info_buf == NULL || info_len == NULL || category >= COURSES_LOOKUP_CATEGORY_INVALID) {
        return ERR_COURSES_INVALID_PARAMETERS;
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
            return ERR_COURSES_INVALID_PARAMETERS;
    }
    return ERR_COURSES_OK;
}

uint8_t courses_flags_from_category(courses_lookup_category_t category) {
    switch (category) {
        case COURSES_LOOKUP_CATEGORY_COURSE_CODE:
            return COURSE_LOOKUP_FLAGS_COURSE_CODE;
        case COURSES_LOOKUP_CATEGORY_CREDITS:
            return COURSE_LOOKUP_FLAGS_CREDITS;
        case COURSES_LOOKUP_CATEGORY_PROFESSOR:
            return COURSE_LOOKUP_FLAGS_PROFESSOR;
        case COURSES_LOOKUP_CATEGORY_DAYS:
            return COURSE_LOOKUP_FLAGS_DAYS;
        case COURSES_LOOKUP_CATEGORY_COURSE_NAME:
            return COURSE_LOOKUP_FLAGS_COURSE_NAME;
        default:
            return 0;
    }
}

courses_lookup_category_t courses_category_from_flags(uint8_t category) {
    switch (category) {
        case COURSE_LOOKUP_FLAGS_COURSE_CODE:
            return COURSES_LOOKUP_CATEGORY_COURSE_CODE;
        case COURSE_LOOKUP_FLAGS_CREDITS:
            return COURSES_LOOKUP_CATEGORY_CREDITS;
        case COURSE_LOOKUP_FLAGS_PROFESSOR:
            return COURSES_LOOKUP_CATEGORY_PROFESSOR;
        case COURSE_LOOKUP_FLAGS_DAYS:
            return COURSES_LOOKUP_CATEGORY_DAYS;
        case COURSE_LOOKUP_FLAGS_COURSE_NAME:
            return COURSES_LOOKUP_CATEGORY_COURSE_NAME;
        default:
            return COURSES_LOOKUP_CATEGORY_INVALID;
    }
}


err_t courses_details_request_encode(char* course_code, struct __message_t* out_msg) {
    if (course_code == NULL || out_msg == NULL) {
        return ERR_COURSES_INVALID_PARAMETERS;
    }
    protocol_encode(out_msg, REQUEST_TYPE_COURSES_DETAIL_LOOKUP, 0, strlen(course_code), course_code);
    return ERR_COURSES_OK;
}

err_t courses_details_request_decode(struct __message_t* in_msg, uint8_t* course_code, uint8_t course_code_size) {
    if (in_msg == NULL || course_code == NULL) {
        return ERR_COURSES_INVALID_PARAMETERS;
    }

    request_type_t type = protocol_get_request_type(in_msg);
    if (type != REQUEST_TYPE_COURSES_DETAIL_LOOKUP) {
        return ERR_COURSES_INVALID_PARAMETERS;
    }


    uint8_t olen = 0;
    protocol_decode(in_msg, NULL, NULL, &olen, course_code_size, course_code);
    course_code[olen] = '\0';

    return ERR_COURSES_OK;
}

err_t courses_lookup_info_request_encode(courses_lookup_params_t* params, struct __message_t* out_msg) {
    if (params == NULL || out_msg == NULL) {
        return ERR_COURSES_INVALID_PARAMETERS;
    }
    uint8_t flags = courses_flags_from_category(params->category);
    protocol_encode(out_msg, REQUEST_TYPE_COURSES_LOOKUP_INFO, flags, strlen(params->course_code), (uint8_t*) params->course_code);
    return ERR_COURSES_OK;
}

err_t courses_lookup_info_request_decode(struct __message_t* msg, courses_lookup_params_t* params) {
    if (params == NULL || msg == NULL) {
        return ERR_COURSES_INVALID_PARAMETERS;
    }

    uint8_t type = protocol_get_request_type(msg);

    if (type != REQUEST_TYPE_COURSES_LOOKUP_INFO && type != RESPONSE_TYPE_COURSES_LOOKUP_INFO) {
        return ERR_COURSES_INVALID_PARAMETERS;
    }

    bzero(params, sizeof(courses_lookup_params_t));
    uint8_t flags = 0;
    uint8_t olen = 0;
    protocol_decode(msg, NULL, &flags, &olen, sizeof(params->course_code), (uint8_t*) params->course_code);
    params->category = courses_category_from_flags(flags);

    return ERR_COURSES_OK;
}

// Get length of the words in a sentence
int get_word_length(char* sentence) {
    int length = 0;
    while (*sentence != ' ' && *sentence != '\0') {
        length++;
        sentence++;
    }
    return length;
}

err_t courses_lookup_multiple_request_encode(struct __message_t* out_msg, uint8_t course_count, uint8_t* course_buffer, uint8_t course_buffer_size) {
    if (out_msg == NULL || course_buffer == NULL) {
        return ERR_COURSES_INVALID_PARAMETERS;
    }

    uint8_t* ptr = course_buffer;
    uint8_t buffer[64];
    uint8_t buffer_len = 0;
    buffer[0] = course_count;
    buffer_len += 1;
    for (uint8_t i = 0; i < course_count; i++) {
        // get length of words and copy into buffer
        int length = get_word_length((char*) ptr);
        buffer[buffer_len] = length;
        buffer_len += 1;
        memcpy(buffer + buffer_len, ptr, length);
        buffer_len += length;
        ptr += length;
        // skip space
        ptr += 1;
    }
    protocol_encode(out_msg, REQUEST_TYPE_COURSES_MULTI_LOOKUP, 0, buffer_len, buffer);
    return ERR_COURSES_OK;
}

err_t courses_lookup_multiple_request_decode(struct __message_t* in_msg, uint8_t* course_count, uint8_t* buffer, uint8_t buffer_len) {
    if (in_msg == NULL || buffer == NULL) {
        return ERR_COURSES_INVALID_PARAMETERS;
    }

    uint8_t type = protocol_get_request_type(in_msg);
    if (type != REQUEST_TYPE_COURSES_MULTI_LOOKUP) {
        return ERR_COURSES_INVALID_PARAMETERS;
    }

    uint8_t data_buffer[64];
    uint8_t olen = 0;    

    protocol_decode(in_msg, NULL, NULL, &olen, sizeof(data_buffer), data_buffer);
    *course_count = data_buffer[0];
    memcpy(buffer, data_buffer + 1, olen - 1);


    return ERR_COURSES_OK;
}

// err_t courses_lookup_multiple_response_encode()


err_t courses_lookup_info_response_encode_error(struct __message_t* out_msg, courses_lookup_params_t* params, uint8_t error_code) {
    if (params == NULL || out_msg == NULL) {
        return ERR_COURSES_INVALID_PARAMETERS;
    }
    uint8_t buffer[64];
    uint8_t buffer_len = 0;

    uint8_t flags = courses_flags_from_category(params->category) | COURSE_LOOKUP_FLAGS_INVALID;

    buffer[0] = strlen(params->course_code);
    buffer_len += 1;
    memcpy(buffer + buffer_len, params->course_code, strlen(params->course_code));
    buffer_len += strlen(params->course_code);
    buffer[buffer_len] = error_code;
    buffer_len += 1;

    protocol_encode(out_msg, RESPONSE_TYPE_COURSES_LOOKUP_INFO, flags, buffer_len, buffer);
    return ERR_COURSES_OK;
}

err_t courses_lookup_info_response_decode_error(struct __message_t* msg, courses_lookup_params_t* params, uint8_t* error_code) {
    if (params == NULL || msg == NULL || error_code == NULL) {
        return ERR_COURSES_INVALID_PARAMETERS;
    }

    uint8_t type = protocol_get_request_type(msg);

    if (type != REQUEST_TYPE_COURSES_LOOKUP_INFO && type != RESPONSE_TYPE_COURSES_LOOKUP_INFO) {
        return ERR_COURSES_INVALID_PARAMETERS;
    }

    bzero(params, sizeof(courses_lookup_params_t));
    uint8_t flags = 0;
    uint8_t olen = 0;
    uint8_t buffer[64];
    protocol_decode(msg, NULL, &flags, &olen, sizeof(buffer), buffer);
    params->category = courses_category_from_flags(flags);
    if (COURSE_LOOKUP_MASK_INVALID(flags)) {
        uint8_t course_code_len = buffer[0];
        memcpy(params->course_code, buffer + 1, course_code_len);
        params->course_code[course_code_len] = '\0';
        *error_code = buffer[1 + course_code_len];
    } else {
        *error_code = 0;
    }

    return ERR_COURSES_OK;
}

err_t courses_lookup_info_response_encode(struct __message_t* out_msg, courses_lookup_params_t* params, uint8_t* info_buffer, uint8_t info_buffer_len) {

    if (params == NULL || out_msg == NULL) {
        return ERR_COURSES_INVALID_PARAMETERS;
    }

    uint8_t flags = courses_flags_from_category(params->category);

    uint8_t buffer[64];
    uint8_t buffer_len = 0;

    if (info_buffer != NULL && info_buffer_len > 0 && info_buffer_len <= 62) {
        uint8_t buffer_offset = 0;
        buffer[buffer_offset++] = strlen(params->course_code);
        buffer[buffer_offset++] = info_buffer_len;
        memcpy(buffer + buffer_offset, params->course_code, buffer[0]);
        buffer_offset += buffer[0];
        memcpy(buffer + buffer_offset, info_buffer, buffer[1]);
        buffer_offset += buffer[1];
        buffer_len = buffer_offset;
        protocol_encode(out_msg, RESPONSE_TYPE_COURSES_LOOKUP_INFO, flags, buffer_len, buffer);
        LOG_BUFFER(buffer, buffer_len);
    }

    return ERR_COURSES_OK;
}

err_t courses_lookup_info_response_decode(struct __message_t* msg, courses_lookup_params_t* params, uint8_t* info_buffer, uint8_t info_buffer_size, uint8_t* info_len) {

    if (params == NULL || msg == NULL) {
        return ERR_COURSES_INVALID_PARAMETERS;
    }

    uint8_t type = protocol_get_request_type(msg);
    if (type != RESPONSE_TYPE_COURSES_LOOKUP_INFO) {
        return ERR_COURSES_INVALID_PARAMETERS;
    }
    uint8_t flags = 0;
    uint8_t output_data_buffer[64];
    uint8_t output_data_buffer_len = 0;
    
    protocol_decode(msg, NULL, &flags, &output_data_buffer_len, sizeof(output_data_buffer), output_data_buffer);    
    LOG_BUFFER(output_data_buffer, output_data_buffer_len);

    params->category = courses_category_from_flags(flags);
    memcpy(params->course_code, output_data_buffer + 2, output_data_buffer[0]);
    params->course_code[output_data_buffer[0]] = '\0';
    memcpy(info_buffer, output_data_buffer + 2 + output_data_buffer[0], output_data_buffer[1]);
    info_buffer[output_data_buffer[1]] = '\0';
    *info_len = output_data_buffer[1];

    return ERR_COURSES_OK;
}

err_t courses_lookup_decode(udp_dgram_t* req_dgram, courses_lookup_params_t* params) {
    if (req_dgram == NULL || params == NULL || req_dgram->data_len < 2) {
        return ERR_COURSES_INVALID_PARAMETERS;
    }

    params->category = req_dgram->data[1];
    memcpy(params->course_code, req_dgram->data + 2, req_dgram->data_len - 2);

    return ERR_COURSES_OK;
}

err_t courses_details_response_decode(udp_dgram_t* req_dgram, course_t* course) {
    if (req_dgram == NULL || course == NULL) {
        return ERR_COURSES_INVALID_PARAMETERS;
    }

    uint8_t buffer[128];
    uint8_t buffer_len = 0;

    protocol_decode(req_dgram, NULL, NULL, &buffer_len, sizeof(buffer), buffer);

    LOG_BUFFER(buffer, buffer_len);

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

    return ERR_COURSES_OK;
}

err_t courses_details_response_encode(const course_t* course, struct __message_t* out_msg) {
    if (course == NULL || out_msg == NULL) {
        return ERR_COURSES_INVALID_PARAMETERS;
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

    protocol_encode(out_msg, RESPONSE_TYPE_COURSES_DETAIL_LOOKUP, 0, offset, buffer);

    return ERR_COURSES_OK;
}

char* courses_category_string_from_enum(courses_lookup_category_t category) {
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