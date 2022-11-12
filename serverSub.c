#include "log.h"
#include "messages.h"
#include "utils.h"

#include "udp_server.h"
#include "courses.h"

static course_t* db = NULL;
static const char* subject_code = NULL;

typedef enum __department_server_request_type_t {
    DEPARTMENT_SERVER_REQUEST_GET_COURSE_INFO = 0x20,
    DEPARTMENT_SERVER_REQUEST_GET_COURSE_SUMMARY,
    DEPARTMENT_SERVER_REQUEST_INVALID,
} department_server_request_type_t;

typedef enum __department_server_response_type_t {
    DEPARTMENT_SERVER_RESPONSE_GET_COURSE_INFO = 0x28,
    DEPARTMENT_SERVER_RESPONSE_GET_COURSE_SUMMARY,
    DEPARTMENT_SERVER_RESPONSE_INVALID,
} department_server_response_type_t;

static department_server_request_type_t get_request_type(udp_dgram_t* req_dgram) {
    if (req_dgram->data_len == 0 || req_dgram->data[0] >= DEPARTMENT_SERVER_REQUEST_INVALID) {
        return ERR_REQ_INVALID;
    }
    return req_dgram->data[0];
}

static void handle_course_info_lookup_request(udp_dgram_t* req_dgram, udp_dgram_t* resp_dgram) {
    courses_lookup_params_t params = {0};

    if (courses_lookup_decode(req_dgram, &params) != ERR_COURSES_OK) {
        ENCODE_SIMPLE_ERROR_MESSAGE(resp_dgram, ERR_COURSES_INVALID_REQUEST);
    } else {
        char* category = courses_category_string_from_enum(params.category);
        LOGI(SERVER_SUB_MESSAGE_ON_LOOKUP_REQUEST_RECEIVED, subject_code, category, params.course_code);
        uint8_t info[128] = {0};
        size_t info_len = 0;
        course_t* course = courses_lookup(db, params.course_code);
        if (!course) {
            LOGI(SERVER_SUB_MESSAGE_ON_COURSE_NOT_FOUND, params.course_code);
            ENCODE_SIMPLE_ERROR_MESSAGE(resp_dgram, ERR_COURSES_NOT_FOUND);
        } else if (params.category >= COURSES_LOOKUP_CATEGORY_INVALID) {
            LOGI("Invalid Category for lookup: %s", category);
            ENCODE_SIMPLE_ERROR_MESSAGE(resp_dgram, ERR_COURSES_INVALID_PARAMETERS);
        } else if (courses_lookup_info(course, params.category, info, sizeof(info), &info_len) == ERR_COURSES_OK) {
            LOGI(SERVER_SUB_MESSAGE_ON_COURSE_FOUND, category, params.course_code, info);
            resp_dgram->data_len = 2 + info_len;
            resp_dgram->data[0] = DEPARTMENT_SERVER_REQUEST_GET_COURSE_INFO;
            resp_dgram->data[1] = params.category;
            memcpy(resp_dgram->data + 2, info, info_len);
        }
    }
}

static void handle_course_summary_lookup_request(udp_dgram_t* req_dgram, udp_dgram_t* resp_dgram) {
    courses_lookup_params_t params = {0};

    if (courses_summary_decode(req_dgram, &params) != ERR_COURSES_OK) {
        ENCODE_SIMPLE_ERROR_MESSAGE(resp_dgram, ERR_COURSES_INVALID_REQUEST);
    } else {
        LOGI(SERVER_SUB_MESSAGE_ON_SUMMARY_REQUEST_RECEIVED, subject_code, params.course_code);
        course_t* course = courses_lookup(db, params.course_code);
        if (!course) {
            LOGI(SERVER_SUB_MESSAGE_ON_COURSE_NOT_FOUND, params.course_code);
            ENCODE_SIMPLE_ERROR_MESSAGE(resp_dgram, ERR_COURSES_NOT_FOUND);
        } else if (courses_summary_encode(course, (uint8_t*) resp_dgram->data + 1, sizeof(resp_dgram->data), &resp_dgram->data_len) == ERR_COURSES_OK) {
            resp_dgram->data[0] = DEPARTMENT_SERVER_RESPONSE_GET_COURSE_SUMMARY;
            resp_dgram->data_len += 1;
        }
    }
}

static void udp_message_rx_handler(udp_ctx_t* udp, udp_endpoint_t* source, udp_dgram_t* req_dgram) {
    udp_dgram_t resp_dgram = {0};
    department_server_request_type_t req_type = get_request_type(req_dgram);

    if (req_type == DEPARTMENT_SERVER_REQUEST_GET_COURSE_INFO) {
        handle_course_info_lookup_request(req_dgram, &resp_dgram);
    } else if (req_type == DEPARTMENT_SERVER_REQUEST_GET_COURSE_SUMMARY) {
        handle_course_summary_lookup_request(req_dgram, &resp_dgram);
    } else {
        LOGI(SERVER_SUB_MESSAGE_ON_REQUEST_INVALID, subject_code);
        ENCODE_SIMPLE_ERROR_MESSAGE(&resp_dgram, DEPARTMENT_SERVER_RESPONSE_INVALID);
    }

    udp_send(udp, source, &resp_dgram);
}

static void udp_message_tx_handler(udp_ctx_t* udp, udp_endpoint_t* dest, udp_dgram_t* datagram) {
    LOGI(SERVER_SUB_MESSAGE_ON_RESPONSE_SENT, subject_code);
}

static void on_server_init(udp_ctx_t* udp) {
    LOGI(SERVER_SUB_MESSAGE_ON_BOOTUP, subject_code, udp->port);
    udp->on_rx = udp_message_rx_handler;
    udp->on_tx = udp_message_tx_handler;
}

int subjectServerMain(const char* subjectCode, const uint16_t port, const char* db_file) {
    subject_code = subjectCode;
    db = courses_init(db_file);
    udp_ctx_t* udp = udp_start(port, on_server_init);
    while(1) {
        udp_receive(udp);
    }
    udp_stop(udp);
    courses_free(db);
    return 0;
}
