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

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

#define SERVER_ADDR_PORT(server, port) do { \
    server.addr.sin_family = AF_INET; \
    server.addr.sin_addr.s_addr = INADDR_ANY; \
    server.addr.sin_port = htons(port); \
    server.addr_len = sizeof(struct sockaddr_in); \
} while (0)


FILE* csv_open(const char* file_name);
void csv_close(FILE* fp);

/**
 * @brief Trim left whitespace from the string
 * 
 * @param s Pointer to string with whitespace
 * 
 * @return Pointer to trimmed string, NULL if s is NULL
 * 
 */ 
char* string_ltrim(char* s);

/**
 * @brief Trim left whitespace from the string
 * 
 * @param s Pointer to string with whitespace
 * 
 * @return Pointer to trimmed string, NULL if s is NULL
 * 
 */ 
char* string_rtrim(char* s);

/**
 * @brief Trim whitespace from the string
 * 
 * @param s Pointer to string with whitespace
 * 
 * @return Pointer to trimmed string, NULL if s is NULL
 * 
 */ 
char* string_trim(char* s);

/// Count words in a string
int utils_get_word_count(char* str);

#endif // __UTILS_H__
