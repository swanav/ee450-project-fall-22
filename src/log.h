#ifndef _LOG_H_
#define _LOG_H_

#ifndef ENABLE_DEBUG_LOGS
#define ENABLE_DEBUG_LOGS 0
#endif // ENABLE_DEBUG_LOGS

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define LOG_TAG(x) __attribute__((unused)) static const char* TAG = #x

typedef enum {
	LOG_LVL_OFF,
	LOG_LVL_ERROR,
	LOG_LVL_WARNING,
	LOG_LVL_INFO,
	LOG_LVL_DEBUG,
	LOG_LVL_VERBOSE,
} LogLevel_t;

#define LOG_INFO(...)           log_text(LOG_LVL_INFO, TAG, __VA_ARGS__)
#define LOG_WARN(...)           log_text(LOG_LVL_WARNING, TAG, __VA_ARGS__)
#define LOG_ERR(...)            log_text(LOG_LVL_ERROR, TAG, __VA_ARGS__)

#if ENABLE_DEBUG_LOGS
#define LOG_DBG(...)            log_text(LOG_LVL_DEBUG, TAG, __VA_ARGS__)
#define LOG_VERBOSE(...)        log_text(LOG_LVL_VERBOSE, TAG, __VA_ARGS__)
#define LOG_BUFFER(buffer, len) log_dbg_buffer(TAG, #buffer, buffer, len)
#else
#define LOG_DBG(...)
#define LOG_VERBOSE(...)
#define LOG_BUFFER(...)
#endif

void log_text(const LogLevel_t logLevel, const char* tag, const char* format, ...);
void log_dbg_buffer(const char* tag, const char* buffer_name, const uint8_t* buffer, size_t len);

/**
 * @brief Log a course
 * 
 * @param course Pointer to the course to log
*/
void log_course(const void* course);

/**
 * @brief Print the given courses linked list
 * 
 * @param head Pointer to the head of the linked list
 * 
*/
void log_courses(const void* head);

/**
 * @brief Print the given credentials
 * 
 * @param credentials The credentials to print
 */
void log_credential(const void* credentials);

/**
 * @brief Print the given credentials linked list
 * 
 * @param credentials The credentials linked list to print
 */
void log_credentials(const void* head);

/**
 * @brief Print the output of the course multiple lookup
 *
 * @param courses The courses to print
*/
void log_course_multi_lookup_result(const void* courses);

#endif // _LOG_H_
