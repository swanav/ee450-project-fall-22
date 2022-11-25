#include <string.h>

#include "protocol.h"

void protocol_encode(struct __message_t* message, uint8_t type, uint8_t flags, uint16_t payload_len, uint8_t* payload) {
    if (sizeof(message->data) >= payload_len + REQUEST_RESPONSE_HEADER_LEN) {
        message->data[REQUEST_RESPONSE_TYPE_OFFSET] = type;
        message->data[REQUEST_RESPONSE_FLAGS_OFFSET] = flags;
        message->data[REQUEST_RESPONSE_PAYLOAD_LEN_OFFSET_1] = payload_len & 0xFF;
        message->data[REQUEST_RESPONSE_PAYLOAD_LEN_OFFSET_2] = payload_len >> 8;
        memcpy(message->data + REQUEST_RESPONSE_HEADER_LEN, payload, payload_len);
        message->data_len = REQUEST_RESPONSE_HEADER_LEN + payload_len;
    }
}

void protocol_decode(struct __message_t* message, request_type_t* request_type, uint8_t* flags, uint16_t *out_data_len, const uint16_t out_data_size, uint8_t* out_data) {
    if (request_type) {
        *request_type = protocol_get_request_type(message);
    }
    if (flags) {
        *flags = protocol_get_flags(message);
    }
    if (out_data_len && out_data) {
        *out_data_len = protocol_get_payload_len(message);
        if (*out_data_len > out_data_size) {
            *out_data_len = out_data_size;
        }
        memcpy(out_data, message->data + REQUEST_RESPONSE_HEADER_LEN, *out_data_len);
    }
}

request_type_t protocol_get_request_type(struct __message_t* message) {
    return message->data_len < REQUEST_RESPONSE_HEADER_LEN ? REQUEST_RESPONSE_INVALID_TYPE : message->data[REQUEST_RESPONSE_TYPE_OFFSET];
}

uint16_t protocol_get_payload_len(struct __message_t* message) {
    return message->data_len < REQUEST_RESPONSE_HEADER_LEN ? REQUEST_RESPONSE_INVALID_TYPE : message->data[REQUEST_RESPONSE_PAYLOAD_LEN_OFFSET_1] | (message->data[REQUEST_RESPONSE_PAYLOAD_LEN_OFFSET_2] << 8);
}

uint8_t protocol_get_flags(struct __message_t* message) {
    return message->data_len < REQUEST_RESPONSE_HEADER_LEN ? 0 : message->data[REQUEST_RESPONSE_FLAGS_OFFSET];
}
