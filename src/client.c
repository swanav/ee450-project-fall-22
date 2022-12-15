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

#include "networking.h"
#include "protocol.h"
#include "log.h"
#include "utils.h"
#include "messages.h"
#include "constants.h"
#include "database.h"

LOG_TAG(client);

// #define CLIENT_TEST
#ifdef CLIENT_TEST
#define TEST_USERNAME "james"
#define TEST_PASSWORD "2kAnsa7s)"
#define TEST_COURSE_INPUT "EE604 CS100 EE450 CS310 EE608 CS561 EE658 CS435 EE520 CS356"
#endif // CLIENT_TEST

typedef struct __client_context_t {
    int auth_failure_count;
    pthread_t user_input_thread;
    pthread_t network_thread;
    sem_t semaphore;
    tcp_client_t *client;
    credentials_t creds;
} client_context_t;

static err_t collect_credentials(credentials_t* user) {

#ifndef CLIENT_TEST
    char buffer[max(CREDENTIALS_MAX_USERNAME_LEN, CREDENTIALS_MAX_PASSWORD_LEN) + 1];
#endif // CLIENT_TEST

    // Prompt for username
    printf(CLIENT_MESSAGE_INPUT_USERNAME);
    fflush(stdout);

#ifdef CLIENT_TEST
    user->username_len = strlen(TEST_USERNAME);
    memcpy(user->username, TEST_USERNAME, user->username_len);
    LOG_WARN(TEST_USERNAME"\r\n");
#else
    if (fgets(buffer, sizeof(buffer), stdin)) {
        memcpy(user->username, utils_string_rtrim_newlines(buffer), strlen(utils_string_rtrim_newlines(buffer))); 
        user->username_len = (uint8_t) strlen((char*) user->username);
    }

#endif
    if (user->username_len < CREDENTIALS_MIN_USERNAME_LEN || user->username_len > CREDENTIALS_MAX_USERNAME_LEN) {
        LOG_ERR(CLIENT_MESSAGE_USERNAME_OUT_OF_BOUNDS, CREDENTIALS_MIN_USERNAME_LEN, CREDENTIALS_MAX_USERNAME_LEN);
        return ERR_INVALID_PARAMETERS;
    }

    // Prompt for password
    printf(CLIENT_MESSAGE_INPUT_PASSWORD);
    fflush(stdout);
#ifdef CLIENT_TEST
    user->password_len = strlen(TEST_PASSWORD);
    memcpy(user->password, TEST_PASSWORD, user->password_len);
    LOG_WARN(TEST_PASSWORD"\r\n");
#else
    if (fgets(buffer, sizeof(buffer), stdin)) {
        memcpy(user->password, utils_string_rtrim_newlines(buffer), strlen(utils_string_rtrim_newlines(buffer))); 
        user->password_len = (uint8_t) strlen((char*) user->password);
    }
#endif 
    if (user->password_len < CREDENTIALS_MIN_PASSWORD_LEN || user->password_len > CREDENTIALS_MAX_PASSWORD_LEN) {
        LOG_ERR(CLIENT_MESSAGE_PASSWORD_OUT_OF_BOUNDS, CREDENTIALS_MIN_PASSWORD_LEN, CREDENTIALS_MAX_PASSWORD_LEN);
        return ERR_INVALID_PARAMETERS;
    }

    return ERR_OK;
}

static int collect_course_codes(uint8_t* course_code, uint8_t course_code_buffer_size) {
#ifdef CLIENT_TEST
    strncpy((char*) course_code, TEST_COURSE_INPUT, course_code_buffer_size);
#else
    fgets((char*) course_code, course_code_buffer_size, stdin);
#endif // CLIENT_TEST
    return utils_get_word_count((char*) course_code);
}

static int new_request_prompt(uint8_t* course_code_buffer, uint8_t course_code_buffer_size, uint8_t* category_buffer, uint8_t category_buffer_size) {
    // Prompt user for course codes
    printf(CLIENT_MESSAGE_INPUT_COURSE_NAME);
    fflush(stdout);
    int courses_count = collect_course_codes(course_code_buffer, course_code_buffer_size);
    if (courses_count == 1) {
        // Only one course code was entered. Prompt user for category
        printf(CLIENT_MESSAGE_INPUT_LOOKUP_CATEGORY);
        fflush(stdout);
        fgets((char*) category_buffer, category_buffer_size, stdin);
    }
    return courses_count;
}

