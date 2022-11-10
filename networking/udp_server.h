#ifndef UDP_SERVER_H
#define UDP_SERVER_H 

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "networking.h"

typedef struct __udp_server_t udp_server_t;

typedef void (*udp_message_rx_cb)(udp_server_t* server, udp_endpoint_t* source, udp_dgram_t* datagram);
typedef void (*udp_message_tx_cb)(udp_server_t* server, udp_endpoint_t* dest, udp_dgram_t* datagram);

struct __udp_server_t {
    int sd;
    int port;
    udp_message_rx_cb on_rx;
    udp_message_tx_cb on_tx;
};


typedef void (*udp_post_start_cb)(udp_server_t* server);

udp_server_t* udp_server_start(int port, udp_post_start_cb on_init);
void udp_server_stop(udp_server_t* server);
void udp_server_receive(udp_server_t* server);
void udp_server_send(udp_server_t* server, udp_endpoint_t* dest, udp_dgram_t* datagram);

#endif // UDP_SERVER_H
