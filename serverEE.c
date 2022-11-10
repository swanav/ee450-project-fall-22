// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <errno.h>
// #include <string.h>
// #include <netdb.h>
// #include <sys/types.h>
// #include <netinet/in.h>
// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <sys/wait.h>

// #include "log.h"
// #include "messages.h"
// #include "utils.h"

// #include "courses.h"
#include "serverSub.h"
#include "utils.h"
// #include "udp_server.h"

// // EE450,Days

// #define EE_DB_FILE "ee.txt"

// static db_entry_t* ee_db = NULL;

// static void udp_message_rx_handler(udp_server_t* server, udp_endpoint_t* source, udp_dgram_t* req_dgram) {
//     lookup_params_t params = {0};
//     if (parse_lookup_request(req_dgram->data, req_dgram->data_len, &params) == 0) {
//         LOGI(SERVER__MESSAGE_ON_REQUEST_RECEIVED(EE), params.category, params.course_code);
//         char information[128] = {0};
//         lookup_result_t result = course_lookup(ee_db, params.category, params.course_code, information, sizeof(information));
//         if (result == LOOKUP_OK) {
//             LOGI(SERVER__MESSAGE_ON_COURSE_FOUND(EE), params.category, params.course_code, information);
//         } else {
//             LOGI(SERVER__MESSAGE_ON_COURSE_NOT_FOUND(EE), params.course_code);
//         }
//     }
// }

// static void udp_message_tx_handler(udp_server_t* server, udp_endpoint_t* dest, udp_dgram_t* datagram) {
//     LOGIM(SERVER__MESSAGE_ON_RESPONSE_SENT(EE));
// }

// static void on_server_init(udp_server_t* server) {
//     LOGI(SERVER__MESSAGE_ON_BOOTUP(EE), SERVER_EE_UDP_PORT_NUMBER);
//     server->on_rx = udp_message_rx_handler;
//     server->on_tx = udp_message_tx_handler;
// }

int main() {
    const char* db_file = "ee.txt";
    db_entry_t* db = NULL;
    const char* subjectCode = "ee";
    const int16_t port = SERVER_EE_UDP_PORT_NUMBER;
    return subjectServerMain(subjectCode, port, db_file, db);
    // ee_db = read_courses_db(EE_DB_FILE);
    // udp_server_t* serverEE = udp_server_start(SERVER_EE_UDP_PORT_NUMBER, on_server_init);
    // while(1) {
    //     udp_server_receive(serverEE);
    // }
    // udp_server_stop(serverEE);
    // free_courses_db(ee_db);
    // return 0;
}
