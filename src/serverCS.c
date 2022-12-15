#include "constants.h"
#include "department_server.h"

int main() {
    // Create a new department server for the CS department
    return department_server_main(DEPARTMENT_PREFIX_CS, SERVER_CS_UDP_PORT_NUMBER, DEPARTMENT_DB_FILE_CS);
}
