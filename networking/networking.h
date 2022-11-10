#ifndef NETWORKING_H
#define NETWORKING_H

#include <arpa/inet.h>

typedef struct __udp_dest_t {
    struct sockaddr_in addr;
    socklen_t addr_len;
} udp_endpoint_t;

typedef struct __udp_dgram_t {
    char data[1024];
    size_t data_len;
} udp_dgram_t;

#endif // NETWORKING_H
