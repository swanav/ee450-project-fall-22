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
#include "courses.h"

LOG_TAG(client);

// #define CLIENT_TEST

typedef struct __client_context_t {
    int auth_failure_count;
    pthread_t user_input_thread;
    pthread_t network_thread;
    sem_t semaphore;
    tcp_client_t *client;
    credentials_t creds;
} client_context_t;

err_t collect_credentials(credentials_t* user) {
    LOG_WARN("Enter username: ");

#ifdef CLIENT_TEST
    user->username_len = strlen("james");
    memcpy(user->username, "james", user->username_len);
    LOG_WARN("james\r\n");
#else
    if (scanf("%s", user->username)) {
        getc(stdin);
        user->username_len = (uint8_t) strlen((char*) user->username);
    }
#endif
    if (user->username_len < CREDENTIALS_MIN_USERNAME_LEN || user->username_len > CREDENTIALS_MAX_USERNAME_LEN) {
        LOG_ERR("Username length not within bounds (Expected: %d ~ %d characters)", CREDENTIALS_MIN_USERNAME_LEN, CREDENTIALS_MAX_USERNAME_LEN);
        return ERR_INVALID_PARAMETERS;
    }

    LOG_WARN("Enter password: ");
#ifdef CLIENT_TEST
    user->password_len = strlen("2kAnsa7s)");
    memcpy(user->password, "2kAnsa7s)", user->password_len);
    LOG_WARN("2kAnsa7s)\r\n");
#else
    if (scanf("%s", user->password)) {
        getc(stdin);
        user->password_len = (uint8_t) strlen((char*) user->password);
    }
#endif 
    if (user->password_len < CREDENTIALS_MIN_PASSWORD_LEN || user->password_len > CREDENTIALS_MAX_PASSWORD_LEN) {
        LOG_ERR("Password length not within bounds (Expected: %d ~ %d characters)", CREDENTIALS_MIN_PASSWORD_LEN, CREDENTIALS_MAX_PASSWORD_LEN);
        return ERR_INVALID_PARAMETERS;
    }

    return ERR_OK;
}

static void on_authentication_success(client_context_t* ctx, uint8_t* username, uint8_t username_len) {
    ctx->auth_failure_count = AUTH_SUCCESS;
    LOG_INFO(CLIENT_MESSAGE_ON_AUTH_RESULT_SUCCESS);
}

static void on_authentication_failure(client_context_t* ctx, uint8_t* username, uint8_t username_len, uint8_t flags) {
    LOG_ERR(CLIENT_MESSAGE_ON_AUTH_RESULT_FAILURE);

    if (AUTH_MASK_USER_NOT_FOUND(flags)) {
        LOG_ERR("User not found");
    } else if (AUTH_MASK_PASSWORD_MISMATCH(flags)) {
        LOG_ERR("Password mismatch");
    }

    if (++ctx->auth_failure_count == 3) {
        LOG_ERR("Maximum attempts reached. Closing client.");
        exit(1);
    } else {
        LOG_WARN("Please try again... %d attempts remaining", 3 - ctx->auth_failure_count);
    }
}

static void on_auth_result(client_context_t* ctx, tcp_sgmnt_t* sgmnt) {
    LOG_INFO(CLIENT_MESSAGE_ON_AUTH_RESULT, ctx->creds.username_len, ctx->creds.username, ntohs(ctx->client->server->addr.sin_port));
    uint8_t flags = 0;
    protocol_authentication_response_decode(sgmnt, &flags);
    if (AUTH_MASK_SUCCESS(flags)) {
        on_authentication_success(ctx, ctx->creds.username, ctx->creds.username_len);
    } else {
        on_authentication_failure(ctx, ctx->creds.username, ctx->creds.username_len, flags);
    }
}

static void authenticate_user(client_context_t* ctx) {
    tcp_sgmnt_t sgmnt = {0};

    if (protocol_authentication_request_encode(&ctx->creds, &sgmnt) != ERR_OK) {
        LOG_ERR("Failed to encode authentication request.");
        return;
    }

    if (tcp_client_send(ctx->client, &sgmnt) == ERR_OK) {
        LOG_INFO(CLIENT_MESSAGE_ON_AUTH_REQUEST, ctx->creds.username_len, ctx->creds.username);
    }
}

static void on_setup_complete(client_context_t* ctx) {
    LOG_INFO(CLIENT_MESSAGE_ON_BOOTUP);
    ctx->client->user_data = ctx;
    sem_post(&ctx->semaphore);
}

static int collect_course_codes(uint8_t* course_code, uint8_t course_code_buffer_size) {
#ifdef CLIENT_TEST
    strncpy((char*) course_code, "CS100 EE450 EE608 CS435 EE520", course_code_buffer_size);
#else
    fgets((char*) course_code, course_code_buffer_size, stdin);
#endif // CLIENT_TEST
    return utils_get_word_count((char*) course_code);
}

static int new_request_prompt(uint8_t* course_code_buffer, uint8_t course_code_buffer_size, uint8_t* category_buffer, uint8_t category_buffer_size) {
    LOG_WARN("Please enter the course code to query: ");
    int courses_count = collect_course_codes(course_code_buffer, course_code_buffer_size);
    if (courses_count == 1) {
        LOG_WARN("Please enter the category (Credit / Professor / Days / CourseName): ");
        fgets((char*) category_buffer, category_buffer_size, stdin);
    }
    return courses_count;
}

