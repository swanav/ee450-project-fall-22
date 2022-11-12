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

#include "log.h"
#include "messages.h"
#include "networking.h"
#include "udp_server.h"
#include "utils.h"
#include "data/credentials.h"

#define CREDENTIALS_FILE "cred.txt"

typedef uint8_t auth_status_t;


static udp_ctx_t* udp_peer_ctx;
static credentials_t* credentials_db = NULL;

typedef enum {
    AUTH_REQUEST_VALIDATE = 0,
    AUTH_REQUEST_END
} auth_request_type_t;

typedef enum {
    AUTH_RESPONSE_VALIDATE = 0,
    AUTH_RESPONSE_END
} auth_response_type_t;


static auth_request_type_t get_request_type(udp_dgram_t* datagram) {
    if (datagram->data_len == 0 || datagram->data[0] >= AUTH_REQUEST_END) {
        return ERR_REQ_INVALID;
    }
    return datagram->data[0];
}

static void handle_auth_request_validate(const udp_dgram_t* req_dgram, udp_dgram_t* res_dgram) {

    res_dgram->data_len = 2;
    res_dgram->data[0] = AUTH_RESPONSE_VALIDATE;

    credentials_t credentials = {0};

    err_t result = credentials_decode(&credentials, (const uint8_t*) req_dgram->data + 1, req_dgram->data_len - 1);
    if (result == ERR_INVALID_PARAMETERS) {
        LOGEM("Failed to parse authentication request");
        res_dgram->data[0] = ERR_CREDENTIALS_INVALID_REQUEST;
        return;
    }

    auth_status_t auth_status = credentials_validate(credentials_db, &credentials);
    if (auth_status == ERR_INVALID_PARAMETERS) {
        LOGEM("Failed to validate credentials: Invalid Parameters");
        res_dgram->data[0] = ERR_CREDENTIALS_INVALID_REQUEST;
        return;
    }
    res_dgram->data[0] = auth_status;
}

static void udp_message_rx_handler(udp_ctx_t* ctx, udp_endpoint_t* source, udp_dgram_t* req_dgram) {
    udp_dgram_t resp_dgram = {0};
    
    auth_request_type_t req_type = get_request_type(req_dgram);

    if (req_type == AUTH_REQUEST_VALIDATE) {
        LOGIM(SERVER_C_MESSAGE_ON_AUTH_REQUEST_RECEIVED);
        handle_auth_request_validate(req_dgram, &resp_dgram);
    } else {
        LOGIM(SERVER_C_MESSAGE_ON_INVALID_REQUEST_RECEIVED);
        resp_dgram.data[0] = AUTH_RESPONSE_END;
        resp_dgram.data_len = 1;
    }

    udp_send(ctx, source, &resp_dgram);
}

static void udp_message_tx_handler(udp_ctx_t* server, udp_endpoint_t* dest, udp_dgram_t* datagram) {
    LOGIM(SERVER_C_MESSAGE_ON_AUTH_RESPONSE_SENT);
}

static void on_server_init(udp_ctx_t* udp) {
    LOGI(SERVER_C_MESSAGE_ON_BOOTUP, udp->port);
    udp->on_rx = udp_message_rx_handler;
    udp->on_tx = udp_message_tx_handler;
}

int main(int argc, char* argv[]) {
    credentials_db = credentials_init(CREDENTIALS_FILE);
    credentials_print(credentials_db);
    udp_peer_ctx = udp_start(SERVER_C_UDP_PORT_NUMBER, on_server_init);
    while(1) {
        udp_receive(udp_peer_ctx);
    }
    udp_stop(udp_peer_ctx);
    return 0;
}
