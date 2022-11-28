#ifndef DATABASE_H
#define DATABASE_H

#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "networking.h"
#include "protocol.h"

#if defined(SERVER_CS) || defined(SERVER_EE)
/**
 * @brief Find a course from a linked list of courses using course_code.
 * 
 * @param db Linked list of courses.
 * @param course_code Course code to search for.
 * 
 * @return Pointer to the course if found, NULL otherwise.
 */
course_t* database_courses_lookup(const course_t* db, const char* course_code);

/**
 * @brief Lookup a course's information.
 * 
 * @param course The course to lookup.
 * @param category The category to lookup.
 * @param info_buf The buffer to store the information in.
 * @param info_buf_size The size of the buffer.
 * @param info_len The length of the information.
 * 
 * @return err_t The error code.
 */
err_t database_courses_lookup_info(const course_t* course, courses_lookup_category_t category, uint8_t* info_buf, size_t info_buf_size, size_t* info_len);
#endif // SERVER_CS || SERVER_EE

#if defined(CLIENT)
/**
 * @brief Converts a string to a courses_lookup_category_t
 * 
 * @param category The string to convert
 * 
 * @return The converted courses_lookup_category_t
 */
courses_lookup_category_t database_courses_lookup_category_from_string(const char* category);
#endif // CLIENT

#if defined(CLIENT) || defined(SERVER_M) || defined(SERVER_CS) || defined(SERVER_EE)
/**
 * @brief Converts a courses_lookup_category_t to a string
 * 
 * @param category The courses_lookup_category_t to convert
 * 
 * @return The category string
 */
char* database_courses_category_string_from_enum(courses_lookup_category_t category);
#endif // CLIENT || SERVER_M || SERVER_CS || SERVER_EE

#if SERVER_M
/**
 * @brief Encrypt the given credentials according to the given method
 * 
 * @param in_credentials The credentials in plain text
 * @param out_credentials The encrypted credentials
 * @return err_t 
 */
err_t database_credentials_encrypt(const credentials_t* in_credentials, credentials_t* out_credentials);

#endif // SERVER_M

#ifdef SERVER_C
/**
 * @brief Validate the credentials from the given credentials linked list
 * 
 * @param credentials_db The credentials linked list
 * @param credential The credentials to validate
 * @return err_t 
 */
err_t database_credentials_validate(const credentials_t* credentials_db, const credentials_t* credential);

#endif // SERVER_C

#endif // DATABASE_H