static void send_request(client_context_t* ctx, int courses_count, uint8_t* course_code_buffer, uint8_t course_code_buffer_size, uint8_t* category_buffer, uint8_t category_buffer_size) {
    tcp_sgmnt_t sgmnt = {0};
    if (courses_count == 1) {
        courses_lookup_category_t category = courses_lookup_category_from_string(utils_string_trim((char*) category_buffer));
        if (category == COURSES_LOOKUP_CATEGORY_INVALID) {
            LOG_ERR("Invalid category.");
            sem_post(&ctx->semaphore);
            return;
        }
        protocol_courses_lookup_single_request_encode((const char*) course_code_buffer, strlen((const char*) course_code_buffer), category, &sgmnt);
    } else if (courses_count > 1) {
        LOG_DBG("Requesting course details for multiple course codes. (%s)", course_code_buffer);
        protocol_courses_lookup_multiple_request_encode(courses_count, course_code_buffer, course_code_buffer_size, &sgmnt);
    }
    tcp_client_send(ctx->client, &sgmnt);
}

static void on_course_lookup_info(client_context_t* ctx, tcp_sgmnt_t* sgmnt) {
    LOG_DBG("Received course lookup info.");
    LOG_BUFFER(sgmnt->data, sgmnt->data_len);

    char course_code[10] = {0};
    uint8_t course_code_len = sizeof(course_code);
    courses_lookup_category_t category = COURSES_LOOKUP_CATEGORY_INVALID;
    char information[120] = {0};
    uint8_t information_len = sizeof(information);
    if (protocol_courses_lookup_single_response_decode(sgmnt, course_code, &course_code_len, &category, (uint8_t*) information, &information_len) != ERR_OK) {
        LOG_ERR("Failed to decode course lookup info.");
    } else if (category == COURSES_LOOKUP_CATEGORY_INVALID) {
        LOG_ERR("Invalid query.");
    } else {
        LOG_INFO("The %s of %s is %.*s", courses_category_string_from_enum(category), course_code, information_len, information);
    }
}

static void on_course_multi_lookup(client_context_t* ctx, tcp_sgmnt_t* sgmnt) {
    LOG_DBG("Received course multi lookup info.");
    course_t* courses = courses_lookup_multiple_response_decode(sgmnt);
    log_course_multi_lookup_result(courses);
    // courses_free(courses);
}

static err_t create_timeout(struct timespec* ts, time_t sec, time_t nsec) {
    if (clock_gettime(CLOCK_REALTIME, ts) == -1) {
        LOG_ERR("Could not get current time.");
        return ERR_INVALID_PARAMETERS;
    }
    ts->tv_sec += sec;
    ts->tv_nsec += nsec;
    return ERR_OK;
}

static void* user_input_task(void* params) {

    client_context_t* ctx = (client_context_t*) params;
    struct timespec ts = {0};

    sem_wait(&ctx->semaphore);

    do {
        bzero(&ctx->creds, sizeof(credentials_t));
        bzero(&ts, sizeof(ts));
        if (collect_credentials(&ctx->creds) == ERR_OK) {
            authenticate_user(ctx);
            create_timeout(&ts, TCP_QUERY_TIMEOUT_DELAY_S, TCP_QUERY_TIMEOUT_DELAY_NS);
            if (sem_timedwait(&ctx->semaphore, &ts) < 0 && errno == ETIMEDOUT) {
                LOG_ERR(CLIENT_MESSAGE_ON_NETWORK_REQUEST_TIMEOUT);
            }
        }
    } while(ctx->auth_failure_count != AUTH_SUCCESS);

    LOG_INFO("-------- User \"%.*s\" Authenticated --------", ctx->creds.username_len, ctx->creds.username);


    uint8_t course_code[COURSE_NAME_BUFFER_SIZE] = {0};
    uint8_t category[COURSE_CATEGORY_BUFFER_SIZE] = {0};

    while(1) {
        bzero(course_code, sizeof(course_code));
        bzero(category, sizeof(category));
        bzero(&ts, sizeof(ts));

        LOG_INFO("------------Start a new request------------");
        int count = new_request_prompt(course_code, sizeof(course_code), category, sizeof(category));
        send_request(ctx, count, course_code, sizeof(course_code), category, sizeof(category));
        create_timeout(&ts, TCP_QUERY_TIMEOUT_DELAY_S, TCP_QUERY_TIMEOUT_DELAY_NS);
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
    LOG_INFO(CLIENT_MESSAGE_ON_RESPONSE, ctx->client->port);
    switch (response_type) {
        case RESPONSE_TYPE_AUTH:
            on_auth_result(ctx, sgmnt);
            break;
        case RESPONSE_TYPE_COURSES_SINGLE_LOOKUP:
            on_course_lookup_info(ctx, sgmnt);
            break;
        case RESPONSE_TYPE_COURSES_MULTI_LOOKUP:
            on_course_multi_lookup(ctx, sgmnt);
            break;
        default:
            LOG_ERR("Unknown segment type. %d", response_type);
            break;
    }
    sem_post(&ctx->semaphore);
}

static void on_tcp_disconnect(tcp_client_t* clientparams) {
    LOG_ERR("Client disconnected from serverM.");
    exit(1);
}

static void* network_thread_task(void* params) {
    client_context_t* ctx = (client_context_t*) params;

    tcp_endpoint_t dst = {0};
    SERVER_ADDR_PORT(dst.addr, SERVER_M_TCP_PORT_NUMBER);

    ctx->client = tcp_client_connect(&dst, on_receive, on_tcp_disconnect);
    on_setup_complete(ctx);
    while (1) {
        tcp_client_receive(ctx->client);
    }
    tcp_client_disconnect(ctx->client);
    return NULL;
}

int main() {
    client_context_t ctx = {0};
    sem_init(&ctx.semaphore, 0, 0);
    pthread_create(&ctx.user_input_thread, NULL, &user_input_task, &ctx);
    pthread_create(&ctx.network_thread, NULL, &network_thread_task, &ctx);
    pthread_join(ctx.network_thread, NULL);
    return 0;
}
