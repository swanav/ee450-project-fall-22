# EE450 Socket Programming Project, Fall 2022

**Name:** Swanav Swaroop

**USC ID:** `1349-7540-53`

----
***What you have done in the assignment, if you have completed the optional part
(suffix). If it’s not mentioned, it will not be considered.***

The assignment contains all the features described in the assignment description, including the optional part of the assignment.
1. Implemented a backend server, `serverC` which receives authentication requests over **UDP** from the frontend server, serverM and responds with the appropriate authentication status after matching the credentials from a encrypted csv database, `cred.txt`.
2. Implemented backend servers, `serverCS` and `serverEE`, which receives course lookup requests over **UDP** from the frontend server, serverM and responds with the appropriate course data from a csv database, `cs.txt` and `ee.txt`.
3. Implemented a frontend server, `serverM` which talks to the backend servers mentioned above over **UDP** and interfaces with the client application over **TCP**. Handles authentication and course information lookup requests. It also aggregates course lookup requests for multiple requests to respond to a bulk query.
4. Implements a `client`, which talks to the frontend server, `serverM` and provides the user an interface to authenticate themselves and request information regarding courses.
-----
***What your code files are and what each one of them does. (Please do not repeat the project description, just name your code files and briefly mention what they do).***

- `client.c`
    - This module contains the code for the client application. It provides the user an interface to authenticate themselves and request information regarding courses.
- `constants.h`
    - This module contains the constants used in the project.
- `database.c`
- `database.h`
    - This module contains the functions to validate the credentials for a user. It also contains the functions to lookup course information.
- `department_server.c`
- `department_server.h`
    - This module contains functionality common to both `serverCS` and `serverEE`. This makes the code simpler to read and removes redundant code.
- `error.h`
    - This module contains the error codes used across the codebase.
- `fileio.c`
- `fileio.h`
    - This module contains the functions to read the csv files and store the data in a data structure.
- `log.c`
- `log.h`
    - This module contains the functions to log the events in the codebase.
- `messages.h`
    - This module contains the message formats used in the project according to the project description.
- `networking.c`
- `networking.h`
    - This module contains the networking functionality, specifically the TCP Server, Client and the UDP Server. It also contains the functionality to send and receive messages over the network. This makes the code simpler to read and removes redundant code.
- `protocol.c`
- `protocol.h`
    - This module contains the functions to parse the messages received over the network and to create the messages to be sent over the network.
- `serverC.c`
    - The main module containing `serverC` functionality.
- `serverCS.c`
    - The main module containing `serverCS` functionality. It initialises the department server module with the appropriate functions.
- `serverEE.c`
    - The main module containing `serverEE` functionality. It initialises the department server module with the appropriate functions.
- `serverM.c`
    - The main module containing `serverM` functionality.
- `utils.c`
- `utils.h`
    - Contains common string manipulation and math utilities used across different programs.

-----
***The format of all the messages exchanged.***

The `protocol.c` contains all the message formats and the functions to serialize and deserialize them.

-----
***Any idiosyncrasy of your project. It should say under what conditions the project fails, if any.***

1. This code does not handle the possibility of multiple clients to serverM properly. Although, serverM will be able to accept multiple connections, it will be able to exchange messages with the most recent client at a time. 

-----
***Reused Code: Did you use code from anywhere for your project? If not, say so. If so, say what functions and where they're from. (Also identify this with a comment in the
source code.)***

The code takes inspirations from the socket programming guide at [beej.us](https://beej.us/guide/bgnet/html/). Also some of the utility functions for string manipulation and csv handling are implemented using reference from Stack Overflow.

-----
