#ifndef ERROR_H
#define ERROR_H

#include <stdint.h>

typedef uint8_t err_t;

#define ERR_OK                              0x00
#define ERR_INVALID_PARAMETERS              0x01
#define ERR_OUT_OF_MEMORY                   0x02

#define ERR_REQ_BASE                        0x10
#define ERR_REQ_INVALID                     (ERR_REQ_BASE | ERR_INVALID_PARAMETERS)

#define ERR_RESP_BASE                       0x20
#define ERR_RESP_INVALID                    (ERR_RESP_BASE | ERR_INVALID_PARAMETERS)

#define ERR_CREDENTIALS_BASE                0x30
#define ERR_CREDENTIALS_USER_NOT_FOUND      (ERR_CREDENTIALS_BASE | 0x02)
#define ERR_CREDENTIALS_PASSWORD_MISMATCH   (ERR_CREDENTIALS_BASE | 0x03)

#define ERR_COURSES_BASE                    0x40
#define ERR_COURSES_NOT_FOUND               (ERR_COURSES_BASE | 0x02)

#endif // ERROR_H
