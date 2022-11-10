#ifndef __UTILS_H__
#define __UTILS_H__

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

// USC Student ID: 1349754-053
#define USC_ID_LAST_3 53

#define SERVER_C_UDP_PORT_NUMBER  21000 + USC_ID_LAST_3
#define SERVER_CS_UDP_PORT_NUMBER 22000 + USC_ID_LAST_3
#define SERVER_EE_UDP_PORT_NUMBER 23000 + USC_ID_LAST_3
#define SERVER_M_UDP_PORT_NUMBER  24000 + USC_ID_LAST_3
#define SERVER_M_TCP_PORT_NUMBER  25000 + USC_ID_LAST_3
#define CLIENT_PORT_NUMBER    // DYNAMICALLY ASSIGNED PORT NUMBER

FILE* csv_open(const char* file_name);
void csv_close(FILE* fp);

#endif // __UTILS_H__
