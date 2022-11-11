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

#include "log.h"
#include "messages.h"
#include "utils.h"

int main() {

    // int sockfd = create_tcp_server(SERVER_M_TCP_PORT_NUMBER);
    // if (sockfd < 0) {
    //     LOGEM("Failed to create TCP Server");
    //     exit(1);
    // }

    LOGIM(SERVER_M_MESSAGE_ON_BOOTUP);



    return 0;
}
