#ifndef _LOG_H_
#define _LOG_H_

#ifndef ENABLE_DEBUG_LOGS
#define ENABLE_DEBUG_LOGS 1
#endif // ENABLE_DEBUG_LOGS

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if ENABLE_DEBUG_LOGS
#define LOG_TAG(x) __attribute__((unused)) static const char* TAG = #x;

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
#define LOG_DBG(...)            log_text(LOG_LVL_DEBUG, TAG, __VA_ARGS__)
#define LOG_VERBOSE(...)        log_text(LOG_LVL_VERBOSE, TAG, __VA_ARGS__)
#define LOG_BUFFER(buffer, len) log_dbg_buffer(TAG, #buffer, buffer, len)

#define LOG_FUNCTION_ENTRY()             LOG_VERBOSE("Entering %s", __func__)
#define LOG_FUNCTION_ENTRY_DEVICE(index) LOG_VERBOSE("Entering %s for device index %d", __func__, index)
#define LOG_FUNCTION_EXIT()              LOG_VERBOSE("Exiting  %s", __func__)

#define LOG_VAR_NULL(var)  LOG_WARN("%s: %s -> null", __func__, #var)
#define LOG_VAR_STR(var)   LOG_DBG("%s: %s -> %s", __func__, #var, var ? var : "null")
#define LOG_VAR_INT(var)   LOG_DBG("%s: %s -> %d", __func__, #var, var)
#define LOG_VAR_HEX(var)   LOG_DBG("%s: %s -> 0x%04X", __func__, #var, var)
#define LOG_VAR_LONG(var)  LOG_DBG("%s: %s -> %ld", __func__, #var, var)
#define LOG_VAR_FLOAT(var) LOG_DBG("%s: %s -> %f", __func__, #var, var)

void log_text(const LogLevel_t logLevel, const char* tag, const char* format, ...);
void log_dbg_buffer(const char* tag, const char* buffer_name, const uint8_t* buffer, size_t len);
#else
#define LOG_TAG(x)

#define LOG_INIT()
#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERR(...)
#define LOG_DBG(...)
#define LOG_VERBOSE(...)
#define LOG_BUFFER(...)

#define LOG_FUNCTION_ENTRY()
#define LOG_FUNCTION_ENTRY_DEVICE(index)
#define LOG_FUNCTION_EXIT()

#define LOG_VAR_NULL(arg)
#define LOG_VAR_STR(var)
#define LOG_VAR_INT(var)
#define LOG_VAR_HEX(var)
#define LOG_VAR_LONG(var)
#define LOG_VAR_FLOAT(var)

#endif

#endif // _LOG_H_
