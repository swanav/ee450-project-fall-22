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
#include "utils.h"

#define CREDENTIALS_FILE "cred.txt"

typedef enum {
    AUTH_OK,
    AUTH_FAIL_UNKNOWN,
    AUTH_DB_NOT_FOUND,
    AUTH_FAIL_NO_USER,
    AUTH_FAIL_PASS_MISMATCH,
} auth_status_t;

auth_status_t authenticate_user(char* username, size_t username_len, char* password, size_t password_len) {
    auth_status_t ret = AUTH_FAIL_NO_USER;
    FILE* fp = csv_open(CREDENTIALS_FILE);
    char line[1024];

    if (fp == NULL) {
        return AUTH_DB_NOT_FOUND;
    }

    // go through each line of the file
    // lines contain comma separated username and password
    // check if the username and password match
    // if so, return AUTH_OK
    // if not, return AUTH_FAIL_PASS_MISMATCH
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

int auth_server_start() {
    // Create a UDP Socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        LOGE("Failed to create socket %d", errno);
        exit(1);
    }
    // Bind the socket to a port
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        LOGE("Failed to bind socket to port %d", 5000);
        exit(1);
    }
    return sockfd;
}

void auth_server_tick(int sockfd) {
    // Receive a message from the client
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buf[1024] = {0};
    LOGD("Waiting for a message on %d on port %d", sockfd, 5000);
    int n = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&client_addr, &client_addr_len);
    if (n < 0) {
        LOGE("Failed to receive message %s", "from client");
        return;
    }
    LOGD("Received message from %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    LOGD("Message: (%d) %.*s", n, n, buf);

    // Parse the message
    char* token = strtok(buf, ",");
    if (token == NULL) {
        LOGE("Invalid message %s", "");
        return;
    }
    char* username = token;
    token = strtok(NULL, ",");
    if (token == NULL) {
        LOGE("Invalid message %s", "");
        return;
    }
    char* password = token;

    // Authenticate the user
    auth_status_t status = authenticate_user(username, strlen(username), password, strlen(password));
    LOGD("Authentication status: %d", status);

    // Send the authentication status back to the client
    n = sendto(sockfd, &status, sizeof(status), 0, (struct sockaddr*)&client_addr, client_addr_len);
    if (n < 0) {
        LOGE("Failed to send message on socket %d", sockfd);
        return;
    }
}

void auth_server_stop(int sockfd) {
    // Close the socket
    close(sockfd);
}

int main(int argc, char* argv[]) {
    int sockfd = auth_server_start();
    while(1) {
        auth_server_tick(sockfd);
    }
    auth_server_stop(sockfd);
    return 0;
}
