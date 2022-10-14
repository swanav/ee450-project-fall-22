#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include "log.h"
#include "messages.h"
#include "utils.h"

// CS100,Days

#define CS_DB_FILE "cs.txt"

static db_entry_t* cs_db = NULL;

static void udp_message_rx_handler(udp_server_t* server, udp_endpoint_t* source, udp_dgram_t* req_dgram) {
    lookup_params_t params = {0};
    if (parse_lookup_request(req_dgram, &params) == 0) {
        LOGI(SERVER__MESSAGE_ON_REQUEST_RECEIVED(CS), params.category, params.course_code);
        char information[128] = {0};
        lookup_result_t result = course_lookup(cs_db, params.category, params.course_code, information, sizeof(information));
        if (result == LOOKUP_OK) {
            LOGI(SERVER__MESSAGE_ON_COURSE_FOUND(CS), params.category, params.course_code, information);
        } else {
            LOGI(SERVER__MESSAGE_ON_COURSE_NOT_FOUND(CS), params.course_code);
        }
    }
}

static void udp_message_tx_handler(udp_server_t* server, udp_endpoint_t* dest, udp_dgram_t* datagram) {
    LOGIM(SERVER__MESSAGE_ON_RESPONSE_SENT(CS));
}

static void on_server_init(udp_server_t* server) {
    LOGI(SERVER__MESSAGE_ON_BOOTUP(CS), SERVER_CS_UDP_PORT_NUMBER);
    server->on_rx = udp_message_rx_handler;
    server->on_tx = udp_message_tx_handler;
}

int main() {
    cs_db = read_courses_db(CS_DB_FILE);
    udp_server_t* serverCS = udp_server_start(SERVER_CS_UDP_PORT_NUMBER, on_server_init);
    while(1) {
        udp_server_receive(serverCS);
    }
    udp_server_stop(serverCS);
    free_courses_db(cs_db);
    return 0;
}
