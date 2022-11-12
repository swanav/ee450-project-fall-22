#ifndef NETWORKING_H
#define NETWORKING_H

#include <arpa/inet.h>

struct ip_dest_t {
    struct sockaddr_in addr;
    socklen_t addr_len;
};

typedef struct ip_dest_t tcp_endpoint_t;
typedef struct ip_dest_t udp_endpoint_t;

#define IP_ADDR_FORMAT "%s:%d"
#define IP_ADDR(ip) inet_ntoa(ip->addr.sin_addr), ntohs(ip->addr.sin_port)

struct __message_t {
    char data[1024];
    size_t data_len;
};

typedef struct __message_t tcp_sgmnt_t;
typedef struct __message_t udp_dgram_t;

#define ENCODE_SIMPLE_ERROR_MESSAGE(message, err_code) \
do { \
    (message)->data_len = 1; \
    (message)->data[0] = err_code; \
} while(0)

#endif // NETWORKING_H
