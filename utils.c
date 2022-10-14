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

db_entry_t* read_courses_db(char* filename) {
    db_entry_t* head = NULL;
    db_entry_t* tail = NULL;
    FILE* fp = csv_open(filename);
    char line[1024];

    if (fp == NULL) {
        return NULL;
    }

    while (fgets(line, sizeof(line), fp)) {
        char* token = strtok(line, ",");
        if (token == NULL) {
            continue;
        }
        db_entry_t* entry = (db_entry_t*)malloc(sizeof(db_entry_t));
        if (entry == NULL) {
            LOGEM("Failed to allocate memory for db_entry_t");
            continue;
        }
        memset(entry, 0, sizeof(db_entry_t));
        strncpy(entry->course_code, token, sizeof(entry->course_code));
        token = strtok(NULL, ",");
        if (token == NULL) {
            continue;
        }
        entry->credits = atoi(token);
        token = strtok(NULL, ",");
        if (token == NULL) {
            continue;
        }
        strncpy(entry->professor, token, sizeof(entry->professor));
        token = strtok(NULL, ",");
        if (token == NULL) {
            continue;
        }
        strncpy(entry->days, token, sizeof(entry->days));
        token = strtok(NULL, ",");
        if (token == NULL) {
            continue;
        }
        strncpy(entry->course_name, token, sizeof(entry->course_name));
        if (head == NULL) {
            head = entry;
            tail = entry;
        } else {
            tail->next = entry;
            tail = entry;
        }
    }

    fclose(fp);
    return head;
}

void free_courses_db(db_entry_t* head) {
    db_entry_t* entry = head;
    while (entry != NULL) {
        db_entry_t* next = entry->next;
        free(entry);
        entry = next;
    }
}

int parse_lookup_request(udp_dgram_t* req_dgram, lookup_params_t* params) {
    char* token = strtok(req_dgram->data, ",");
    if (token == NULL) {
        return -1;
    }
    strncpy(params->course_code, token, sizeof(params->course_code));

    token = strtok(NULL, ",");
    if (token == NULL) {
        return -1;
    }

    strncpy(params->category, token, sizeof(params->category));

    return 0;
}

lookup_result_t course_lookup(const db_entry_t* db, char* category, char* course_code, char* info_buf, size_t info_buf_len) {
    db_entry_t* ptr = (db_entry_t*) db;
    while(ptr) {
        if (strcmp(ptr->course_code, course_code) == 0) {
            if (strcmp(category, "Credits") == 0) {
                snprintf(info_buf, info_buf_len, "%d", ptr->credits);
                return LOOKUP_OK;
            } else if (strcmp(category, "Professor") == 0) {
                strncpy(info_buf, ptr->professor, info_buf_len);
                return LOOKUP_OK;
            } else if (strcmp(category, "Days") == 0) {
                strncpy(info_buf, ptr->days, info_buf_len);
                return LOOKUP_OK;
            } else if (strcmp(category, "CourseName") == 0) {
                strncpy(info_buf, ptr->course_name, info_buf_len);
                return LOOKUP_OK;
            }
        }
        ptr =  ptr->next;
    }
    return LOOKUP_FAIL;
}
