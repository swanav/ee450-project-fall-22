#include "courses.h"
#include <log.h>
#include <stdio.h>
#include <utils.h>

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
            LOGEM("Failed to allocate memory for course_t");
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
        if (strcmp(entry->course_code, course_code) == 0) {
            return (course_t*)entry;
        }
        entry = entry->next;
    }
    return NULL;
}

courses_lookup_category_t courses_lookup_category_from_string(const char* category) {
    if (strcmp(category, "CourseCode") == 0) {
        return COURSES_LOOKUP_CATEGORY_COURSE_CODE;
    } else if (strcmp(category, "Credits") == 0) {
        return COURSES_LOOKUP_CATEGORY_CREDITS;
    } else if (strcmp(category, "Professor") == 0) {
        return COURSES_LOOKUP_CATEGORY_PROFESSOR;
    } else if (strcmp(category, "Days") == 0) {
        return COURSES_LOOKUP_CATEGORY_DAYS;
    } else if (strcmp(category, "CourseName") == 0) {
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


err_t courses_lookup_info_response_encode(struct __message_t* out_msg, courses_lookup_params_t* params, uint8_t* info_buffer, uint8_t info_buffer_len) {

    if (params == NULL || out_msg == NULL) {
        return ERR_COURSES_INVALID_PARAMETERS;
    }

    uint8_t flags = courses_flags_from_category(params->category);

    uint8_t buffer[64];
    uint8_t buffer_len = 0;

    if (info_buffer != NULL && info_buffer_len > 0 && info_buffer_len <= 62) {
        buffer_len = info_buffer_len + 2;
        buffer[0] = strlen(params->course_code);
        buffer[1] = info_buffer_len;
        memcpy(buffer + 2, params->course_code, buffer[0]);
        memcpy(buffer + 2 + buffer[0], info_buffer, buffer[1]);
        protocol_encode(out_msg, RESPONSE_TYPE_COURSES_LOOKUP_INFO, flags, buffer_len, buffer);
    }

    return ERR_COURSES_OK;
}

err_t courses_lookup_info_response_decode(struct __message_t* msg, courses_lookup_params_t* params, uint8_t* info_buffer, uint8_t info_buffer_size, uint8_t* info_len) {

    if (params == NULL || msg == NULL) {
        return ERR_COURSES_INVALID_PARAMETERS;
    }

    

    // protocol_decode
    


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

err_t courses_summary_decode(udp_dgram_t* req_dgram, courses_lookup_params_t* params) {
    if (req_dgram == NULL || params == NULL || req_dgram->data_len < 2) {
        return ERR_COURSES_INVALID_PARAMETERS;
    }

    memcpy(params->course_code, req_dgram->data + 2, req_dgram->data_len - 2);

    return ERR_COURSES_OK;
}

err_t courses_summary_encode(const course_t* course, uint8_t* buffer, size_t buffer_len, size_t* encoded_len) {
    if (course == NULL || buffer == NULL || encoded_len == NULL) {
        return ERR_COURSES_INVALID_PARAMETERS;
    }

    size_t offset = 0;
    
    buffer[offset++] = 2 + strlen(course->course_code);
    buffer[offset++] = COURSES_LOOKUP_CATEGORY_COURSE_CODE;
    memcpy(buffer + offset, course->course_code, strlen(course->course_code));
    offset += strlen(course->course_code);

    buffer[offset++] = 2 + strlen(course->course_name);
    buffer[offset++] = COURSES_LOOKUP_CATEGORY_COURSE_NAME;
    memcpy(buffer + offset, course->course_name, strlen(course->course_name));
    offset += strlen(course->course_name);

    buffer[offset++] = 2 + strlen(course->professor);
    buffer[offset++] = COURSES_LOOKUP_CATEGORY_PROFESSOR;
    memcpy(buffer + offset, course->professor, strlen(course->professor));
    offset += strlen(course->professor);

    buffer[offset++] = 2 + strlen(course->days);
    buffer[offset++] = COURSES_LOOKUP_CATEGORY_DAYS;
    memcpy(buffer + offset, course->days, strlen(course->days));
    offset += strlen(course->days);

    buffer[offset++] = 2 + 1;
    buffer[offset++] = COURSES_LOOKUP_CATEGORY_CREDITS;
    buffer[offset++] = course->credits;

    *encoded_len = offset;
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