static void on_authentication_success(client_context_t* ctx, uint8_t* username, uint8_t username_len) {
    // Authentication successful. Inform user
    ctx->auth_failure_count = AUTH_SUCCESS;
    LOG_INFO(CLIENT_MESSAGE_ON_AUTH_RESULT_SUCCESS);
}

static void on_authentication_failure(client_context_t* ctx, uint8_t* username, uint8_t username_len, uint8_t flags) {
    // Authentication failed. Inform user of reason
    LOG_ERR(CLIENT_MESSAGE_ON_AUTH_RESULT_FAILURE);
    if (AUTH_MASK_USER_NOT_FOUND(flags)) {
        LOG_ERR("Username does not exist");
    } else if (AUTH_MASK_PASSWORD_MISMATCH(flags)) {
        LOG_ERR("Password does not match");
    }

    // Increment failure count and exit if max attempts reached.
    if (++ctx->auth_failure_count == AUTH_MAX_ATTEMPTS) {
        LOG_ERR("Authentication Failed for %d attempts. Client will shut down.", AUTH_MAX_ATTEMPTS);
        exit(1);
    } else {
        // Warn user of remaining attempts
        LOG_WARN("Attempts remaining: %d", AUTH_MAX_ATTEMPTS - ctx->auth_failure_count);
    }
}

static void on_auth_result(client_context_t* ctx, tcp_sgmnt_t* sgmnt) {
    LOG_INFO(CLIENT_MESSAGE_ON_AUTH_RESULT, ctx->creds.username_len, ctx->creds.username, ctx->client->port);
    uint8_t flags = 0;
    // Decode the authentication result
    protocol_authentication_response_decode(sgmnt, &flags);
    if (AUTH_MASK_SUCCESS(flags)) {
        // Authentication success
        on_authentication_success(ctx, ctx->creds.username, ctx->creds.username_len);
    } else {
        // Authentication failure
        on_authentication_failure(ctx, ctx->creds.username, ctx->creds.username_len, flags);
    }
}

static void authenticate_user(client_context_t* ctx) {
    tcp_sgmnt_t sgmnt = {0};

    // Encode the authentication request
    if (protocol_authentication_request_encode(&ctx->creds, &sgmnt) != ERR_OK) {
        LOG_ERR("Failed to encode authentication request.");
        return;
    }

    // Send authentication request
    if (tcp_client_send(ctx->client, &sgmnt) == ERR_OK) {
        LOG_INFO(CLIENT_MESSAGE_ON_AUTH_REQUEST, ctx->creds.username_len, ctx->creds.username);
    }
}

static void on_setup_complete(client_context_t* ctx) {
    LOG_INFO(CLIENT_MESSAGE_ON_BOOTUP);
    ctx->client->user_data = ctx;
    sem_post(&ctx->semaphore);
}

static void send_request(client_context_t* ctx, int courses_count, uint8_t* course_code_buffer, uint8_t course_code_buffer_size, uint8_t* category_buffer, uint8_t category_buffer_size) {
    tcp_sgmnt_t sgmnt = {0};
    if (courses_count == 1) {
        // Only one course code was entered. Send a lookup request for the course code and category.
        courses_lookup_category_t category = database_courses_lookup_category_from_string(utils_string_trim((char*) category_buffer));
        if (category == COURSES_LOOKUP_CATEGORY_INVALID) {
            LOG_ERR("Invalid category.");
            sem_post(&ctx->semaphore);
            return;
        }
        // Encode the lookup request.
        protocol_courses_lookup_single_request_encode((const char*) course_code_buffer, strlen((const char*) course_code_buffer), category, &sgmnt);
    } else if (courses_count > 1) {
        LOG_DBG("Requesting course details for multiple course codes. (%s)", course_code_buffer);
        // Encode the lookup request for multiple courses.
        protocol_courses_lookup_multiple_request_encode(courses_count, course_code_buffer, course_code_buffer_size, &sgmnt);
    }
    // Send the request.
    if (tcp_client_send(ctx->client, &sgmnt) == ERR_OK) {
        if (courses_count == 1) {
            LOG_INFO("%.*s sent a request to the main server.", ctx->creds.username_len, ctx->creds.username);
        } else {
            LOG_INFO("%.*s sent a request with multiple CourseCode to the main server", ctx->creds.username_len, ctx->creds.username);
        }
    }
}

