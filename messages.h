#ifndef MESSAGES_H
#define MESSAGES_H

#define CLIENT_MESSAGE_ON_BOOTUP "The client is up and running."
#define CLIENT_MESSAGE_INPUT_USERNAME "Please enter the username: "
#define CLIENT_MESSAGE_INPUT_PASSWORD "Please enter the password: "
#define CLIENT_MESSAGE_ON_AUTH_REQUEST "%s sent an authentication request to the main server."
#define CLIENT_MESSAGE_ON_AUTH_RESULT_SUCCESS "%s received the result of authentication using TCP over port %d."
#define CLIENT_MESSAGE_ON_AUTH_RESULT_FAILURE "%s received the result of authentication using TCP over port %d."

#define SERVER_C_MESSAGE_ON_BOOTUP "The ServerC is up and running using UDP on port %d."
#define SERVER_C_MESSAGE_ON_AUTH_REQUEST_RECEIVED "The ServerC received an authentication request from the Main Server."
#define SERVER_C_MESSAGE_ON_AUTH_RESPONSE_SENT "The ServerC finished sending the response to the Main Server."

#define SERVER__MESSAGE_ON_BOOTUP(branch) "The Server"#branch" is up and running using UDP on port %d."
#define SERVER__MESSAGE_ON_REQUEST_RECEIVED(branch) "The Server"#branch" received a request from the Main Server about the %s of %s."
#define SERVER__MESSAGE_ON_COURSE_FOUND(branch) "The course information has been found: The %s of the %s is %s."
#define SERVER__MESSAGE_ON_COURSE_NOT_FOUND(branch) "Didn't find the course: %s."
#define SERVER__MESSAGE_ON_RESPONSE_SENT(branch) "The Server"#branch" finished sending the response to the Main Server."

#define SERVER_M_MESSAGE_ON_BOOTUP "The main server is up and running."

#endif // MESSAGES_H