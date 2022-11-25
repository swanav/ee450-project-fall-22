/*-------------------------------------------------

                CREDENTIAL SERVER

Checks the credentials of the user and returns the 
authentication status.

---------------------------------------------------*/

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
#include "log.h"
#include "messages.h"
#include "networking.h"
#include "utils.h"
#include "data/credentials.h"

#include "protocol.h"

LOG_TAG(serverC);

static credentials_t* credentials_db = NULL;

static void handle_auth_request_validate(const udp_dgram_t* req_dgram, udp_dgram_t* res_dgram) {

    uint8_t flags = AUTH_FLAGS_FAILURE;

    credentials_t credentials = {0};

    err_t result = credentials_decode(&credentials, (const uint8_t*) req_dgram->data + REQUEST_RESPONSE_HEADER_LEN, req_dgram->data_len - REQUEST_RESPONSE_HEADER_LEN);

    if (result == ERR_INVALID_PARAMETERS) {
        LOG_ERR("Failed to parse authentication request");
    } else {
        LOG_INFO("Received authentication request for user %.*s", credentials.username_len, credentials.username);
        err_t auth_status = credentials_validate(credentials_db, &credentials);
        if (auth_status == ERR_INVALID_PARAMETERS) {
            LOG_WARN("Failed to validate credentials: Invalid Parameters");
        } else if (auth_status == ERR_CREDENTIALS_USER_NOT_FOUND) {
            flags |= AUTH_FLAGS_USER_NOT_FOUND;
        } else if (auth_status == ERR_CREDENTIALS_PASSWORD_MISMATCH) {
            flags |= AUTH_FLAGS_PASSWORD_MISMATCH;
        } else if (auth_status == ERR_CREDENTIALS_OK) {
            flags = AUTH_FLAGS_SUCCESS;
        }
    }
    protocol_encode(res_dgram, RESPONSE_TYPE_AUTH, flags, 0, NULL);
}

static void udp_message_rx_handler(udp_ctx_t* ctx, udp_endpoint_t* source, udp_dgram_t* req_dgram) {
    udp_dgram_t resp_dgram = {0};
    
    request_type_t req_type = protocol_get_request_type(req_dgram);
    if (req_type == REQUEST_TYPE_AUTH) {
        LOG_INFO(SERVER_C_MESSAGE_ON_AUTH_REQUEST_RECEIVED);
        handle_auth_request_validate(req_dgram, &resp_dgram);
    } else {
        LOG_WARN("Unknown request type: %d\n", req_type);
        LOG_INFO(SERVER_C_MESSAGE_ON_INVALID_REQUEST_RECEIVED);
        protocol_encode(&resp_dgram, RESPONSE_TYPE_AUTH, AUTH_FLAGS_FAILURE, 0, NULL);
    }

    udp_send(ctx, source, &resp_dgram);
}

static void udp_message_tx_handler(udp_ctx_t* server, udp_endpoint_t* dest, udp_dgram_t* datagram) {
    LOG_INFO(SERVER_C_MESSAGE_ON_AUTH_RESPONSE_SENT);
}

int main(int argc, char* argv[]) {
    credentials_db = credentials_init(CREDENTIALS_FILE);
    credentials_print(credentials_db);

    udp_ctx_t* udp = udp_start(SERVER_C_UDP_PORT_NUMBER);
    LOG_INFO(SERVER_C_MESSAGE_ON_BOOTUP, udp->port);
    udp->on_rx = udp_message_rx_handler;
    udp->on_tx = udp_message_tx_handler;

    while(1) {
        udp_receive(udp);
    }
    udp_stop(udp);

    return 0;
}