static void on_course_lookup_info(client_context_t* ctx, tcp_sgmnt_t* sgmnt) {
    LOG_DBG("Received course lookup info.");
    LOG_BUFFER(sgmnt->data, sgmnt->data_len);

    char course_code[10] = {0};
    uint8_t course_code_len = sizeof(course_code);
    courses_lookup_category_t category = COURSES_LOOKUP_CATEGORY_INVALID;
    char information[120] = {0};
    uint8_t information_len = sizeof(information);
    // Decode the single course lookup response.
    if (protocol_courses_lookup_single_response_decode(sgmnt, course_code, &course_code_len, &category, (uint8_t*) information, &information_len) != ERR_OK) {
        LOG_ERR("Failed to decode course lookup info.");
    } else if (category == COURSES_LOOKUP_CATEGORY_INVALID) {
        LOG_ERR("Invalid query.");
        // TODO: Course Not found.
    } else {
        // Print the course lookup info.
        LOG_INFO("The %s of %s is %.*s", database_courses_category_string_from_enum(category), course_code, information_len, information);
    }
}

static void on_course_multi_lookup(client_context_t* ctx, tcp_sgmnt_t* sgmnt) {
    LOG_DBG("Received course multi lookup info.");
    course_t* courses = NULL;
    // Decode the multiple courses lookup response. This is an allocating function.
    protocol_courses_lookup_multiple_response_decode(sgmnt, &courses);
    // Print the courses information.
    log_course_multi_lookup_result(courses);
    // Free the courses.
    protocol_courses_lookup_multiple_response_decode_dealloc(courses);
}

static err_t create_timeout(struct timespec* ts, time_t sec, time_t nsec) {
    // Get current time
    if (clock_gettime(CLOCK_REALTIME, ts) == -1) {
        LOG_ERR("Could not get current time.");
        return ERR_INVALID_PARAMETERS;
    }
    // Add timeout to current time
    ts->tv_sec += sec;
    ts->tv_nsec += nsec;
    return ERR_OK;
}

static void on_course_lookup_error(client_context_t* ctx, tcp_sgmnt_t* sgmnt) {
    err_t error_code = ERR_OK;
    uint8_t buffer[20] = {0};
    uint8_t buffer_len = sizeof(buffer);

    if (protocol_courses_error_decode(sgmnt, &error_code, buffer, &buffer_len) == ERR_OK) {
        if (error_code == ERR_COURSES_NOT_FOUND) {
            LOG_WARN("Didn't find the course: %.*s", buffer_len, (char*) buffer);
        } else {
            LOG_ERR("Unknown error code: %d", error_code);
        }
    } else {
        LOG_ERR("Failed to decode %d", protocol_courses_error_decode(sgmnt, &error_code, buffer, &buffer_len));
    }
}

