#ifndef CREDENTIALS_H
#define CREDENTIALS_H

#include <stdint.h>

#include "error.h"

#include "protocol.h"

/**
 * @brief Encrypt the given credentials according to the given method
 * 
 * @param in_credentials The credentials in plain text
 * @param out_credentials The encrypted credentials
 * @return err_t 
 */
err_t credentials_encrypt(const credentials_t* in_credentials, credentials_t* out_credentials);

/**
 * @brief Validate the credentials from the given credentials linked list
 * 
 * @param credentials_db The credentials linked list
 * @param credential The credentials to validate
 * @return err_t 
 */
err_t credentials_validate(const credentials_t* credentials_db, const credentials_t* credential);

#endif //CREDENTIALS_H
