#include "database.h"
#include "department_server.h"
#include "fileio.h"
#include "log.h"
#include "messages.h"
#include "networking.h"
#include "utils.h"

LOG_TAG(department_server);

static course_t* db = NULL;
static const char* subject_code = NULL;

static void handle_course_info_lookup_request(udp_dgram_t* req_dgram, udp_dgram_t* resp_dgram) {

    char course_code[10] = {0};
    uint8_t size = sizeof(course_code);
    courses_lookup_category_t category = COURSES_LOOKUP_CATEGORY_INVALID;

    // Get the course code and category from the request
    if (protocol_courses_lookup_single_request_decode(req_dgram, course_code, &size, &category) != ERR_OK) {
        // Failed to parse the request
        protocol_courses_error_encode(ERR_REQ_INVALID, (uint8_t*) course_code, strlen(course_code), resp_dgram);
    } else {
        LOG_INFO(SERVER_SUB_MESSAGE_ON_LOOKUP_REQUEST_RECEIVED, subject_code, database_courses_category_string_from_enum(category), course_code);
        uint8_t info[128] = {0};
        size_t info_len = 0;
        // Lookup the course in the database
        course_t* course = database_courses_lookup(db, course_code);
        if (!course) {
            // Course not found
            LOG_WARN(SERVER_SUB_MESSAGE_ON_COURSE_NOT_FOUND, course_code);
            protocol_courses_error_encode(ERR_COURSES_NOT_FOUND, (uint8_t*) course_code, strlen(course_code), resp_dgram);
        } else if (category >= COURSES_LOOKUP_CATEGORY_INVALID) {
            // Invalid category
            LOG_WARN("Invalid category for lookup: %s", category);
            protocol_courses_error_encode(ERR_REQ_INVALID, (uint8_t*) course_code, strlen(course_code), resp_dgram);
        } else {
            LOG_DBG("Course found: %s", course->course_code);
            // Get the course info
            if (database_courses_lookup_info(course, category, info, sizeof(info), &info_len) == ERR_OK) {
                LOG_INFO(SERVER_SUB_MESSAGE_ON_COURSE_FOUND, database_courses_category_string_from_enum(category), course_code, info);
                // Encode the response
                protocol_courses_lookup_single_response_encode(course_code, size, category, info, info_len, resp_dgram);
            }
        }
    }
}

static void handle_course_detail_lookup_request(udp_dgram_t* req_dgram, udp_dgram_t* resp_dgram) {
    uint8_t course_code[10] = {0};
    uint8_t size = sizeof(course_code);
    // Decode the request
    if (protocol_courses_lookup_detail_request_decode(req_dgram, course_code, &size) != ERR_OK) {
        // If the request is invalid, send an error response
        protocol_courses_error_encode(ERR_REQ_INVALID, course_code, strlen((char*) course_code), resp_dgram);
    } else {
        LOG_INFO(SERVER_SUB_MESSAGE_ON_SUMMARY_REQUEST_RECEIVED, subject_code, course_code);
        // If the request is valid, lookup the course
        course_t* course = database_courses_lookup(db, (const char*) course_code);
        if (!course) {
            LOG_WARN(SERVER_SUB_MESSAGE_ON_COURSE_NOT_FOUND, course_code);
            // If the course is not found, send an error response
            protocol_courses_error_encode(ERR_COURSES_NOT_FOUND, course_code, strlen((char*) course_code), resp_dgram);
        } else {
            // If the course is found, send a response with the course details
            protocol_courses_lookup_detail_response_encode(course, resp_dgram);
        }
    }
}

static void udp_message_rx_handler(udp_ctx_t* udp, udp_endpoint_t* src, udp_dgram_t* req_dgram) {
    udp_dgram_t resp_dgram = {0};
    request_type_t req_type = protocol_get_request_type(req_dgram);
    if (req_type == REQUEST_TYPE_COURSES_SINGLE_LOOKUP) {
        // Handle course info lookup request
        handle_course_info_lookup_request(req_dgram, &resp_dgram);
    } else if (req_type == REQUEST_TYPE_COURSES_DETAIL_LOOKUP) {
        // Handle course detail lookup request
        handle_course_detail_lookup_request(req_dgram, &resp_dgram);
    } else {
        // Handle invalid request
        LOG_WARN(SERVER_SUB_MESSAGE_ON_REQUEST_INVALID, subject_code);
        protocol_courses_error_encode(ERR_REQ_INVALID, NULL, 0, &resp_dgram);
    }

    // Send response
    udp_send(udp, src, &resp_dgram);
    LOG_INFO(SERVER_SUB_MESSAGE_ON_RESPONSE_SENT, subject_code);
}

int department_server_main(const char* subjectCode, const uint16_t port, const char* db_file) {
    subject_code = subjectCode;

    // Load the department server database
    db = fileio_department_server_db_create(db_file);

    // Create the UDP context. Bind it to the relevant port.
    udp_ctx_t* udp = udp_start(port);
    if (!udp) {
        // UDP context creation failed. Exit.
        LOG_ERR("SERVER_SUB_MESSAGE_ON_UDP_START_FAILED", subject_code);
        return -1;
    }
    LOG_INFO(SERVER_SUB_MESSAGE_ON_BOOTUP, subject_code, udp->port);
    // Register the UDP message handler
    udp->on_rx = udp_message_rx_handler;

    // Wait for incoming messages
    while(1) {
        udp_receive(udp);
    }

    // Stop the UDP context. Free up the memory.
    udp_stop(udp);

    // Free up the database
    fileio_department_server_db_free(db);
    return 0;
}
