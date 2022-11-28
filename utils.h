#ifndef __UTILS_H__
#define __UTILS_H__

#include "constants.h"

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

#define SERVER_ADDR_PORT(addr, port) do { \
    addr.sin_family = AF_INET; \
    addr.sin_addr.s_addr = htonl(LOCALHOST); \
    addr.sin_port = htons(port); \
} while (0)

/**
 * @brief Trim left whitespace from the string
 * 
 * @param s Pointer to string with whitespace
 * 
 * @return Pointer to trimmed string, NULL if s is NULL
 */ 
char* utils_string_ltrim(char* s);

/**
 * @brief Trim left whitespace from the string
 * 
 * @param s Pointer to string with whitespace
 * 
 * @return Pointer to trimmed string, NULL if s is NULL
 */ 
char* utils_string_rtrim(char* s);

/**
 * @brief Trim whitespace from the string
 * 
 * @param s Pointer to string with whitespace
 * 
 * @return Pointer to trimmed string, NULL if s is NULL
 */ 
char* utils_string_trim(char* s);

/**
 * @brief Trim newlines from the string
 * 
 * @param s Pointer to string with newlines
 * 
 * @return Pointer to trimmed string, NULL if s is NULL
 */
char* utils_string_rtrim_newlines(char* s);

/**
 * @brief Count words in a string
 *
 * @param str Pointer to string
 * 
 * @return Number of words in the string
 */
int utils_get_word_count(char* str);

/**
 * @brief Count characters in the word
 *
 * @param str Pointer to word
 * 
 * @return Number of characters in the word
 */
int utils_get_word_length(char* str);

#endif // __UTILS_H__
