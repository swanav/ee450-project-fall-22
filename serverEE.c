#include "constants.h"
#include "department_server.h"

int main() {
    return department_server_main(
        DEPARTMENT_PREFIX_EE, 
        SERVER_EE_UDP_PORT_NUMBER, 
        DEPARTMENT_DB_FILE_EE
    );
}
