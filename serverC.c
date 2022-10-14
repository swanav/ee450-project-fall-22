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
#include "utils.h"

#define CREDENTIALS_FILE "cred.txt"

typedef enum {
    AUTH_OK,
    AUTH_FAIL_UNKNOWN,
    AUTH_DB_NOT_FOUND,
    AUTH_FAIL_NO_USER,
    AUTH_FAIL_PASS_MISMATCH,
    AUTH_FAIL_INVALID_REQUEST,
} auth_status_t;

// TODO - Check Auth implementation

auth_status_t authenticate_user(char* username, size_t username_len, char* password, size_t password_len) {
    auth_status_t ret = AUTH_FAIL_NO_USER;
    FILE* fp = csv_open(CREDENTIALS_FILE);
    char line[1024];

    if (fp == NULL) {
        return AUTH_DB_NOT_FOUND;
    }

    LOGI("Authenticating user '%s' with password '%s'...", username, password);

    while (fgets(line, sizeof(line), fp)) {
        char* token = strtok(line, ",");
        if (token == NULL) {
            continue;
        }
        if (strncmp(token, username, username_len) == 0) {
            token = strtok(NULL, ",");
            if (token == NULL) {
                continue;
            }
            if (strncmp(token, password, password_len) == 0) {
                ret = AUTH_OK;
                break;
            } else {
                ret = AUTH_FAIL_PASS_MISMATCH;
                break;
            }
        }
    }

    csv_close(fp);

    return ret;
}

static int parse_authentication_request(udp_dgram_t* datagram, char* username_buf, size_t username_buf_len, char* password_buf, size_t password_buf_len) {
    char* token = strtok(datagram->data, ",");
    if (token == NULL) {
        return -1;
    }
    strncpy(username_buf, token, username_buf_len);
    token = strtok(NULL, ",");
    if (token == NULL) {
        return -1;
    }
    strncpy(password_buf, token, password_buf_len);
    return 0;
}

static void udp_message_rx_handler(udp_server_t* server, udp_endpoint_t* source, udp_dgram_t* req_dgram) {
    LOGIM(SERVER_C_MESSAGE_ON_AUTH_REQUEST_RECEIVED);
    char username[1024] = {0};
    char password[1024] = {0};
    auth_status_t status = AUTH_FAIL_UNKNOWN;
    if (parse_authentication_request(req_dgram, username, sizeof(username), password, sizeof(password)) == 0) {
        status = authenticate_user(username, strlen(username), password, strlen(password));
    } else {
        status = AUTH_FAIL_INVALID_REQUEST;
    }
    udp_dgram_t resp_dgram = {0};
    resp_dgram.data_len = sizeof(status);
    memcpy(resp_dgram.data, (char*)&status, sizeof(status));
    udp_server_send(server, source, &resp_dgram);
}

static void udp_message_tx_handler(udp_server_t* server, udp_endpoint_t* dest, udp_dgram_t* datagram) {
    LOGIM(SERVER_C_MESSAGE_ON_AUTH_RESPONSE_SENT);
}


static void on_server_init(udp_server_t* server) {
    LOGI(SERVER_C_MESSAGE_ON_BOOTUP, server->port);
    server->on_rx = udp_message_rx_handler;
    server->on_tx = udp_message_tx_handler;
}

int main(int argc, char* argv[]) {
    udp_server_t* serverC = udp_server_start(SERVER_C_UDP_PORT_NUMBER, on_server_init);

    while(1) {
        udp_server_receive(serverC);
    }

    udp_server_stop(serverC);
    return 0;
}
