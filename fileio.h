#ifndef FILEIO_H
#define FILEIO_H

#include "protocol.h"

/**
 * @brief Create the credentials db from the given file
 * 
 * @param filename The file to parse
 * @return credentials_t* The parsed credentials linked list
 */
credentials_t* fileio_credential_server_db_create(const char* filename);

/**
 * @brief Free the given credentials linked list
 * 
 * @param credentials The credentials linked list to free
 */
void fileio_credential_server_db_free(credentials_t* credentials);

/**
 * @brief Create the courses db from the given file
 * 
 * @param filename The file to parse
 * @return course_t* The parsed credentials linked list
 */
course_t* fileio_department_server_db_create(const char* filename);

/**
 * @brief Free the given credentials linked list
 * 
 * @param courses The credentials linked list to free
 */
void fileio_department_server_db_free(course_t* courses);

#endif // FILEIO_H
