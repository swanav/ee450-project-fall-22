#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>

#include "constants.h"
#include "database.h"
#include "log.h"
#include "protocol.h"
#include "messages.h"
#include "networking.h"
#include "utils.h"

LOG_TAG(serverM);

static udp_ctx_t* udp = NULL;
static tcp_server_t* tcp = NULL;

static udp_endpoint_t serverC; 
static udp_endpoint_t serverCS; 
static udp_endpoint_t serverEE;

course_t* multi_course_response = NULL;

static pthread_t thread;
static sem_t semaphore;

static void on_course_lookup_error_received(udp_dgram_t *req_dgram);

/* ======================================== Authentication ============================================= */

static void set_username(char* username, uint8_t username_len) {
    strncpy(tcp->username, username, min(username_len, CREDENTIALS_MAX_USERNAME_LEN));
}

static void clear_username() {
    memset(tcp->username, 0, CREDENTIALS_MAX_USERNAME_LEN);
}

static void authenticate_user(credentials_t* user) {
    if (udp) {
        udp_dgram_t dgram = {0};
        credentials_t enc_user = {0};

        // Encrypt the request using the logic from the assignment
        if (database_credentials_encrypt(user, &enc_user) == ERR_OK) {
            // Encode the authentication request
            if (protocol_authentication_request_encode(&enc_user, &dgram) == ERR_OK) {
                // Send the request to the authentication server
                udp_send(udp, &serverC, &dgram);
                LOG_INFO(SERVER_M_MESSAGE_ON_AUTH_REQUEST_FORWARDED);
            }
        }
    }
}

static void on_auth_request_received(tcp_server_t* tcp, tcp_endpoint_t* src, tcp_sgmnt_t* sgmnt) {
    credentials_t credentials = {0};
    // Decode the authentication request
    if (protocol_authentication_request_decode(sgmnt, &credentials) == ERR_OK) {
        LOG_INFO(SERVER_M_MESSAGE_ON_AUTH_REQUEST_RECEIVED, credentials.username, ntohs(src->addr.sin_port));
        // Store the username of the authenticating user.
        set_username((char*) credentials.username, credentials.username_len);
        // Authenticate the user
        authenticate_user(&credentials);
    }
}

static void on_auth_response_received(udp_dgram_t* req_dgram) {
    // Response received for authentication result. Forward to client.
    uint8_t auth_result = AUTH_SUCCESS;
    protocol_authentication_response_decode(req_dgram, &auth_result);
    if (AUTH_MASK_FAILURE(auth_result)) {
        // Clear the username if the user failed to authenticate
        clear_username();
    }
    tcp_server_send(tcp, tcp->endpoints, req_dgram);
    LOG_INFO(SERVER_M_MESSAGE_ON_AUTH_RESULT_FORWARDED);
}

/* ============================================================================================================ */

static udp_endpoint_t* get_department_server_endpoint(const char* course_code) {
    if (strncasecmp((char*) course_code, DEPARTMENT_PREFIX_EE, DEPARTMENT_PREFIX_LEN) == 0) {
        return &serverEE;
    } else if (strncasecmp((char*) course_code, DEPARTMENT_PREFIX_CS, DEPARTMENT_PREFIX_LEN) == 0) {
        return &serverCS;
    } else {
        return NULL;
    }
}

static void send_request_to_department_server(udp_dgram_t* dgram, const char* course_code, uint8_t course_code_len) {
    // Figure out which department server to send the request to based on the course code
    udp_endpoint_t* endpoint = get_department_server_endpoint((char*) course_code);
        if (endpoint) {
            // Send the request to the department server
            udp_send(udp, endpoint, dgram);
            LOG_INFO(SERVER_M_MESSAGE_ON_QUERY_FORWARDED, 2, course_code);
        } else {
            // Send an error response to the client
            LOG_WARN("Invalid course code: %.*s", course_code_len, course_code);
            protocol_courses_error_encode(ERR_COURSES_NOT_FOUND, (uint8_t*) course_code, course_code_len, dgram);
            on_course_lookup_error_received(dgram);
            // sem_post(&semaphore);
        }
}

static void request_course_category_information(char* course_code, uint8_t course_code_len, courses_lookup_category_t category) {
    if (udp) {
        udp_dgram_t dgram = {0};
        // Encode the course category lookup request
        protocol_courses_lookup_single_request_encode(course_code, course_code_len, category, &dgram);
        // send request to department server
        send_request_to_department_server(&dgram, course_code, course_code_len);
    }
}

