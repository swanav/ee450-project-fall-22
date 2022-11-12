#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <udp_server.h>
#include <log.h>

udp_ctx_t* udp_start(int port, udp_post_start_cb on_init) {
    udp_ctx_t* udp = (udp_ctx_t*) calloc(1, sizeof(udp_ctx_t));

    if (udp == NULL) {
        LOGEM("Failed to allocate memory for udp_server_t");
    } else {
        udp->port = port;
        udp->sd = socket(AF_INET, SOCK_DGRAM, 0);
        if (udp->sd < 0) {
            LOGE("Failed to create socket on port %d. Error: %s.", port, strerror(errno));
            free(udp);
        } else {
            struct sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_addr.s_addr = INADDR_ANY;
            server_addr.sin_port = htons(port);
            if (bind(udp->sd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
                LOGE("Failed to bind socket to port %d. Error: %s.", port, strerror(errno));
                free(udp);
            } else {
                LOGD("UDP Server started on port %d", port);
                on_init(udp);
            }
        }
    }

    return udp;
}

void udp_stop(udp_ctx_t* udp) {
    if (udp != NULL) {
        close(udp->sd);
        free(udp);
    }
}

void udp_receive(udp_ctx_t* udp) {
    if (udp != NULL) {
        LOGDM("Waiting for a UDP Datagram");
        udp_endpoint_t src = {0};
        src.addr_len = sizeof(src.addr);
        udp_dgram_t dgram = {0};

        dgram.data_len = recvfrom(udp->sd, dgram.data, sizeof(dgram.data), 0, (struct sockaddr*) &src.addr, &src.addr_len);
        if (dgram.data_len < 0) {
            LOGE("Failed to receive UDP Datagram. Error: %s.", strerror(errno));
        } else {
            LOGD("Received UDP Datagram from %s:%d", inet_ntoa(src.addr.sin_addr), ntohs(src.addr.sin_port));
            udp->on_rx(udp, &src, &dgram);
        }
    }
}

void udp_send(udp_ctx_t* udp, udp_endpoint_t* dst, udp_dgram_t* dgram) {
    if (udp != NULL && dst != NULL && dgram != NULL) {
        LOGD("Sending UDP Datagram to %s:%d", inet_ntoa(dst->addr.sin_addr), ntohs(dst->addr.sin_port));
        if (sendto(udp->sd, dgram->data, dgram->data_len, 0, (struct sockaddr*)&dst->addr, dst->addr_len) < 0) {
            LOGE("Failed to send UDP Datagram. Error: %s.", strerror(errno));
        } else {
            udp->on_tx(udp, dst, dgram);
        }
    }
}
