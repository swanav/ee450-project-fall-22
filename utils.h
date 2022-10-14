#ifndef __UTILS_H__
#define __UTILS_H__

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

// USC Student ID: 1349754-053
#define USC_ID_LAST_3 53

#define SERVER_C_UDP_PORT_NUMBER  21000 + USC_ID_LAST_3
#define SERVER_CS_UDP_PORT_NUMBER 22000 + USC_ID_LAST_3
#define SERVER_EE_UDP_PORT_NUMBER 23000 + USC_ID_LAST_3
#define SERVER_M_UDP_PORT_NUMBER  24000 + USC_ID_LAST_3
#define SERVER_M_TCP_PORT_NUMBER  25000 + USC_ID_LAST_3
#define CLIENT_PORT_NUMBER    // DYNAMICALLY ASSIGNED PORT NUMBER

FILE* csv_open(const char* file_name);
void csv_close(FILE* fp);


typedef struct __udp_dest_t {
    struct sockaddr_in addr;
    socklen_t addr_len;
} udp_endpoint_t;

typedef struct __udp_dgram_t {
    char data[1024];
    size_t data_len;
} udp_dgram_t;

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

typedef struct __lookup_params_t {
    char category[64];
    char course_code[64];
} lookup_params_t;

typedef enum __lookup_result_t {
    LOOKUP_OK,
    LOOKUP_FAIL,
} lookup_result_t;

typedef struct __db_entry_t {
    char course_code[32];
    int credits;
    char professor[64];
    char days[32];
    char course_name[128];
    struct __db_entry_t* next;
} db_entry_t;

db_entry_t* read_courses_db(char* filename);
void free_courses_db(db_entry_t* head);
int parse_lookup_request(udp_dgram_t* req_dgram, lookup_params_t* params);
lookup_result_t course_lookup(const db_entry_t* db, char* category, char* course_code, char* info_buf, size_t info_buf_len);

#endif // __UTILS_H__
