#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "networking.h"

typedef struct __tcp_server_t tcp_server_t;

struct __tcp_server_t {
    int sd;
    int port;
    int max_sd;
    fd_set server_fd_set;
    // tcp_message_rx_cb on_rx;
    // tcp_message_tx_cb on_tx;
};

typedef void (*tcp_post_start_cb)(tcp_server_t* server);

tcp_server_t* tcp_server_start(int port, tcp_post_start_cb on_init);
void tcp_server_stop(tcp_server_t* server);
void tcp_server_receive(tcp_server_t* server);
void tcp_server_send(tcp_server_t* server, tcp_endpoint_t* dest, tcp_sgmnt_t* datagram);

#endif // TCP_SERVER_H
