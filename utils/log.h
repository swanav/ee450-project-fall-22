#ifndef __LOG__H__
#define __LOG__H__

#include <stdio.h>
#include "../networking/networking.h"

// #define SUBMIT_LOGS_FORMAT_ENABLED
#define LOG_DEBUG
#ifndef LOG_DEBUG
#define DLOG 0
#else
#define DLOG 1
#endif

#define LOGE(format, ...) LOG_E(E, format, __VA_ARGS__)

#define LOGI(format, ...) LOG(I, format, __VA_ARGS__)
#if DLOG
#define LOGW(format, ...) LOG_E(W, format, __VA_ARGS__)
#define LOGD(format, ...) LOG(D, format, __VA_ARGS__)
#define LOGV(format, ...) LOG(V, format, __VA_ARGS__)
#else
#define LOGW(format, ...)
#define LOGD(format, ...)
#define LOGV(format, ...)
#endif

#define LOGEM(message)    LOG_EM(E, message)

#define LOGIM(message)    LOGM(I, message)
#if DLOG
#define LOGWM(message)    LOG_EM(W, message)
#define LOGDM(message)    LOGM(D, message)
#define LOGVM(message)    LOGM(V, message)
#else
#define LOGWM(message)
#define LOGDM(message)
#define LOGVM(message)
#endif

#ifdef SUBMIT_LOGS_FORMAT_ENABLED
#define LOG_EM(level, message) fprintf(stderr, "%s\r\n", message)
#define LOGM(level, message)   fprintf(stdout, "%s\r\n", message)

#define LOG_E(level, format, ...) fprintf(stderr, format "\r\n", __VA_ARGS__)
#define LOG(level, format, ...)   fprintf(stdout, format "\r\n", __VA_ARGS__)
#else
#define LOG_EM(level, message) fprintf(stderr, #level " | %*s | " " %s " "\r\n", -10, __FILE__, message)
#define LOGM(level, message)   fprintf(stdout, #level " | %*s | " " %s " "\r\n", -10, __FILE__, message)

#define LOG_E(level, format, ...) fprintf(stderr, #level " | %*s | " format "\r\n", -10, __FILE__, __VA_ARGS__)
#define LOG(level, format, ...)   fprintf(stdout, #level " | %*s | " format "\r\n", -10, __FILE__, __VA_ARGS__)
#endif // SUBMIT_LOGS_FORMAT_ENABLED

void log_message(struct __message_t message);
void log_buffer(uint8_t* buffer, uint8_t buffer_len);

#endif // __LOG__H__