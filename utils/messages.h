#ifndef MESSAGES_H
#define MESSAGES_H

#define CLIENT_MESSAGE_ON_BOOTUP "The client is up and running."
#define CLIENT_MESSAGE_INPUT_USERNAME "Please enter the username: "
#define CLIENT_MESSAGE_INPUT_PASSWORD "Please enter the password: "
#define CLIENT_MESSAGE_ON_AUTH_REQUEST "%.*s sent an authentication request to the main server."
#define CLIENT_MESSAGE_ON_AUTH_RESULT "%.*s received the result of authentication using TCP over port %d."
#define CLIENT_MESSAGE_ON_AUTH_RESULT_SUCCESS "Authentication is successful."
#define CLIENT_MESSAGE_ON_AUTH_RESULT_FAILURE "Authentication failed."

#define SERVER_C_MESSAGE_ON_BOOTUP "The serverC is up and running using UDP on port %d."
#define SERVER_C_MESSAGE_ON_BOOTUP_FAILURE "The serverC failed to start using UDP. Reason: %s."
#define SERVER_C_MESSAGE_ON_AUTH_REQUEST_RECEIVED "The serverC received an authentication request from the Main Server."
#define SERVER_C_MESSAGE_ON_AUTH_RESPONSE_SENT "The serverC finished sending the response to the Main Server."
#define SERVER_C_MESSAGE_ON_INVALID_REQUEST_RECEIVED "The serverC received an invalid request from the Main Server."

#define SERVER_SUB_MESSAGE_ON_BOOTUP "The server%s is up and running using UDP on port %d."
#define SERVER_SUB_MESSAGE_ON_BOOTUP_FAILED "The server%s failed to start with UDP. Reason: %s."
#define SERVER_SUB_MESSAGE_ON_LOOKUP_REQUEST_RECEIVED "The server%s received a request from the Main Server about the %s of %s."
#define SERVER_SUB_MESSAGE_ON_SUMMARY_REQUEST_RECEIVED "The server%s received a request from the Main Server for all the details of %s."
#define SERVER_SUB_MESSAGE_ON_COURSE_FOUND "The course information has been found: The %s of the %s is %s."
#define SERVER_SUB_MESSAGE_ON_COURSE_NOT_FOUND "Didn't find the course: %s."
#define SERVER_SUB_MESSAGE_ON_RESPONSE_SENT "The server%s finished sending the response to the Main Server."
#define SERVER_SUB_MESSAGE_ON_REQUEST_INVALID "The server%s received an invalid request from the Main Server."

#define SERVER_M_MESSAGE_ON_BOOTUP "The main server is up and running."
#define SERVER_M_MESSAGE_ON_BOOTUP_FAILURE "The main server failed to start. Reason: %s."
#define SERVER_M_MESSAGE_ON_AUTH_REQUEST_RECEIVED "The main server received the authentication for \"%s\" using TCP over port %d."
#define SERVER_M_MESSAGE_ON_AUTH_REQUEST_FORWARDED "The main server sent an authentication request to serverC."
#define SERVER_M_MESSAGE_ON_AUTH_RESULT_RECEIVED "The main server received the result of the authentication request from ServerC using UDP over port %d."
#define SERVER_M_MESSAGE_ON_AUTH_RESULT_FORWARDED "The main server sent the authentication result to the client."
#define SERVER_M_MESSAGE_ON_QUERY_RECEIVED "The main server received from %s to query course %s about %s using TCP over port %d."
#define SERVER_M_MESSAGE_ON_QUERY_FORWARDED "The main server sent a request to server%s."
#define SERVER_M_MESSAGE_ON_RESULT_RECEIVED "The main server received the response from server%s using UDP over port %d."
#define SERVER_M_MESSAGE_ON_RESULT_FORWARDED "The main server sent the query information to the client."

#endif // MESSAGES_H
