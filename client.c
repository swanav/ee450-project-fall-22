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

#include "networking/udp_server.h"

typedef struct __user_credentials {
    char username[50];
    char password[50];
} user_credentials;

int collect_credentials(user_credentials* user) {
    // Store the username and password in the user_credentials struct
    // Return the user_credentials struct
    printf("Enter username: ");
    scanf("%s", user->username);
    printf("Enter password: ");
    scanf("%s", user->password);
    return 0;
}

static void on_init(udp_server_t* server) {
    // It is used to set up the server
    // In this case, we are going to collect the user's credentials
    user_credentials user;
    collect_credentials(&user);
}

int login_user(user_credentials* user) {
    udp_server_t *client = udp_server_start(21051, on_init);
    udp_endpoint_t dst = {0};
    dst.addr.sin_family = AF_INET;
    dst.addr.sin_addr.s_addr = INADDR_ANY;
    dst.addr.sin_port = htons(21053);
    dst.addr_len = sizeof(dst.addr);

    udp_dgram_t dgram = {0};
    memcpy(dgram.data, user, sizeof(user_credentials));
    dgram.data_len = sizeof(user_credentials);
    udp_server_send(client, &dst, &dgram);
    udp_server_receive(client);
    udp_server_stop(client);
    return 0;
}

int main() {
    printf("Hello, client.c!\r\n");

    user_credentials user;
    collect_credentials(&user);
    login_user(&user);

    printf("Username: %s\r\n", user.username);
    printf("Password: %s\r\n", user.password);


    return 0;
}
