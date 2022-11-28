#ifndef NETWORKING_H
#define NETWORKING_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "error.h"

struct ip_dest_t {
    struct sockaddr_in addr;
    int sd;
    struct ip_dest_t *next;
};

#define IP_ADDR_FORMAT "%s:%d"
#define IP_ADDR(ip) inet_ntoa(ip->addr.sin_addr), ntohs(ip->addr.sin_port)

struct __message_t {
    uint8_t data[1024];
    size_t data_len;
};

/* ------------------------------------------ TCP --------------------------------------------- */

typedef struct __message_t tcp_sgmnt_t;
typedef struct ip_dest_t tcp_endpoint_t;

#if defined(CLIENT)

typedef struct __tcp_client_t tcp_client_t;

typedef void (*tcp_receive_handler_t)(struct __tcp_client_t*, tcp_sgmnt_t*);
typedef void (*tcp_disconnect_handler_t)(struct __tcp_client_t*);

struct __tcp_client_t {
    int sd;
    int port;
    tcp_endpoint_t* server;
    tcp_receive_handler_t on_receive;
    tcp_disconnect_handler_t on_disconnect;
    void* user_data;
};

tcp_client_t* tcp_client_connect(tcp_endpoint_t* dest, tcp_receive_handler_t on_receive, tcp_disconnect_handler_t on_disconnect);
void tcp_client_disconnect(tcp_client_t* client);
err_t tcp_client_send(tcp_client_t* client, tcp_sgmnt_t* sgmnt);
void tcp_client_receive(tcp_client_t* client);
#endif // CLIENT

#if defined(SERVER_M)
typedef struct __tcp_server_t tcp_server_t;

typedef void (*tcp_message_tx_cb_t) (tcp_server_t* tcp, tcp_endpoint_t* dest, tcp_sgmnt_t* res_sgmnt);
typedef void (*tcp_message_rx_cb_t) (tcp_server_t* tcp, tcp_endpoint_t* dest, tcp_sgmnt_t* res_sgmnt);

struct __tcp_server_t {
    int sd;
    uint16_t port;
    int max_sd;
    fd_set server_fd_set;
    tcp_endpoint_t *endpoints;    
    tcp_message_rx_cb_t on_rx;
    tcp_message_tx_cb_t on_tx;
};

tcp_server_t* tcp_server_start(uint16_t port);
void tcp_server_stop(tcp_server_t* server);
void tcp_server_tick(tcp_server_t* server);
void tcp_server_send(tcp_server_t* server, tcp_endpoint_t* dest, tcp_sgmnt_t* datagram);

void tcp_server_receive(tcp_server_t* server, int child_sd);
void tcp_server_accept(tcp_server_t* server);
#endif // SERVER_M

/* ------------------------------------------ UDP --------------------------------------------- */

#if defined(SERVER_M) || defined(SERVER_C) || defined(SERVER_CS) || defined(SERVER_EE)
typedef struct __message_t udp_dgram_t;
typedef struct ip_dest_t udp_endpoint_t;

typedef struct __udp_ctx_t udp_ctx_t;

typedef void (*udp_message_rx_cb_t)(udp_ctx_t* udp, udp_endpoint_t* src, udp_dgram_t* dgram);
typedef void (*udp_message_tx_cb_t)(udp_ctx_t* udp, udp_endpoint_t* dst, udp_dgram_t* dgram);

struct __udp_ctx_t {
    int sd;
    uint16_t port;
    udp_message_rx_cb_t on_rx;
    udp_message_tx_cb_t on_tx;
};

udp_ctx_t* udp_start(uint16_t port);
void udp_stop(udp_ctx_t* udp);
void udp_receive(udp_ctx_t* udp);
void udp_send(udp_ctx_t* udp, udp_endpoint_t* dest, udp_dgram_t* datagram);
#endif // SERVER_M || SERVER_C || SERVER_CS || SERVER_EE

#endif // NETWORKING_H
