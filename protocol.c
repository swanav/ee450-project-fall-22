#include <string.h>

#include "protocol.h"

// Create UDP or TCP packet based on given buffer
void protocol_encode(struct __message_t* message, uint8_t type, uint8_t flags, uint8_t payload_len, uint8_t* payload) {
    if (sizeof(message->data) >= payload_len + REQUEST_RESPONSE_HEADER_LEN) {
        message->data[REQUEST_RESPONSE_TYPE_OFFSET] = type;
        message->data[REQUEST_RESPONSE_FLAGS_OFFSET] = flags;
        message->data[REQUEST_RESPONSE_PAYLOAD_LEN_OFFSET] = payload_len;
        memcpy(message->data + REQUEST_RESPONSE_HEADER_LEN, payload, payload_len);
        message->data_len = REQUEST_RESPONSE_HEADER_LEN + payload_len;
    }
}

// Recreate buffer from given UDP or TCP packet
void protocol_decode(struct __message_t* message, request_type_t* request_type, uint8_t* flags, uint8_t *out_data_len, const uint8_t out_data_size, uint8_t* out_data) {
    if (request_type) {
        *request_type = message->data[REQUEST_RESPONSE_TYPE_OFFSET];
    }
    if (flags) {
        *flags = message->data[REQUEST_RESPONSE_FLAGS_OFFSET];
    }
    if (out_data_len && out_data) {
        *out_data_len = message->data[REQUEST_RESPONSE_PAYLOAD_LEN_OFFSET];
        if (*out_data_len > out_data_size) {
            *out_data_len = out_data_size;
        }
        memcpy(out_data, message->data + REQUEST_RESPONSE_HEADER_LEN, *out_data_len);
    }
}

request_type_t protocol_get_request_type(struct __message_t* message) {
    return message->data_len < REQUEST_RESPONSE_HEADER_LEN ? REQUEST_RESPONSE_INVALID_TYPE : message->data[REQUEST_RESPONSE_TYPE_OFFSET];
}

uint8_t protocol_get_payload_len(struct __message_t* message) {
    return message->data_len < REQUEST_RESPONSE_HEADER_LEN ? REQUEST_RESPONSE_INVALID_TYPE : message->data[REQUEST_RESPONSE_PAYLOAD_LEN_OFFSET];
}
