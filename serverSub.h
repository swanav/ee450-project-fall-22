#ifndef SERVERSUB_H
#define SERVERSUB_H

#include <stdint.h>

#include "courses.h"

int subjectServerMain(const char* subjectCode, const uint16_t port, const char* db_file, db_entry_t* db);

#endif // SERVERSUB_H