static void* user_input_task(void* params) {

    client_context_t* ctx = (client_context_t*) params;
    struct timespec ts = {0};

    // Wait for the client to connect to the server.
    sem_wait(&ctx->semaphore);

    // Authenticate user and wait for response.
    do {
        bzero(&ctx->creds, sizeof(credentials_t));
        bzero(&ts, sizeof(ts));
        // Wait for user input.
        if (collect_credentials(&ctx->creds) == ERR_OK) {
            // Send authentication request.
            authenticate_user(ctx);
            // Create timeout.
            create_timeout(&ts, TCP_QUERY_TIMEOUT_DELAY_S, TCP_QUERY_TIMEOUT_DELAY_NS);
            // Wait for authentication response.
            if (sem_timedwait(&ctx->semaphore, &ts) < 0 && errno == ETIMEDOUT) {
                LOG_ERR(CLIENT_MESSAGE_ON_NETWORK_REQUEST_TIMEOUT);
            }
        }
    } while(ctx->auth_failure_count != AUTH_SUCCESS);

    // User has successfully authenticated.
    LOG_INFO("-------- User \"%.*s\" Authenticated --------", ctx->creds.username_len, ctx->creds.username);


    uint8_t course_code[COURSE_NAME_BUFFER_SIZE] = {0};
    uint8_t category[COURSE_CATEGORY_BUFFER_SIZE] = {0};

    while(1) {
        bzero(course_code, sizeof(course_code));
        bzero(category, sizeof(category));
        bzero(&ts, sizeof(ts));

        LOG_INFO("------------Start a new request------------");
        int count = new_request_prompt(course_code, sizeof(course_code), category, sizeof(category));
        // Send request for course lookup.
        send_request(ctx, count, course_code, sizeof(course_code), category, sizeof(category));
        // Create timeout.
        create_timeout(&ts, TCP_QUERY_TIMEOUT_DELAY_S, TCP_QUERY_TIMEOUT_DELAY_NS);
        // Wait for response.
        if (sem_timedwait(&ctx->semaphore, &ts) < 0 && errno == ETIMEDOUT) {
            LOG_ERR(CLIENT_MESSAGE_ON_NETWORK_REQUEST_TIMEOUT);
        }
        LOG_INFO("------------End of Request------------\r\n");
    }

    return NULL;
}

static void on_receive(tcp_client_t* client, tcp_sgmnt_t* sgmnt) {
    client_context_t* ctx = (client_context_t*) client->user_data;
    response_type_t response_type = protocol_get_request_type(sgmnt);
    if (response_type != RESPONSE_TYPE_AUTH) {
        LOG_INFO(CLIENT_MESSAGE_ON_RESPONSE, client->port);
    }
    switch (response_type) {
        case RESPONSE_TYPE_AUTH:
            // On authentication response
            on_auth_result(ctx, sgmnt);
            break;
        case RESPONSE_TYPE_COURSES_SINGLE_LOOKUP:
            // On course lookup response
            on_course_lookup_info(ctx, sgmnt);
            break;
        case RESPONSE_TYPE_COURSES_MULTI_LOOKUP:
            // On course multi lookup response
            on_course_multi_lookup(ctx, sgmnt);
            break;
        case RESPONSE_TYPE_COURSES_ERROR:
            // On course lookup error
            on_course_lookup_error(ctx, sgmnt);
            break;
        default:
            LOG_ERR("Unknown segment type. %d", response_type);
            break;
    }
    // Notify the user input task that the response has been received.
    sem_post(&ctx->semaphore);
}

static void on_tcp_disconnect(tcp_client_t* clientparams) {
    // TCP client disconnected. exit the program.
    LOG_ERR("Client disconnected from serverM.");
    exit(1);
}

static void* network_thread_task(void* params) {
    client_context_t* ctx = (client_context_t*) params;

    // Create a new TCP endpoint
    tcp_endpoint_t dst = {0};
    // Set the destination IP address
    SERVER_ADDR_PORT(dst.addr, SERVER_M_TCP_PORT_NUMBER);

    // Create a new TCP client
    ctx->client = tcp_client_connect(&dst, on_receive, on_tcp_disconnect);

    // Set the user data to be passed to the callback functions
    on_setup_complete(ctx);

    // Listen for incoming data
    while (1) {
        tcp_client_receive(ctx->client);
    }

    // Close the TCP client
    tcp_client_disconnect(ctx->client);
    return NULL;
}

int main() {
    // Initialize the client context
    client_context_t ctx = {0};

    // Initialize the semaphore
    sem_init(&ctx.semaphore, 0, 0);

    // Start the user input thread
    pthread_create(&ctx.user_input_thread, NULL, &user_input_task, &ctx);

    // Start the network thread
    pthread_create(&ctx.network_thread, NULL, &network_thread_task, &ctx);

    // Wait for the network thread to finish
    pthread_join(ctx.network_thread, NULL);
    return 0;
}
