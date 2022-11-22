#include "constants.h"
#include "serverSub.h"
#include "utils.h"

// EE450,Days

int main() {
    const char* db_file = "ee.txt";
    const char* subjectCode = DEPARTMENT_PREFIX_EE;
    const uint16_t port = SERVER_EE_UDP_PORT_NUMBER;
    return subjectServerMain(subjectCode, port, db_file);
}
