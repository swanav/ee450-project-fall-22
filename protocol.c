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

void protocol_decode(const struct __message_t* message, request_type_t* request_type, uint8_t* flags, uint16_t *out_data_len, const uint16_t out_data_size, uint8_t* out_data) {
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

request_type_t protocol_get_request_type(const struct __message_t* message) {
    return message->data_len < REQUEST_RESPONSE_HEADER_LEN ? REQUEST_RESPONSE_INVALID_TYPE : message->data[REQUEST_RESPONSE_TYPE_OFFSET];
}

uint16_t protocol_get_payload_len(const struct __message_t* message) {
    return message->data_len < REQUEST_RESPONSE_HEADER_LEN ? REQUEST_RESPONSE_INVALID_TYPE : message->data[REQUEST_RESPONSE_PAYLOAD_LEN_OFFSET_1] | (message->data[REQUEST_RESPONSE_PAYLOAD_LEN_OFFSET_2] << 8);
}

uint8_t protocol_get_flags(const struct __message_t* message) {
    return message->data_len < REQUEST_RESPONSE_HEADER_LEN ? 0 : message->data[REQUEST_RESPONSE_FLAGS_OFFSET];
}


err_t protocol_authentication_request_encode(const credentials_t* credentials, udp_dgram_t* out_dgrm) {
    if (credentials == NULL || out_dgrm == NULL) {
        return ERR_INVALID_PARAMETERS;
    }

    uint8_t credentials_buffer[sizeof(credentials_t)];
    uint8_t offset = 0;

    credentials_buffer[offset++] = credentials->username_len;
    credentials_buffer[offset++] = credentials->password_len;
    memcpy(credentials_buffer + offset, credentials->username, credentials->username_len);
    offset += credentials->username_len;
    memcpy(credentials_buffer + offset, credentials->password, credentials->password_len);
    offset += credentials->password_len;

    protocol_encode(out_dgrm, REQUEST_TYPE_AUTH, 0, offset, credentials_buffer);

    return ERR_OK;
}

err_t protocol_authentication_request_decode(const udp_dgram_t* in_dgrm, credentials_t* credentials) {

    if (!in_dgrm || !credentials) {
        return ERR_INVALID_PARAMETERS;
    }

    uint8_t buffer[REQUEST_RESPONSE_HEADER_LEN + sizeof(credentials_t)];
    uint16_t buffer_size = sizeof(buffer);

    request_type_t type = protocol_get_request_type(in_dgrm);
    if (type != REQUEST_TYPE_AUTH) {
        return ERR_INVALID_PARAMETERS;
    }

    protocol_decode(in_dgrm, NULL, NULL, &buffer_size, sizeof(buffer), buffer);

    credentials->username_len = CREDENTIALS_USERNAME_LEN(buffer);
    credentials->password_len = CREDENTIALS_PASSWORD_LEN(buffer);
    uint8_t offset = CREDENTIALS_HEADER_LEN;
    memcpy(credentials->username, buffer + offset, credentials->username_len);
    offset += credentials->username_len;
    memcpy(credentials->password, buffer + offset, credentials->password_len);
    return ERR_OK;
}

err_t protocol_authentication_response_encode(const uint8_t authentication_result, udp_dgram_t* out_dgrm) {
    if (out_dgrm == NULL) {
        return ERR_INVALID_PARAMETERS;
    }
    protocol_encode(out_dgrm, RESPONSE_TYPE_AUTH, authentication_result, 0, NULL);
    return ERR_OK;
}

err_t protocol_authentication_response_decode(const udp_dgram_t* in_dgrm, uint8_t* authentication_result) {
    if (in_dgrm == NULL || authentication_result == NULL) {
        return ERR_INVALID_PARAMETERS;
    }

    uint8_t buffer[REQUEST_RESPONSE_HEADER_LEN];

    request_type_t type = protocol_get_request_type(in_dgrm);
    if (type != RESPONSE_TYPE_AUTH) {
        return ERR_INVALID_PARAMETERS;
    }

    protocol_decode(in_dgrm, NULL, authentication_result, NULL, 0, buffer);

    return ERR_OK;
}
