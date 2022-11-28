#include "courses.h"
#include <stdio.h>
#include <string.h>
#include "../utils.h"
#include "../log.h"
#include "../protocol.h"

LOG_TAG(courses);

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

err_t courses_details_request_encode(uint8_t * course_code, uint8_t course_code_len, struct __message_t* out_msg) {
    if (course_code == NULL || out_msg == NULL) {
        return ERR_INVALID_PARAMETERS;
    }
    protocol_encode(out_msg, REQUEST_TYPE_COURSES_DETAIL_LOOKUP, 0, course_code_len, course_code);
    return ERR_OK;
}

err_t courses_details_request_decode(struct __message_t* in_msg, uint8_t* course_code, uint8_t course_code_size) {
    if (in_msg == NULL || course_code == NULL) {
        return ERR_INVALID_PARAMETERS;
    }

    request_type_t type = protocol_get_request_type(in_msg);
    if (type != REQUEST_TYPE_COURSES_DETAIL_LOOKUP) {
        return ERR_INVALID_PARAMETERS;
    }


    uint16_t olen = 0;
    protocol_decode(in_msg, NULL, NULL, &olen, course_code_size, course_code);
    course_code[olen] = '\0';

    return ERR_OK;
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
        return ERR_INVALID_PARAMETERS;
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
    return ERR_OK;
}

err_t courses_lookup_multiple_request_decode(struct __message_t* in_msg, uint8_t* course_count, uint8_t* buffer, uint8_t buffer_len) {
    if (in_msg == NULL || buffer == NULL) {
        return ERR_INVALID_PARAMETERS;
    }

    uint8_t type = protocol_get_request_type(in_msg);
    if (type != REQUEST_TYPE_COURSES_MULTI_LOOKUP) {
        return ERR_INVALID_PARAMETERS;
    }

    uint8_t data_buffer[64];
    uint16_t olen = 0;    

    protocol_decode(in_msg, NULL, NULL, &olen, sizeof(data_buffer), data_buffer);
    *course_count = data_buffer[0];
    memcpy(buffer, data_buffer + 1, olen - 1);


    return ERR_OK;
}

uint8_t course_details_encode(course_t* course, uint8_t* buffer, uint8_t buffer_size) {
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

err_t courses_lookup_multiple_response_encode(struct __message_t* out_msg, course_t* courses) {
    if (out_msg == NULL || courses == NULL) {
        return ERR_INVALID_PARAMETERS;
    }
    
    uint8_t buffer[1016];
    uint16_t buffer_offset = 0;

    course_t* course_ptr = courses;
    uint8_t course_count = 0;
    while(course_ptr) {
        buffer_offset += course_details_encode(course_ptr, buffer + buffer_offset, sizeof(buffer) - buffer_offset);
        course_count++;
        course_ptr = course_ptr->next;
    }

    protocol_encode(out_msg, RESPONSE_TYPE_COURSES_MULTI_LOOKUP, course_count, buffer_offset, buffer);

    return ERR_OK;
}

course_t* course_details_decode(uint8_t* buffer, uint8_t buffer_size, uint16_t* bytes_read) {
    course_t* course = calloc(1, sizeof(course_t));

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

    return course;
}

course_t* courses_lookup_multiple_response_decode(struct __message_t* in_msg) {
    if (in_msg == NULL || (protocol_get_request_type(in_msg) != RESPONSE_TYPE_COURSES_MULTI_LOOKUP)) {
        return NULL;
    }

    uint8_t course_count = protocol_get_flags(in_msg);

    uint8_t data_buffer[1016];
    uint16_t olen = 0;
    protocol_decode(in_msg, NULL, NULL, &olen, sizeof(data_buffer), data_buffer);
    uint16_t bytes_read = 0;

    course_t* courses = NULL;
    for (int i = 0; i < course_count; i++) {
        course_t* course = course_details_decode(data_buffer + bytes_read, data_buffer[bytes_read], &bytes_read);
        if (courses == NULL) {
            courses = course;
        } else {
            course_t* course_ptr = courses;
            while(course_ptr->next) {
                course_ptr = course_ptr->next;
            }
            course_ptr->next = course;
        }
    }

    return courses;
}

err_t courses_details_response_decode(udp_dgram_t* req_dgram, course_t* course) {
    if (req_dgram == NULL || course == NULL) {
        return ERR_INVALID_PARAMETERS;
    }

    uint8_t buffer[128];
    uint16_t buffer_len = 0;

    protocol_decode(req_dgram, NULL, NULL, &buffer_len, sizeof(buffer), buffer);

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

err_t courses_details_response_encode(const course_t* course, struct __message_t* out_msg) {
    if (course == NULL || out_msg == NULL) {
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

    protocol_encode(out_msg, RESPONSE_TYPE_COURSES_DETAIL_LOOKUP, 0, offset, buffer);

    return ERR_OK;
}
