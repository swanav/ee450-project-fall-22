#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "log.h"
#include "../protocol.h"

#define LOG_CYAN_STR        "\033[0;36m"
#define LOG_CYAN_BOLD_STR   "\033[1;36m"
#define LOG_GREEN_STR       "\033[0;32m"
#define LOG_GREEN_BOLD_STR  "\033[1;32m"
#define LOG_YELLOW_STR      "\033[0;33m"
#define LOG_YELLOW_BOLD_STR "\033[1;33m"
#define LOG_RED_STR         "\033[0;31m"
#define LOG_RED_BOLD_STR    "\033[1;31m"
#define LOG_ENDING_STR      "\033[0m\n"

#define BYTES_PER_LINE 16

typedef struct {
	const char* colorTag;
	const char* tagString;
	const char* highlightColorTag;
} LogInfo_t;

static const LogInfo_t LOG_INFO_TAGS[] = {
    {"", "", ""},
    {LOG_RED_STR, "E", LOG_RED_BOLD_STR},
    {LOG_YELLOW_STR, "W", LOG_YELLOW_BOLD_STR},
    {LOG_GREEN_STR, "I", LOG_GREEN_BOLD_STR},
    {LOG_CYAN_STR, "D", LOG_CYAN_BOLD_STR},
	{"", "V", ""}
};

void log_text(const LogLevel_t logLevel, const char* tag, const char* format, ...) {
    printf("%s%s %s : %s", LOG_INFO_TAGS[logLevel].highlightColorTag, LOG_INFO_TAGS[logLevel].tagString, tag, LOG_INFO_TAGS[logLevel].colorTag);
    va_list argptr;
    va_start(argptr, format);
    vprintf(format, argptr);
    va_end(argptr);
    printf(LOG_ENDING_STR);
}

void log_dbg_buffer(const char* tag, const char* buffer_name, const uint8_t* buffer, size_t len) {
	if (len > 0) {
		char        temp_buffer[BYTES_PER_LINE + 3]; // for not-byte-accessible memory
		const char* ptr_line;
		// format: field[length]
		// ADDR[10]+"   "+DATA_HEX[8*3]+" "+DATA_HEX[8*3]+"  |"+DATA_CHAR[8]+"|"
		char  hd_buffer[10 + 3 + BYTES_PER_LINE * 3 + 3 + BYTES_PER_LINE + 1 + 1];
		char* ptr_hd;
		int   bytes_cur_line;

		do {
			if (len > BYTES_PER_LINE) {
				bytes_cur_line = BYTES_PER_LINE;
			}
			else {
				bytes_cur_line = (int)len;
			}
			// use memcpy to get around alignment issue
			memcpy(temp_buffer, buffer, (bytes_cur_line + 3) / 4 * 4);
			ptr_line = temp_buffer;
			ptr_hd   = hd_buffer;

			ptr_hd += sprintf(ptr_hd, "%p ", buffer);
			for (int i = 0; i < BYTES_PER_LINE; i++) {
				if ((i & 7) == 0) {
					ptr_hd += sprintf(ptr_hd, " ");
				}
				if (i < bytes_cur_line) {
					ptr_hd += sprintf(ptr_hd, " %02x", ptr_line[i]);
				}
				else {
					ptr_hd += sprintf(ptr_hd, "   ");
				}
			}
			ptr_hd += sprintf(ptr_hd, "  |");
			for (int i = 0; i < bytes_cur_line; i++) {
				if (isprint((int)ptr_line[i])) {
					ptr_hd += sprintf(ptr_hd, "%c", ptr_line[i]);
				}
				else {
					ptr_hd += sprintf(ptr_hd, ".");
				}
			}
			ptr_hd += sprintf(ptr_hd, "|");

			log_text(LOG_LVL_DEBUG, tag, "%s: %s", buffer_name, hd_buffer);
			buffer += bytes_cur_line;
			len -= bytes_cur_line;
		} while (len);
	}
}

void log_course(const void* course) {
#if ENABLE_DEBUG_LOGS
	static const char* TAG = "course";
	const course_t* ptr = (const course_t*)course;
    LOG_DBG("%s %s %s %s %d", ptr->course_code, ptr->course_name, ptr->professor, ptr->days, ptr->credits);
#endif // ENABLE_DEBUG_LOGS
}

void log_courses(const void* head)  {
#if ENABLE_DEBUG_LOGS
	const course_t* ptr = (const course_t*)head;
    while (ptr != NULL) {
        courses_print(ptr);
        ptr = ptr->next;
    }
#endif // ENABLE_DEBUG_LOGS
}

void log_credential(const void* credentials) {
#if ENABLE_DEBUG_LOGS
	static const char* TAG = "credential";
	const credentials_t* ptr = (const credentials_t*)credentials;
	LOG_DBG("%s %s", ptr->username, ptr->password);
#endif // ENABLE_DEBUG_LOGS
}

void log_credentials(const void* head) {
#if ENABLE_DEBUG_LOGS
	const credentials_t* ptr = (const credentials_t*) head;
	while (ptr != NULL) {
		log_credential(ptr);
		ptr = ptr->next;
	}
#endif // ENABLE_DEBUG_LOGS
}
