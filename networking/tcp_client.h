#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "networking.h"
#include "../utils/error.h"


struct __tcp_client_t;

typedef void (*tcp_receive_handler_t)(struct __tcp_client_t*, tcp_sgmnt_t*);
typedef void (*tcp_disconnect_handler_t)(struct __tcp_client_t*);

typedef struct __tcp_client_t
{
    int sd;
    tcp_endpoint_t* server;
    tcp_receive_handler_t on_receive;
    tcp_disconnect_handler_t on_disconnect;
    void* user_data;
} tcp_client_t;

tcp_client_t* tcp_client_connect(tcp_endpoint_t* dest, tcp_receive_handler_t on_receive, tcp_disconnect_handler_t on_disconnect);
void tcp_client_disconnect(tcp_client_t* client);

err_t tcp_client_send(tcp_client_t* client, tcp_sgmnt_t* sgmnt);
void tcp_client_receive(tcp_client_t* client);

#endif // TCP_CLIENT_H
