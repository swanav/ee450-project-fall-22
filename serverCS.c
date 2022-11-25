#include "constants.h"
#include "serverSub.h"

int main() {
    return subjectServerMain(DEPARTMENT_PREFIX_CS, SERVER_CS_UDP_PORT_NUMBER, "cs.txt");
}
