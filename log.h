#ifndef __LOG__H__
#define __LOG__H__

#include <stdio.h>

#define LOGE(format, ...) LOG_E(E, format, __VA_ARGS__)
#define LOGW(format, ...) LOG_E(W, format, __VA_ARGS__)
#define LOGI(format, ...) LOG(I, format, __VA_ARGS__)
#define LOGD(format, ...) LOG(D, format, __VA_ARGS__)
#define LOGV(format, ...) LOG(V, format, __VA_ARGS__)
#define LOG_E(level, format, ...) fprintf(stderr, #level " | %*s | " format "\r\n", -10, __FILE__, __VA_ARGS__)
#define LOG(level, format, ...) fprintf(stdout, #level " | %*s | " format "\r\n", -10, __FILE__, __VA_ARGS__)


#endif // __LOG__H__
