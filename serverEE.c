#include "serverSub.h"
#include "utils.h"

int main() {
    const char* db_file = "ee.txt";
    db_entry_t* db = NULL;
    const char* subjectCode = "ee";
    const int16_t port = SERVER_EE_UDP_PORT_NUMBER;
    return subjectServerMain(subjectCode, port, db_file, db);
}