static void request_course_details(const uint8_t* course_code, const uint8_t course_code_len) {
    if (udp) {
        udp_dgram_t dgram = {0};
        // Encode the course detail lookup request
        protocol_courses_lookup_detail_request_encode(course_code, course_code_len, &dgram);
        // Send the request to the department server
        send_request_to_department_server(&dgram, (const char*) course_code, course_code_len);
    }
}

static void on_course_lookup_info_request_received(tcp_server_t* tcp, tcp_endpoint_t* src, tcp_sgmnt_t* req_sgmnt) {
    LOG_INFO("Received course lookup info request from " IP_ADDR_FORMAT, IP_ADDR(src));

    char course_code[10] = {0};
    uint8_t size = sizeof(course_code);
    courses_lookup_category_t category = COURSES_LOOKUP_CATEGORY_INVALID;

    // Decode Single Course Lookup Request
    if (protocol_courses_lookup_single_request_decode(req_sgmnt, course_code, &size, &category) == ERR_OK) {
        LOG_INFO(SERVER_M_MESSAGE_ON_QUERY_RECEIVED, tcp->username, course_code, database_courses_category_string_from_enum(category), ntohs(src->addr.sin_port));
        // Send Single Course Lookup Request to Department Server
        request_course_category_information(course_code, size, category);
    } else {
        LOG_ERR("Failed to decode course lookup info request");
    }
}

static course_t* insert_to_end_of_linked_list(course_t* list, course_t* item) {
    item->next = NULL;
    if (list == NULL) {
        list = item;
    } else {
        course_t* ptr = list;
        while (ptr->next) {
            ptr = ptr->next;
        }
        ptr->next = item;
    }
    return list;
}

static void drop_linked_list(course_t* ptr) {
    while (ptr) {
        course_t* next = ptr->next;
        free(ptr);
        ptr = next;
    }
}

static void single_course_code_handler(const uint8_t idx, const char* course_code, const uint8_t course_code_len) {
    LOG_DBG("%d) Requesting course details for %.*s", idx, course_code_len, course_code);
    // Request course details for individual course code
    request_course_details((const uint8_t*) course_code, course_code_len);
    // Wait for response
    sem_wait(&semaphore);
}

static void* multi_request_thread(void* params) {
    LOG_DBG("Starting multi request thread");
    tcp_sgmnt_t* req_sgmnt = (tcp_sgmnt_t*) params;

    uint8_t courses_length = 0;
    multi_course_response = NULL;
    // Decode the multiple course lookup request. single_course_code_handler is called for each course code
    protocol_courses_lookup_multiple_request_decode(req_sgmnt, &courses_length, single_course_code_handler);
    LOG_DBG("Received multi request for %d courses", courses_length);
    log_courses(multi_course_response);

    tcp_sgmnt_t sgmnt = {0};
    // Encode the multiple course lookup response.
    protocol_courses_lookup_multiple_response_encode(multi_course_response, &sgmnt);
    // Send the multiple course lookup response.
    tcp_server_send(tcp, tcp->endpoints, &sgmnt);

    // Free the linked list containing the course information.
    course_t* ptr = multi_course_response;
    drop_linked_list(ptr);
    multi_course_response = NULL;
    thread = (pthread_t) NULL;

    return NULL;
}

static void on_course_lookup_multi_request_received(tcp_server_t* tcp, tcp_endpoint_t* src, tcp_sgmnt_t* req_sgmnt) {
    LOG_DBG("Received course lookup multiple request from " IP_ADDR_FORMAT, IP_ADDR(src));
    // Start a new thread to handle the request
    pthread_create(&thread, NULL, multi_request_thread, (void*) req_sgmnt);
}

static void on_single_course_lookup_info_response_received(udp_dgram_t* req_dgram) {
    // Forward the single course lookup response to the client
    tcp_server_send(tcp, tcp->endpoints, req_dgram);
    LOG_INFO(SERVER_M_MESSAGE_ON_RESULT_FORWARDED);
}

static void on_course_lookup_error_received(udp_dgram_t* req_dgram) {
    // On course lookup error response from department server
    err_t error_code = ERR_OK;
    protocol_courses_error_decode(req_dgram, &error_code, NULL, NULL);
    LOG_WARN("Received course lookup error (%d).", error_code);
    if (!thread) {
        // This is a single course query. Send the response.
        tcp_server_send(tcp, tcp->endpoints, req_dgram);
    } else {
        // This is a multi course query. Signal the thread to continue.
        sem_post(&semaphore);
    }
}

