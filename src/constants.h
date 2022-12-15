#ifndef CONSTANTS_H
#define CONSTANTS_H

// USC Student ID: 1349754-053
#define USC_ID_LAST_3 53

// Host
#define LOCALHOST                                   INADDR_LOOPBACK // 32 bit representation of 127.0.0.1 (localhost) from the standard C library

// Ports
#define SERVER_C_UDP_PORT_NUMBER                    21000 + USC_ID_LAST_3
#define SERVER_CS_UDP_PORT_NUMBER                   22000 + USC_ID_LAST_3
#define SERVER_EE_UDP_PORT_NUMBER                   23000 + USC_ID_LAST_3
#define SERVER_M_UDP_PORT_NUMBER                    24000 + USC_ID_LAST_3
#define SERVER_M_TCP_PORT_NUMBER                    25000 + USC_ID_LAST_3
#define CLIENT_PORT_NUMBER                          // DYNAMICALLY ASSIGNED PORT NUMBER (Macro not used)

// Department code prefixes
#define DEPARTMENT_PREFIX_EE                        "EE"
#define DEPARTMENT_PREFIX_CS                        "CS"
#define DEPARTMENT_PREFIX_LEN                       2

// Credentials server database file
#define CREDENTIALS_FILE                            "cred.txt"
// Department server database files
#define DEPARTMENT_DB_FILE_CS                       "cs.txt"
#define DEPARTMENT_DB_FILE_EE                       "ee.txt"

#define AUTH_SUCCESS                                -1
#define AUTH_MAX_ATTEMPTS                           3

// Client Timeout for a response from the server
#define TCP_QUERY_TIMEOUT_DELAY_S                   2
#define TCP_QUERY_TIMEOUT_DELAY_NS                  0

#define COURSE_NAME_BUFFER_SIZE                     128
#define COURSE_CATEGORY_BUFFER_SIZE                 24

#define CREDENTIALS_MIN_USERNAME_LEN                 5
#define CREDENTIALS_MIN_PASSWORD_LEN                 5
#define CREDENTIALS_MAX_USERNAME_LEN                50
#define CREDENTIALS_MAX_PASSWORD_LEN                50

#define CREDENTIALS_ENC_CHAR_SHIFT_COUNT            4


#endif // CONSTANTS_H
