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

#define ON_BOOT "The main server is up and running."
#define ON_AUTH_REQUEST_RECEIVED "The main server received the authentication for %s using TCP over port %d."
#define ON_AUTH_REQUEST_FORWARDED "The main server sent an authentication request to serverC."
#define ON_AUTH_RESULT_RECEIVED "The main server received the result of the authentication request from ServerC using UDP over port %d."
#define ON_AUTH_RESULT_FORWARDED "The main server sent the authentication result to the client."
#define ON_QUERY_RECEIVED "The main server received from %s to query course %s about %s using TCP over port %d."
#define ON_QUERY_FORWARDED "The main server sent a request to server%s."
#define ON_RESULT_RECEIVED "The main server received the response from server%s using UDP over port %d."
#define ON_RESULT_FORWARDED "The main server sent the query information to the client."

/* Authentication Methods */

char shift_char(char c, int shift) {
    if (c >= 'A' && c <= 'Z') {
        c = (c - 'A' + shift) % 26 + 'A';
    } else if (c >= 'a' && c <= 'z') {
        c = (c - 'a' + shift) % 26 + 'a';
    } else if (c >= '0' && c <= '9') {
        c = (c - '0' + shift) % 10 + '0';
    }
    return c;
}

static int auth_encrypt(char* plaintext, size_t plaintext_len, char* ciphertext_buffer, size_t ciphertext_buffer_len, size_t* ciphertext_len) {
    if (ciphertext_buffer_len < plaintext_len) {
        LOGE("ciphertext buffer must be equal to or larger than %d bytes", (int) plaintext_len);
        return -1;
    }

    for (int i = 0; i < plaintext_len; i++) {
        ciphertext_buffer[i] = shift_char(plaintext[i], 4);
    }

    *ciphertext_len = plaintext_len;

    return 0;
}

// Create a TCP Server to communicate with the Client
int create_tcp_server(int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        LOGEM("socket() error");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero), 8);

    if (bind(sockfd, (struct sockaddr*) &server_addr, sizeof(struct sockaddr)) == -1) {
        LOGEM("bind() error");
        return -1;
    }

    if (listen(sockfd, 10) == -1) {
        LOGEM("listen() error");
        return -1;
    }

    return sockfd;
}

// Close the TCP Server
void close_tcp_server(int sockfd) {
    close(sockfd);
}

int main() {

    int sockfd = create_tcp_server(SERVER_M_TCP_PORT_NUMBER);
    if (sockfd < 0) {
        LOGEM("Failed to create TCP Server");
        exit(1);
    }

    LOGIM(SERVER_M_MESSAGE_ON_BOOTUP);


    // char plaintext1[50] = "Welcome to EE450!";
    // char ciphertext[50];
    // size_t ciphertext_len;

    // auth_encrypt(plaintext1, strlen(plaintext1), ciphertext, sizeof(ciphertext), &ciphertext_len);
    // LOGV("1 --> %s --> %.*s", plaintext1, (int) ciphertext_len, ciphertext);

    // char plaintext2[50] = "199@$";
    // auth_encrypt(plaintext2, strlen(plaintext2), ciphertext, sizeof(ciphertext), &ciphertext_len);
    // LOGV("2 --> %s --> %.*s", plaintext2, (int) ciphertext_len, ciphertext);
    
    // char plaintext3[50] = "0.27#&";
    // auth_encrypt(plaintext3, strlen(plaintext3), ciphertext, sizeof(ciphertext), &ciphertext_len);
    // LOGV("3 --> %s --> %.*s", plaintext3, (int) ciphertext_len, ciphertext);


    return 0;
}