static void on_udp_server_rx(udp_ctx_t* udp, udp_endpoint_t* source, udp_dgram_t* req_dgram) {
    // Received a response from a backend server
    uint8_t response_type = protocol_get_request_type(req_dgram);
    if (response_type == RESPONSE_TYPE_AUTH) {
        LOG_INFO(SERVER_M_MESSAGE_ON_AUTH_RESULT_RECEIVED, ntohs(source->addr.sin_port));
        // On auth response from auth server
        on_auth_response_received(req_dgram);
    } else if (response_type == RESPONSE_TYPE_COURSES_SINGLE_LOOKUP) {
        LOG_INFO("Received course lookup info response for a single course.");
        // On single course lookup response from department server
        on_single_course_lookup_info_response_received(req_dgram);
    } else if (response_type == RESPONSE_TYPE_COURSES_DETAIL_LOOKUP) {
        LOG_INFO("Received course detail response.");
        course_t* course = calloc(1, sizeof(course_t));
        // On course detail response from department server
        if (protocol_courses_lookup_detail_response_decode(req_dgram, course) == ERR_OK) {
            // Insert into linked list
            multi_course_response = insert_to_end_of_linked_list(multi_course_response, course);
        }
        // Signal semaphore to place next request
        sem_post(&semaphore);
    } else if (response_type == RESPONSE_TYPE_COURSES_ERROR) {
        on_course_lookup_error_received(req_dgram);
    } else {
        LOG_ERR("SERVER_M_MESSAGE_ON_UNKNOWN_REQUEST_TYPE: %d", response_type);
        // On unknown response type
        sem_post(&semaphore);
    }
}

static void on_tcp_server_rx(tcp_server_t* tcp, tcp_endpoint_t* src, tcp_sgmnt_t* req_sgmnt) {
    uint8_t request_type = protocol_get_request_type(req_sgmnt);
    switch (request_type) {
        case REQUEST_TYPE_AUTH:
            // Received auth request
            on_auth_request_received(tcp, src, req_sgmnt);
            break;
        case REQUEST_TYPE_COURSES_SINGLE_LOOKUP:
            // Received a request for a single course lookup
            on_course_lookup_info_request_received(tcp, src, req_sgmnt);
            break;
        case REQUEST_TYPE_COURSES_MULTI_LOOKUP:
            // Received a request for multiple courses
            on_course_lookup_multi_request_received(tcp, src, req_sgmnt);
            break;
        default:
            LOG_ERR("Unknown type: %d", request_type);
            break;
    }
}

static void tick(tcp_server_t* tcp, udp_ctx_t* udp) {
    if (tcp != NULL) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        memcpy(&read_fds, &tcp->server_fd_set, sizeof(tcp->server_fd_set));
        FD_SET(udp->sd, &read_fds);

        struct timeval timeout = { .tv_sec = 1, .tv_usec = 100*1000 };
        int err = select(max(udp->sd, tcp->max_sd) + 1, &read_fds, NULL, NULL, &timeout);
        if (err > 0) {            
            for (int sd = min(tcp->sd, udp->sd); sd <= tcp->max_sd; sd++) {
                if (FD_ISSET(sd, &read_fds)) {
                    if (sd == udp->sd) {
                        // Data on UDP socket
                        udp_receive(udp);
                    } else if (sd == tcp->sd) {
                        // Data on TCP Parent socket
                        tcp_server_accept(tcp);
                    } else {
                        // Data on TCP Child socket
                        tcp_server_receive(tcp, sd);
                    }
                }
            }
        }
    }
}

int main() {

    // Initialize semaphores
    sem_init(&semaphore, 0, 0);

    // Initialize server addresses
    SERVER_ADDR_PORT(serverC.addr, SERVER_C_UDP_PORT_NUMBER);
    SERVER_ADDR_PORT(serverCS.addr, SERVER_CS_UDP_PORT_NUMBER);
    SERVER_ADDR_PORT(serverEE.addr, SERVER_EE_UDP_PORT_NUMBER);

    // Start UDP server
    udp = udp_start(SERVER_M_UDP_PORT_NUMBER);
    if (!udp) {
        // UDP server failed to start. Exit.
        LOG_ERR(SERVER_M_MESSAGE_ON_BOOTUP_FAILURE, "Error starting UDP server");
        return 1;
    }

    // Start TCP server
    tcp = tcp_server_start(SERVER_M_TCP_PORT_NUMBER);
    if (!tcp) {
        // TCP server failed to start. Exit.
        LOG_ERR(SERVER_M_MESSAGE_ON_BOOTUP_FAILURE, "Error starting TCP server");
        return 1;
    }

    // Register callbacks
    udp->on_rx = on_udp_server_rx;
    tcp->on_rx = on_tcp_server_rx;

    LOG_INFO(SERVER_M_MESSAGE_ON_BOOTUP);

    // Start listening for requests
    while(1) {
        tick(tcp, udp);
    }

    return 0;
}
