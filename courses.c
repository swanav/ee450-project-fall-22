#include <stdio.h>
#include <string.h>

#include "courses.h"
#include "log.h"
#include "protocol.h"
#include "utils.h"

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
