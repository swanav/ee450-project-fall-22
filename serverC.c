/*-------------------------------------------------

                CREDENTIAL SERVER

Checks the credentials of the user and returns the 
authentication status.

---------------------------------------------------*/

#include "database.h"
#include "constants.h"
#include "fileio.h"
#include "log.h"
#include "messages.h"
#include "networking.h"
#include "protocol.h"

LOG_TAG(serverC);

static credentials_t* credentials_db = NULL;

static void handle_auth_request_validate(const udp_dgram_t* req_dgram, udp_dgram_t* res_dgram) {

    uint8_t flags = AUTH_FLAGS_FAILURE;

    credentials_t credentials = {0};

    // Get the username and password from the request
    if (protocol_authentication_request_decode(req_dgram, &credentials) != ERR_OK) {
        LOG_ERR("Failed to parse authentication request");
    } else {
        LOG_INFO(SERVER_C_MESSAGE_ON_AUTH_REQUEST_RECEIVED_FOR_USER, credentials.username_len, credentials.username);
        // Check if the username and password are valid
        err_t auth_status = database_credentials_validate(credentials_db, &credentials);
        // Set the flags based on the authentication status
        if (auth_status == ERR_INVALID_PARAMETERS) {
            LOG_WARN("Failed to validate credentials: Invalid Parameters");
        } else if (auth_status == ERR_CREDENTIALS_USER_NOT_FOUND) {
            flags |= AUTH_FLAGS_USER_NOT_FOUND;
        } else if (auth_status == ERR_CREDENTIALS_PASSWORD_MISMATCH) {
            flags |= AUTH_FLAGS_PASSWORD_MISMATCH;
        } else if (auth_status == ERR_OK) {
            flags = AUTH_FLAGS_SUCCESS;
        }
    }
    // Send the response
    protocol_authentication_response_encode(flags, res_dgram);
}

static void udp_message_rx_handler(udp_ctx_t* ctx, udp_endpoint_t* source, udp_dgram_t* req_dgram) {

    // Received a message over UDP

    udp_dgram_t resp_dgram = {0};
    
    request_type_t req_type = protocol_get_request_type(req_dgram);
    if (req_type == REQUEST_TYPE_AUTH) {
        // Handle authentication request
        LOG_INFO(SERVER_C_MESSAGE_ON_AUTH_REQUEST_RECEIVED);
        handle_auth_request_validate(req_dgram, &resp_dgram);
    } else {
        // Invalid request
        LOG_INFO(SERVER_C_MESSAGE_ON_INVALID_REQUEST_RECEIVED);
        // Encode an error message response to the user
        protocol_authentication_response_encode(AUTH_FLAGS_FAILURE, &resp_dgram);
    }

    // Send the response to the received message
    udp_send(ctx, source, &resp_dgram);

    LOG_INFO(SERVER_C_MESSAGE_ON_AUTH_RESPONSE_SENT);
}

int main() {

    // Read and store the credentials database from `CREDENTIALS_FILE`
    credentials_db = fileio_credential_server_db_create(CREDENTIALS_FILE);

    // [Debug only] Log the credentials
    log_credentials(credentials_db);

    // Create a UDP context. Bind it to SERVER_C_UDP_PORT_NUMBER.
    udp_ctx_t* udp = udp_start(SERVER_C_UDP_PORT_NUMBER);
    if (!udp) {
        // UDP failed to start. Show an error and exit.
        LOG_ERR("SERVER_C_MESSAGE_ON_UDP_START_FAILURE");
        return -1;
    }

    // UDP Ready.
    LOG_INFO(SERVER_C_MESSAGE_ON_BOOTUP, udp->port);

    // Attach the UDP message handler
    udp->on_rx = udp_message_rx_handler;

    // Listen to UDP messages
    while(1) {
        udp_receive(udp);
    }

    // Stop the UDP context. Free the memory and exit.
    udp_stop(udp);

    return 0;
}
