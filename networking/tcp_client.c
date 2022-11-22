#include "tcp_client.h"
#include "../utils/log.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

LOG_TAG(tcp_client);

tcp_client_t* tcp_client_connect(tcp_endpoint_t* dest, tcp_receive_handler_t on_receive, tcp_disconnect_handler_t on_disconnect) {
    tcp_client_t* client = malloc(sizeof(tcp_client_t));
    if (!client) {
        LOG_ERR("tcp_client_connect: Error allocating memory for client");
        return NULL;
    }

    client->server = dest;

    client->sd = socket(AF_INET, SOCK_STREAM, 0);
    if (client->sd < 0) {
        LOG_ERR("tcp_client_connect: Error creating socket");
        free(client);
        return NULL;
    }

    if (connect(client->sd, (struct sockaddr*)&dest->addr, dest->addr_len) < 0) {
        LOG_ERR("tcp_client_connect: Error connecting to server");
        free(client);
        return NULL;
    }

    client->on_receive = on_receive;
    client->on_disconnect = on_disconnect;

    return client;
}

void tcp_client_disconnect(tcp_client_t* client) {
    if (client) {
        close(client->sd);
        free(client);
    }
}

err_t tcp_client_send(tcp_client_t* client, tcp_sgmnt_t* sgmnt) {
    if (client) {
        // (*sgmnt);
        if (send(client->sd, sgmnt->data, sgmnt->data_len, 0) > 0) {
            return ERR_OK;
        }
    }
    return ERR_INVALID_PARAMETERS;
}

void tcp_client_receive(tcp_client_t* client) {
    if (client) {
        tcp_sgmnt_t sgmnt;
        int bytes_read = recv(client->sd, sgmnt.data, sizeof(sgmnt.data), 0);
        if (bytes_read > 0) {
            sgmnt.data_len = bytes_read;
            LOG_INFO("Received %d bytes from server", bytes_read);
            if (client->on_receive) {
                client->on_receive(client, &sgmnt);
            }
        } else {
            if (client->on_disconnect) {
                client->on_disconnect(client);
            }
        }
    }
}
