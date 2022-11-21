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

#include "constants.h"
#include "credentials.h"
#include "data/courses.h"
#include "log.h"
#include "protocol.h"
#include "messages.h"
#include "udp_server.h"
#include "tcp_server.h"
#include "utils.h"

static udp_ctx_t* udp = NULL;
static tcp_server_t* tcp = NULL;

static udp_endpoint_t serverC; 
static udp_endpoint_t serverCS; 
static udp_endpoint_t serverEE;

static void authenticate_user(credentials_t* user, tcp_endpoint_t* src) {
    if (udp) {
        udp_dgram_t dgram = {0};
        credentials_t enc_user = {0};
        uint8_t data_buffer[1024] = {0};
        uint8_t data_buffer_len = 0;

        credentials_encrypt(user, &enc_user);
        credentials_encode(&enc_user, data_buffer, sizeof(data_buffer), &data_buffer_len);

        protocol_encode(&dgram, REQUEST_TYPE_AUTH, 0, data_buffer_len, data_buffer);

        udp_send(udp, &serverC, &dgram);
    }
}

static void on_auth_request_received(tcp_server_t* tcp, tcp_endpoint_t* src, tcp_sgmnt_t* sgmnt) {
    uint8_t buffer[128];
    uint8_t buffer_size = 0;
    credentials_t credentials = {0};

    protocol_decode(sgmnt, NULL, NULL, &buffer_size, sizeof(buffer), buffer);
    credentials_decode(&credentials, buffer, buffer_size);

    LOGI(SERVER_M_MESSAGE_ON_AUTH_REQUEST_RECEIVED, credentials.username, ntohs(src->addr.sin_port));

    authenticate_user(&credentials, src);
}

static void on_auth_response_received(udp_dgram_t* req_dgram) {
    LOGIM(SERVER_M_MESSAGE_ON_AUTH_RESULT_FORWARDED);
    tcp_server_send(tcp, tcp->endpoints, req_dgram);
}

static void request_course_lookup_info(courses_lookup_params_t* params) {
    if (udp) {
        udp_dgram_t dgram = {0};
        courses_lookup_info_encode(params, &dgram);
        if (strncasecmp(params->course_code, "EE", 2) == 0) {
            udp_send(udp, &serverEE, &dgram);
        } else if (strncasecmp(params->course_code, "CS", 2) == 0) {
            udp_send(udp, &serverCS, &dgram);
        } else {
            LOG_DEBUG("Invalid course code: %s", params->course_code);
        }
    }
}

static void on_course_lookup_info_request_received(tcp_server_t* tcp, tcp_endpoint_t* src, tcp_sgmnt_t* req_sgmnt) {
    LOGI("Received course lookup info request from " IP_ADDR_FORMAT, IP_ADDR(src));
    log_message(*req_sgmnt);
    courses_lookup_params_t params = {0};
    courses_lookup_info_decode(req_sgmnt, &params);
    LOGI("Course lookup info request: %s %s", courses_category_string_from_enum(params.category), params.course_code);
    request_course_lookup_info(&params);
}

static void on_udp_server_rx(udp_ctx_t* udp, udp_endpoint_t* source, udp_dgram_t* req_dgram) {
    LOGI(SERVER_M_MESSAGE_ON_AUTH_RESULT_RECEIVED, ntohs(source->addr.sin_port));
    uint8_t response_type = protocol_get_request_type(req_dgram);
    if (response_type == RESPONSE_TYPE_AUTH) {
        on_auth_response_received(req_dgram);
    } else if (response_type == RESPONSE_TYPE_COURSES_LOOKUP_INFO) {
        // on_course_lookup_info_response_received(req_dgram);
    } else {
        LOGIM("SERVER_M_MESSAGE_ON_UNKNOWN_REQUEST_TYPE");
    }
}

static void on_udp_server_tx(udp_ctx_t* udp, udp_endpoint_t* dest, udp_dgram_t* datagram) {
    LOGIM(SERVER_M_MESSAGE_ON_AUTH_REQUEST_FORWARDED);
}

static void on_tcp_server_rx(tcp_server_t* tcp, tcp_endpoint_t* src, tcp_sgmnt_t* req_sgmnt) {
    uint8_t request_type = protocol_get_request_type(req_sgmnt);
    switch (request_type) {
        case REQUEST_TYPE_AUTH:
            on_auth_request_received(tcp, src, req_sgmnt);
            break;
        case REQUEST_TYPE_COURSES_LOOKUP_INFO:
            on_course_lookup_info_request_received(tcp, src, req_sgmnt);
            break;
        default:
            LOGE("Unknown type: %d", request_type);
            break;
    }
}

static void on_tcp_server_tx(tcp_server_t* tcp, tcp_endpoint_t* dest, tcp_sgmnt_t* res_sgmnt) {
    LOGIM(SERVER_M_MESSAGE_ON_AUTH_RESULT_FORWARDED);
}

static void on_udp_server_init(udp_ctx_t* udp) {
    udp->on_rx = on_udp_server_rx;
    udp->on_tx = on_udp_server_tx;
}

static void on_tcp_server_init(tcp_server_t* tcp) {
    tcp->on_rx = on_tcp_server_rx;
    tcp->on_tx = on_tcp_server_tx;
}

// static void on_tcp_server_init_failure(start_failure_reason_t reason, int error_code) {
//     LOGE(SERVER_M_MESSAGE_ON_BOOTUP_FAILURE, strerror(error_code));
//     exit(1);
// }

static void on_udp_server_init_failure(start_failure_reason_t reason, int error_code) {
    LOGE(SERVER_M_MESSAGE_ON_BOOTUP_FAILURE, strerror(error_code));
    exit(1);
}


static void tick(tcp_server_t* tcp, udp_ctx_t* udp) {
    if (tcp != NULL) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        memcpy(&read_fds, &tcp->server_fd_set, sizeof(tcp->server_fd_set));
        FD_SET(udp->sd, &read_fds);

        struct timeval timeout = { .tv_sec = 1, .tv_usec = 100*1000 };
        int err = select(max(udp->sd, tcp->max_sd) + 1, &read_fds, NULL, NULL, &timeout);
        if (err > 0) {            
            for (int sd = min(tcp->sd, udp->sd); sd <= tcp->max_sd; sd++) {
                if (FD_ISSET(sd, &read_fds)) {
                    if (sd == udp->sd) {
                        udp_receive(udp);
                    } else if (sd == tcp->sd) {
                        tcp_server_accept(tcp);
                    } else {
                        tcp_server_receive(tcp, sd);
                    }
                }
            }
        }
    }
}
int main() {

    SERVER_ADDR_PORT(serverC, SERVER_C_UDP_PORT_NUMBER);
    SERVER_ADDR_PORT(serverCS, SERVER_CS_UDP_PORT_NUMBER);
    SERVER_ADDR_PORT(serverEE, SERVER_EE_UDP_PORT_NUMBER);

    udp = udp_start(SERVER_M_UDP_PORT_NUMBER, on_udp_server_init, on_udp_server_init_failure);
    if (!udp) {
        // LOGEM(SERVER_M_MESSAGE_ON_STARTUP_ERROR);
        LOGEM("serverM: Error starting UDP server");
        return 1;
    }

    tcp = tcp_server_start(SERVER_M_TCP_PORT_NUMBER, on_tcp_server_init);
    if (!tcp) {
        LOGEM("serverM: Error starting TCP server");
        return 1;
    }

    LOGIM(SERVER_M_MESSAGE_ON_BOOTUP);


    while(1) {
        tick(tcp, udp);
    }

    return 0;
}
