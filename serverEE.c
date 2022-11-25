#include "constants.h"
#include "serverSub.h"

int main() {
    return subjectServerMain(DEPARTMENT_PREFIX_EE, SERVER_EE_UDP_PORT_NUMBER, "ee.txt");
}
