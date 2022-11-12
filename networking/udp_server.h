#ifndef UDP_SERVER_H
#define UDP_SERVER_H 

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "networking.h"

typedef struct __udp_ctx_t udp_ctx_t;

typedef void (*udp_message_rx_cb)(udp_ctx_t* udp, udp_endpoint_t* src, udp_dgram_t* dgram);
typedef void (*udp_message_tx_cb)(udp_ctx_t* udp, udp_endpoint_t* dst, udp_dgram_t* dgram);

struct __udp_ctx_t {
    int sd;
    int port;
    udp_message_rx_cb on_rx;
    udp_message_tx_cb on_tx;
};

typedef void (*udp_post_start_cb)(udp_ctx_t* udp);

udp_ctx_t* udp_start(int port, udp_post_start_cb on_init);
void udp_stop(udp_ctx_t* udp);
void udp_receive(udp_ctx_t* udp);
void udp_send(udp_ctx_t* udp, udp_endpoint_t* dest, udp_dgram_t* datagram);

#endif // UDP_SERVER_H
