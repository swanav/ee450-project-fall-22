// Create a TCP Server
// 1. Create a socket
// 2. Bind the socket to a port
// 3. Listen for incoming connections
// 4. Accept incoming connections
// 5. Receive data from the client
// 6. Send data to the client
// 7. Close the connection
// 8. Close the socket
//

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <tcp_server.h>
#include <log.h>

// Create and Start a TCP Server
tcp_server_t* tcp_server_start(int port, tcp_post_start_cb on_init) {
    tcp_server_t* server = (tcp_server_t*) calloc(1, sizeof(tcp_server_t));

    if (server == NULL) {
        LOGEM("Failed to allocate memory for tcp_server_t");
    } else {
        server->sd = socket(AF_INET, SOCK_STREAM, 0);
        if (server->sd < 0) {
            LOGE("Failed to create socket. Error: %s.", strerror(errno));
            free(server);
        } else {
            struct sockaddr_in server_addr = {0};
            server_addr.sin_family = AF_INET;
            server_addr.sin_addr.s_addr = INADDR_ANY;
            server_addr.sin_port = htons(port);
            if (bind(server->sd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
                LOGE("Failed to bind socket. Error: %s.", strerror(errno));
                free(server);
            } else {
                if (listen(server->sd, 5) < 0) {
                    LOGE("Failed to listen on socket. Error: %s.", strerror(errno));
                } else {
                    LOGI("TCP Server started on port %d", port);
                    on_init(server);
                }
            }
        }
    }
    return server;
}

// Close the TCP Server
void tcp_server_stop(tcp_server_t* server) {
    if (server != NULL) {
        close(server->sd);
        free(server);
    }
}

static struct timeval timeout = {
    .tv_sec = 0,
    .tv_usec = 100*1000
};

static void tcp_server_accept(tcp_server_t* server) {
    if (server != NULL) {
        int new_sd = -1;
        if ((new_sd = accept(server->sd, NULL, NULL)) < 0) {
            LOGE("Failed to accept connection. Error: %s.", strerror(errno));
        } else {
            if (new_sd > server->max_sd) {
                server->max_sd = new_sd;
            }
        }
    }
}

void tcp_server_tick(tcp_server_t* server) {
    if (server != NULL) {
        fd_set read_fds = {0};
        memcpy(&read_fds, &server->server_fd_set, sizeof(server->server_fd_set));
        if (select(server->max_sd + 1, &read_fds, NULL, NULL, &timeout) > 0) {
            for (int sd = server->sd; sd <= server->max_sd; sd++) {
                if (FD_ISSET(sd, &read_fds)) {
                    if (sd == server->sd) {
                        tcp_server_accept(server);
                    } else {
                        tcp_server_receive(server);
                    }
                }
            }
        }
    }
}

void tcp_server_receive(tcp_server_t* server) {
    if (server != NULL) {
        char buffer[1024] = {0};
        int bytes_read = read(server->sd, buffer, sizeof(buffer));
        if (bytes_read < 0) {
            LOGE("Failed to read from socket. Error: %s.", strerror(errno));
        } else if (bytes_read == 0) {
            LOGIM("Client disconnected.");
        } else {
            LOGI("Received %d bytes from client: %s", bytes_read, buffer);
        }
    }
}

void tcp_server_send(tcp_server_t* server, tcp_endpoint_t* dst, tcp_sgmnt_t* segment) {
    if (server != NULL && dst != NULL && segment != NULL) {
        LOGD("Sending TCP Segment to "IP_ADDR_FORMAT, IP_ADDR(dst));
        int bytes_sent = sendto(server->sd, segment->data, segment->data_len, 0, (struct sockaddr*) &dst->addr, dst->addr_len);
        if (bytes_sent < 0) {
            LOGE("Failed to send TCP Segment. Error: %s.", strerror(errno));
        } else {
            LOGD("Sent %d bytes", bytes_sent);
        }
    }
}
