#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

#include "log.h"
#include "messages.h"


FILE* csv_open(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        LOGE("Failed to open file %s", filename);
        return NULL;
    }
    return fp;
}

void csv_close(FILE* fp) {
    fclose(fp);
}

// read csv file
void read_csv(const char *filename, int *n, int *d) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        LOGE("Failed to open file %s", filename);
        exit(1);
    }
    char line[1024];
    int i = 0;
    while (fgets(line, 1024, fp)) {
        LOGD("line %d: %s", i, line);
        // char *tmp = strdup(line);
        // int j = 0;
        // const char *tok;
        // for (tok = strtok(line, ","); tok && *tok; j++, tok = strtok(NULL, ",\n")) {
        //     data[i][j] = atof(tok);
        // }
        // if (i == 0) {
        //     *d = j;
        // }
        // free(tmp);
        i++;
    }
    *n = i;
    fclose(fp);
}


udp_server_t* udp_server_start(int port, udp_post_start_cb on_init) {
    udp_server_t* server = (udp_server_t*) calloc(1, sizeof(udp_server_t));

    if (server == NULL) {
        LOGEM("Failed to allocate memory for udp_server_t");
    } else {
        server->port = port;
        server->sd = socket(AF_INET, SOCK_DGRAM, 0);
        if (server->sd < 0) {
            LOGE("Failed to create socket on port %d. Error: %s.", port, strerror(errno));
            free(server);
        } else {
            struct sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_addr.s_addr = INADDR_ANY;
            server_addr.sin_port = htons(port);
            if (bind(server->sd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
                LOGE("Failed to bind socket to port %d. Error: %s.", port, strerror(errno));
                free(server);
            } else {
                on_init(server);
            }
        }
    }

    return server;
}

void udp_server_stop(udp_server_t* server) {
    if (server != NULL) {
        close(server->sd);
        free(server);
    }
}

void udp_server_receive(udp_server_t* server) {
    if (server != NULL) {
        LOGDM("Waiting for a UDP Datagram");
        udp_endpoint_t src = {0};
        src.addr_len = sizeof(src.addr);
        udp_dgram_t dgram = {0};

        dgram.data_len = recvfrom(server->sd, dgram.data, sizeof(dgram.data), 0, (struct sockaddr*) &src.addr, &src.addr_len);
        if (dgram.data_len < 0) {
            LOGE("Failed to receive UDP Datagram. Error: %s.", strerror(errno));
        } else {
            LOGD("Received UDP Datagram from %s:%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
            server->on_rx(server, &src, &dgram);
        }
    }
}

void udp_server_send(udp_server_t* server, udp_endpoint_t* dst, udp_dgram_t* dgram) {
    if (server != NULL && dst != NULL && dgram != NULL) {
        LOGD("Sending UDP Datagram to %s:%d", inet_ntoa(dst->addr.sin_addr), ntohs(dst->addr.sin_port));
        if (sendto(server->sd, dgram->data, dgram->data_len, 0, (struct sockaddr*)&dst->addr, dst->addr_len) < 0) {
            LOGE("Failed to send UDP Datagram. Error: %s.", strerror(errno));
        } else {
            server->on_tx(server, dst, dgram);
        }
    }
}
