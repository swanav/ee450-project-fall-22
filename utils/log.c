#include "log.h"

void log_message(struct __message_t message) {
    log_buffer(message.data, message.data_len);
}

void log_buffer(uint8_t* buffer, uint8_t buffer_len) {
    // printf("B | log.c | ");
    // for (int i = 0; i < buffer_len; i++) {
    //     printf("0x%02X ", buffer[i]);
    // }
    // printf("\r\n");
}
