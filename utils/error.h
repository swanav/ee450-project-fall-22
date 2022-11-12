#ifndef ERROR_H
#define ERROR_H

#include <stdint.h>

typedef uint8_t err_t;

#define ERR_OK                              0x00
#define ERR_INVALID_PARAMETERS              0x01

#define ERR_REQ_BASE                        0x10
#define ERR_REQ_INVALID                     (ERR_REQ_BASE | ERR_INVALID_PARAMETERS)

#define ERR_RESP_BASE                       0x20
#define ERR_RESP_INVALID                    (ERR_RESP_BASE | ERR_INVALID_PARAMETERS)

#define ERR_CREDENTIALS_BASE                0x30
#define ERR_CREDENTIALS_OK                  (ERR_CREDENTIALS_BASE | 0x01)
#define ERR_CREDENTIALS_USER_NOT_FOUND      (ERR_CREDENTIALS_BASE | 0x02)
#define ERR_CREDENTIALS_PASSWORD_MISMATCH   (ERR_CREDENTIALS_BASE | 0x03)
#define ERR_CREDENTIALS_INVALID_REQUEST     (ERR_CREDENTIALS_BASE | 0x04)
#define ERR_CREDENTIALS_INVALID_PARAMETERS  (ERR_CREDENTIALS_BASE | 0x05)

#define ERR_COURSES_BASE                    0x40
#define ERR_COURSES_OK                      (ERR_COURSES_BASE | 0x01)
#define ERR_COURSES_NOT_FOUND               (ERR_COURSES_BASE | 0x02)
// #define ERR_COURSES_PASSWORD_MISMATCH       (ERR_COURSES_BASE | 0x03)
#define ERR_COURSES_INVALID_REQUEST         (ERR_COURSES_BASE | 0x04)
#define ERR_COURSES_INVALID_PARAMETERS      (ERR_COURSES_BASE | 0x05)


#endif // ERROR_H
