#include "log.h"
#include "messages.h"
#include "utils.h"

#include "networking.h"
#include "courses.h"

LOG_TAG(serverSub);

static course_t* db = NULL;
static const char* subject_code = NULL;

typedef enum __department_server_response_type_t {
    DEPARTMENT_SERVER_RESPONSE_GET_COURSE_INFO = 0x28,
    DEPARTMENT_SERVER_RESPONSE_GET_COURSE_SUMMARY,
    DEPARTMENT_SERVER_RESPONSE_INVALID,
} department_server_response_type_t;

static void handle_course_info_lookup_request(udp_dgram_t* req_dgram, udp_dgram_t* resp_dgram) {
    courses_lookup_params_t params = {0};

    if (courses_lookup_info_request_decode(req_dgram, &params) != ERR_COURSES_OK) {
        ENCODE_SIMPLE_ERROR_MESSAGE(resp_dgram, ERR_COURSES_INVALID_REQUEST);
    } else {
        char* category = courses_category_string_from_enum(params.category);
        LOG_INFO(SERVER_SUB_MESSAGE_ON_LOOKUP_REQUEST_RECEIVED, subject_code, category, params.course_code);
        uint8_t info[128] = {0};
        size_t info_len = 0;
        course_t* course = courses_lookup(db, params.course_code);
        if (!course) {
            LOG_WARN(SERVER_SUB_MESSAGE_ON_COURSE_NOT_FOUND, params.course_code);
            courses_lookup_info_response_encode_error(resp_dgram, &params, ERR_COURSES_NOT_FOUND);
        } else if (params.category >= COURSES_LOOKUP_CATEGORY_INVALID) {
            LOG_WARN("Invalid Category for lookup: %s", category);
            courses_lookup_info_response_encode_error(resp_dgram, &params, ERR_COURSES_INVALID_PARAMETERS);
        } else if (courses_lookup_info(course, params.category, info, sizeof(info), &info_len) == ERR_COURSES_OK) {
            LOG_INFO(SERVER_SUB_MESSAGE_ON_COURSE_FOUND, category, params.course_code, info);
            courses_lookup_info_response_encode(resp_dgram, &params, info, info_len);
        }
    }
}

static void handle_course_detail_lookup_request(udp_dgram_t* req_dgram, udp_dgram_t* resp_dgram) {
    // courses_lookup_params_t params = {0};

    uint8_t course_code[10] = {0};

    if (courses_details_request_decode(req_dgram, course_code, sizeof(course_code)) != ERR_COURSES_OK) {
        ENCODE_SIMPLE_ERROR_MESSAGE(resp_dgram, ERR_COURSES_INVALID_REQUEST);
    } else {
        LOG_INFO(SERVER_SUB_MESSAGE_ON_SUMMARY_REQUEST_RECEIVED, subject_code, course_code);
        course_t* course = courses_lookup(db, (const char*) course_code);
        if (!course) {
            LOG_WARN(SERVER_SUB_MESSAGE_ON_COURSE_NOT_FOUND, course_code);
            ENCODE_SIMPLE_ERROR_MESSAGE(resp_dgram, ERR_COURSES_NOT_FOUND);
        } else {
            courses_details_response_encode(course, resp_dgram);
        }
    }
}

static void udp_message_rx_handler(udp_ctx_t* udp, udp_endpoint_t* source, udp_dgram_t* req_dgram) {
    udp_dgram_t resp_dgram = {0};
    request_type_t req_type = protocol_get_request_type(req_dgram);
    if (req_type == REQUEST_TYPE_COURSES_LOOKUP_INFO) {
        handle_course_info_lookup_request(req_dgram, &resp_dgram);
    } else if (req_type == REQUEST_TYPE_COURSES_DETAIL_LOOKUP) {
        handle_course_detail_lookup_request(req_dgram, &resp_dgram);
    } else {
        LOG_WARN(SERVER_SUB_MESSAGE_ON_REQUEST_INVALID, subject_code);
        ENCODE_SIMPLE_ERROR_MESSAGE(&resp_dgram, DEPARTMENT_SERVER_RESPONSE_INVALID);
    }

    udp_send(udp, source, &resp_dgram);
}

static void udp_message_tx_handler(udp_ctx_t* udp, udp_endpoint_t* dest, udp_dgram_t* datagram) {
    LOG_INFO(SERVER_SUB_MESSAGE_ON_RESPONSE_SENT, subject_code);
}


int subjectServerMain(const char* subjectCode, const uint16_t port, const char* db_file) {
    subject_code = subjectCode;
    db = courses_init(db_file);

    udp_ctx_t* udp = udp_start(port);
    LOG_INFO(SERVER_SUB_MESSAGE_ON_BOOTUP, subject_code, udp->port);
    udp->on_rx = udp_message_rx_handler;
    udp->on_tx = udp_message_tx_handler;

    while(1) {
        udp_receive(udp);
    }
    udp_stop(udp);
    courses_free(db);
    return 0;
}
