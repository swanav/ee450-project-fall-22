#ifndef CREDENTIALS_H
#define CREDENTIALS_H

#include <stdint.h>

#include "../utils/error.h"

#include "../protocol.h"

err_t credentials_encode(const credentials_t* in_credentials, uint8_t* out_buffer, const size_t out_buffer_size, uint8_t* out_buffer_len);
err_t credentials_decode(credentials_t* out_credentials, const uint8_t* in_buffer, const uint8_t in_buffer_len);

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

/**
 * @brief Encode a credentials request
 * 
 * @param credentials [in] Credentials to encode
 * @param out_dgrm [out] Datagram to encode into
 * @return err_t 
 */
err_t credentials_request_encode(const credentials_t* credentials, udp_dgram_t* out_dgrm);

/**
 * @brief Decode a credentials request
 * 
 * @param in_dgrm [in] Datagram to decode from
 * @param credentials [out] Credentials to decode into
 * @return err_t 
 */
err_t credentials_request_decode(const udp_dgram_t* in_dgrm, credentials_t* credentials);

/**
 * @brief Encode a credentials response
 * 
 * @param authentication_result [in] The authentication result
 * @param out_dgrm [out] The encoded datagram
 * @return err_t 
 */
err_t credentials_response_encode(const uint8_t authentication_result, udp_dgram_t* out_dgrm);

/**
 * @brief Decodes a credentials response
 * 
 * @param in_dgrm [in] The datagram to decode
 * @param authentication_result [out] The authentication result
 * @return err_t 
 */
err_t credentials_response_decode(const udp_dgram_t* in_dgrm, uint8_t* authentication_result);

#endif //CREDENTIALS_H
