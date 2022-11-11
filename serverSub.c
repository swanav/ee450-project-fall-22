#include "log.h"
#include "messages.h"
#include "utils.h"

#include "udp_server.h"
#include "courses.h"

// EE450,Days

static db_entry_t* db = NULL;

static void udp_message_rx_handler(udp_server_t* server, udp_endpoint_t* source, udp_dgram_t* req_dgram) {
    lookup_params_t params = {0};
    if (parse_lookup_request(req_dgram->data, req_dgram->data_len, &params) == 0) {
        LOGI(SERVER__MESSAGE_ON_REQUEST_RECEIVED(EE), params.category, params.course_code);
        char information[128] = {0};
        lookup_result_t result = course_lookup(db, params.category, params.course_code, information, sizeof(information));
        if (result == LOOKUP_OK) {
            LOGI(SERVER__MESSAGE_ON_COURSE_FOUND(EE), params.category, params.course_code, information);
        } else {
            LOGI(SERVER__MESSAGE_ON_COURSE_NOT_FOUND(EE), params.course_code);
        }
    }
}

static void udp_message_tx_handler(udp_server_t* server, udp_endpoint_t* dest, udp_dgram_t* datagram) {
    LOGIM(SERVER__MESSAGE_ON_RESPONSE_SENT(EE));
}

static void on_server_init(udp_server_t* server) {
    LOGI(SERVER__MESSAGE_ON_BOOTUP(EE), SERVER_EE_UDP_PORT_NUMBER);
    server->on_rx = udp_message_rx_handler;
    server->on_tx = udp_message_tx_handler;
}

int subjectServerMain(const char* subjectCode, const uint16_t port, const char* db_file, db_entry_t* db) {
    db = read_courses_db(db_file);
    udp_server_t* server = udp_server_start(port, on_server_init);
    while(1) {
        udp_server_receive(server);
    }
    udp_server_stop(server);
    free_courses_db(db);
    return 0;
}
