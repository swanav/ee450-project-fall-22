#include "serverSub.h"
#include "utils.h"

int main() {
    const char* db_file = "cs.txt";
    db_entry_t* db = NULL;
    const char* subjectCode = "cs";
    const int16_t port = SERVER_CS_UDP_PORT_NUMBER;
    return subjectServerMain(subjectCode, port, db_file, db);
}
