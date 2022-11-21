#include "constants.h"
#include "serverSub.h"
#include "utils.h"

// CS356,Days

int main() {
    const char* db_file = "cs.txt";
    const char* subjectCode = "CS";
    const uint16_t port = SERVER_CS_UDP_PORT_NUMBER;
    return subjectServerMain(subjectCode, port, db_file);
}
