#include "log.h"

void log_message(struct __message_t message) {
    for (int i = 0; i < message.data_len; i++) {
        printf("0x%02X ", message.data[i]);
    }
    printf("\r\n");
}