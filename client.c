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

int login_user(user_credentials* user) {
    
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
