#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <tcp_server.h>
#include <log.h>

LOG_TAG(tcp_server)

// Create and Start a TCP Server
tcp_server_t* tcp_server_start(int port, tcp_post_start_cb on_init) {
    tcp_server_t* server = (tcp_server_t*) calloc(1, sizeof(tcp_server_t));

    if (server == NULL) {
        LOG_ERR("Failed to allocate memory for tcp_server_t");
    } else {
        server->sd = socket(AF_INET, SOCK_STREAM, 0);
        if (server->sd < 0) {
            LOG_ERR("Failed to create socket. Error: %s.", strerror(errno));
            free(server);
        } else {
            struct sockaddr_in server_addr = {0};
            server_addr.sin_family = AF_INET;
            server_addr.sin_addr.s_addr = INADDR_ANY;
            server_addr.sin_port = htons(port);

            setsockopt(server->sd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

            if (bind(server->sd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
                LOG_ERR("Failed to bind socket. Error: %s.", strerror(errno));
                free(server);
            } else {
                if (listen(server->sd, 5) < 0) {
                    LOG_ERR("Failed to listen on socket. Error: %s.", strerror(errno));
                } else {
                    server->max_sd = server->sd;
                    FD_SET(server->sd, &server->server_fd_set);
                    LOG_INFO("TCP Server started on port %d", port);
                    if (on_init) {
                        on_init(server);
                    }
                }
            }
        }
    }
    return server;
}

// Close a TCP Child Socket
static void close_child_socket(tcp_server_t* server, int child_sd) {
    if (server != NULL) {
        close(child_sd);
        FD_CLR(child_sd, &server->server_fd_set);
        if (child_sd == server->max_sd) {
            server->max_sd--;
        }
    }
}

// Close the TCP Server
void tcp_server_stop(tcp_server_t* server) {
    if (server != NULL) {
        while(server->max_sd > server->sd) {
            close_child_socket(server, server->max_sd);
        }
        close(server->sd);
        free(server);
    }
}

// Create a new TCP Child Socket
static void create_child_socket(tcp_server_t* server, int child_sd) {
    if (server != NULL) {
        tcp_endpoint_t* endpoint = calloc(1, sizeof(tcp_endpoint_t));
        endpoint->sd = child_sd;
        endpoint->next = server->endpoints;
        server->endpoints = endpoint;
        endpoint->addr = (struct sockaddr_in) {0};
        endpoint->addr_len = sizeof(endpoint->addr);
        if (getpeername(child_sd, (struct sockaddr*) &endpoint->addr, &endpoint->addr_len) < 0) {
            LOG_ERR("Failed to get peer name. Error: %s.", strerror(errno));
        } else {
            LOG_INFO("Peer name: " IP_ADDR_FORMAT, IP_ADDR(endpoint));
        }

        FD_SET(child_sd, &server->server_fd_set);
        if (child_sd > server->max_sd) {
            server->max_sd = child_sd;
        }
    }
}

// Accept a new connection. Open a Child Socket
void tcp_server_accept(tcp_server_t* server) {
    if (server != NULL) {
        int new_sd = -1;
        if ((new_sd = accept(server->sd, NULL, NULL)) < 0) {
            LOG_ERR("Failed to accept connection. Error: %s.", strerror(errno));
        } else {
            LOG_INFO("Accepted connection on socket %d", new_sd);
            create_child_socket(server, new_sd);
        }
    }
}

// Get TCP Endpoint from Socket Descriptor
static tcp_endpoint_t* get_endpoint(tcp_server_t* server, int sd) {
    tcp_endpoint_t* endpoint = NULL;
    if (server != NULL) {
        endpoint = server->endpoints;
        while(endpoint != NULL) {
            if (endpoint->sd == sd) {
                break;
            }
            endpoint = endpoint->next;
        }
    }
    return endpoint;
}

// Receive data from a Child Socket
void tcp_server_receive(tcp_server_t* server, int child_sd) {
    uint8_t buffer[1024] = {0};
    int bytes_read = read(child_sd, buffer, sizeof(buffer));
    if (bytes_read < 0) {
        LOG_ERR("Failed to read from socket. Error: %s.", strerror(errno));
    } else if (bytes_read == 0) {
        LOG_INFO("Client disconnected.");
        close_child_socket(server, child_sd);
    } else {
        tcp_endpoint_t* endpoint = get_endpoint(server, child_sd);
        LOG_INFO("Received %d bytes from "IP_ADDR_FORMAT" : %s", bytes_read, IP_ADDR(endpoint), buffer);
        LOG_BUFFER(buffer, bytes_read);
        tcp_sgmnt_t sgmnt = {0};
        memcpy(sgmnt.data, buffer, bytes_read);
        sgmnt.data_len = bytes_read;
        server->on_rx(server, endpoint, &sgmnt);
    }
}

// Server Loop. Accept new connections and receive data
void tcp_server_tick(tcp_server_t* server) {
    if (server != NULL) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        memcpy(&read_fds, &server->server_fd_set, sizeof(server->server_fd_set));

        struct timeval timeout = { .tv_sec = 1, .tv_usec = 100*1000 };

        int err = select(server->max_sd + 1, &read_fds, NULL, NULL, &timeout);
        if (err > 0) {
            for (int sd = server->sd; sd <= server->max_sd; sd++) {
                if (FD_ISSET(sd, &read_fds)) {
                    if (sd == server->sd) {
                        tcp_server_accept(server);
                    } else {
                        tcp_server_receive(server, sd);
                    }
                }
            }
        }
    }
}

// Send data to a Child Socket
void tcp_server_send(tcp_server_t* server, tcp_endpoint_t* dst, tcp_sgmnt_t* segment) {
    if (server != NULL && dst != NULL && segment != NULL) {
        LOG_INFO("Sending TCP Segment to "IP_ADDR_FORMAT" %.*s", IP_ADDR(dst), (int) segment->data_len, segment->data);
        int bytes_sent = sendto(dst->sd, segment->data, segment->data_len, 0, (struct sockaddr*) &dst->addr, dst->addr_len);
        if (bytes_sent < 0) {
            LOG_ERR("Failed to send TCP Segment. Error: %s.", strerror(errno));
        } else {
            LOG_INFO("Sent %d bytes", bytes_sent);
        }
    }
}
