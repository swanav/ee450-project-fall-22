# EE450 Socket Programming Project, Fall 2022

**Name:** Swanav Swaroop

**USC ID:** `1349-7540-53`

----
***What you have done in the assignment, if you have completed the optional part (suffix). If it’s not mentioned, it will not be considered.***

> **The assignment contains all the features described in the assignment description, including the `optional` part of the assignment.**

1. Implemented a backend server, `serverC` which receives authentication requests over **UDP** from the frontend server, serverM and responds with the appropriate authentication status after matching the credentials from a encrypted csv database, `cred.txt`.
2. Implemented backend servers, `serverCS` and `serverEE`, which receives course lookup requests over **UDP** from the frontend server, serverM and responds with the appropriate course data from a csv database, `cs.txt` and `ee.txt`.
3. Implemented a frontend server, `serverM` which talks to the backend servers mentioned above over **UDP** and interfaces with the client application over **TCP**. Handles authentication and course information lookup requests. It also aggregates course lookup requests for multiple requests to respond to a bulk query.
4. Implements a `client`, which talks to the frontend server, `serverM` and provides the user an interface to authenticate themselves and request information regarding courses.

> This implementation contains extensive use of function pointers and macros to reduce code duplication and increase readability. Socket functionalities (TCP and UDP) have been carefully encapsulated in separate `networking.[ch]` files. The servers and the client reuse the same codebase to communicate over TCP and UDP. Please note that in accordance to the project guidelines, it is ***only the code that is being reused***. The applications themselves do not share any runtime and operate in ***complete isolation***. Where possible, the code employs macro guards to make it easy to understand which functions are accessible to which applications.

> Similarly, since the `serverCS` and `serverEE` provide the same functionality to the user. `department_server.[ch]` implements the core functionality and both the department servers are implemented as wrappers around it. This allows for easy addition of new departments in the future without code duplication.

> I had started this implementation to serve multiple clients at the same time, but didn't after clarifications received on class forum and time constraints. The codebase is structured to support this feature in the future with minimal changes.

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

All the exchanged messages are in the format:

```
| < -------------- Protocol Header -------------- > | < Payload > |
|     Type     |     Flags     | Message Length (N) |   Message   |
| <  1 byte  > | <  1 byte   > | <     2 bytes    > | < N bytes > |
```

`Type` can be any of the following depending on the transaction.

- `0x61 - REQUEST_TYPE_AUTH`
- `0x62 - REQUEST_TYPE_COURSES_SINGLE_LOOKUP`
- `0x63 - REQUEST_TYPE_COURSES_MULTI_LOOKUP`
- `0x64 - REQUEST_TYPE_COURSES_DETAIL_LOOKUP`
- `0x71 - RESPONSE_TYPE_AUTH`
- `0x72 - RESPONSE_TYPE_COURSES_SINGLE_LOOKUP`
- `0x73 - RESPONSE_TYPE_COURSES_MULTI_LOOKUP`
- `0x74 - RESPONSE_TYPE_COURSES_DETAIL_LOOKUP`
- `0x75 - RESPONSE_TYPE_COURSES_ERROR`

`Flags` contains the flags for the message. It varies depending on the transaction.

`Length` contains the length of the message. `16 bits` allows it support messages with lengths upto 65536. The payload however cannot exceed `1024 - 4 = 1020` bytes (A limit that can be changed in `constants.h`).

`Message` contains the payload of the message.

---

***Authentication Request***

```
| Protocol Header | Username Len (X) | Password Len (Y) |   Username  |  Password   |
| <   4 bytes   > | <    1 byte    > | <    1 byte    > | < X bytes > | < Y bytes > |
```

`Type = REQUEST_TYPE_AUTH (0x61)`

`Flags = 0x00`

`Length = 2 + X + Y`

`Username Len (X)` contains the length of the username.

`Password Len (Y)` contains the length of the password.

`Username` contains the username.

`Password` contains the password.

---

***Authentication Response***
```
| Protocol Header |
| <   4 bytes   > |
```

`Type = RESPONSE_TYPE_AUTH (0x71)`

`Flags`
```
| X X S F X X U P |

S - SUCCESS
F - FAILURE 
U - USER_NOT_FOUND 
P - PASSWORD_MISMATCH
```

`Length = 0`

---

***Single Course Lookup Request***

```
| Protocol Header | Payload (X) |
| <   4 bytes   > | < X bytes > |
```

`Type = REQUEST_TYPE_COURSES_SINGLE_LOOKUP (0x62)`

`Flags`
```
0x50 - Course Code
0x51 - Credits
0x52 - Professor    
0x53 - Days
0x54 - Course Name    
0x55 - Invalid
```

`Length = X`

`Payload = Course Code (X Bytes)`

---

***Single Course Lookup Response***

```
| Protocol Header | Course Code Length (X) | Course Code | Information Length (Y) | Information |
| <   4 bytes   > | <       1 byte       > | < X bytes > | <       1 byte       > | < Y bytes > |
```
`Type = RESPONSE_TYPE_COURSES_SINGLE_LOOKUP (0x72)`

`Flags`
```
0x50 - Course Code
0x51 - Credits
0x52 - Professor    
0x53 - Days
0x54 - Course Name    
0x55 - Invalid
```
`Length = 2 + X + Y`

`Course Code Length (X)` contains the length of the course code.

`Course Code` contains the course code.

`Information Length (Y)` contains the length of the information.

`Information` contains the information.

---

***Course Detail Lookup Request***

```
| Protocol Header | Course Code |
| <   4 bytes   > | < X bytes > |
```

`Type = REQUEST_TYPE_COURSES_DETAIL_LOOKUP (0x64)`

`Flags = 0`

`Length = X`

`Course Code` contains the course code.

---

***Course Detail Lookup Response***

```
| Protocol Header | Course Code Len (A) | Course Code | Course Name Len (B) | Course Name | Professor Name Len (C) | Professor Name | Days Len (D) |    Days   | Credits Len (E) |   Credits   |
| <   4 bytes   > | <      1 byte     > | < A bytes > | <      1 byte     > | < B bytes > | <       1 byte       > | <   C bytes  > | <  1 byte  > |< D bytes >| <    1 byte   > | < E bytes > |
```

`Type = RESPONSE_TYPE_COURSES_DETAIL_LOOKUP (0x74)`

`Flags = 0`

`Length = 5 + A + B + C + D + E`

`Course Code Len (A)` contains the length of the course code.

`Course Code` contains the course code.

`Course Name Len (B)` contains the length of the course name.

`Course Name` contains the course name.

`Professor Name Len (C)` contains the length of the professor name.

`Professor Name` contains the professor name.

`Days Len (D)` contains the length of the days.

`Days` contains the days.

`Credits Len (E)` contains the length of the credits.

`Credits` contains the credits.

---

***Course Multi Lookup Request***

```
| Protocol Header | Course Count | Course1 Len (A) | Course1 Name | Course 2 Len (B) | Course 2 Name | ... | Course N Len (N) | Course N Name |
| <   4 bytes   > | <  1 byte  > | <    1 byte   > | <  A bytes > | <    1 byte    > | <  B bytes  > | ... | <    1 byte    > |<   N bytes   >|
```

`Type = REQUEST_TYPE_COURSES_MULTI_LOOKUP (0x63)`

`Flags = 0`

`Length = 1 + N + Sum(A, B, ..., N)`

`Course Count` contains the number of courses.

`Course1 Len (A)` contains the length of the first course.

`Course1 Name` contains the first course.

`Course 2 Len (B)` contains the length of the second course.

`Course 2 Name` contains the second course.

...

`Course N Len (N)` contains the length of the Nth course.

`Course N Name` contains the Nth course.

---

***Course Multi Lookup Response***

```
                  | <  ..  ..  ..  ..  ..  ..  ..  ..  ..  .. Repeating ..  ..  ..  ..  ..  ..  ..  ..  ..  ..  > |
| Protocol Header |  Course Details Len (A) | Field Len (A1) |  Field Value | ... | Field Len (An) |  Field Value | ...... | 
| <   4 bytes   > |  <       1 byte       > | <   1 byte   > | < A1 bytes > | ... | <   1 byte   > | < An bytes > | ...... | 
```

`Type = RESPONSE_TYPE_COURSES_MULTI_LOOKUP (0x73)`

`Flags = Course Count (X)`

`Length = Sum((1 + N + Sum(A1, A2, ..., An))...(1 + N + Sum(X1, X2, ..., Xn)))`

> ***Repeating block***
>
> `Course Details Len (A)` contains the length of the course details.
> 
> `Field Len (A1)` contains the length of the first field.
> 
> `Field Value` contains the first field.
> 
> ...
> 
> `Field Len (An)` contains the length of the nth field.
> 
> `Field Value` contains the nth field.
> 
> ***End repeating block***

``

---

***Course Lookup Error Response***

```
| Protocol Header |  Error Data  |
| <   4 bytes   > | < X bytes > |
```

`Type = RESPONSE_TYPE_COURSES_ERROR (0x75)`

`Flags = Error Code` (Listed in `error.h`)

`Length = X`

`Error Data` contains the error data.

-----
-----
***Any idiosyncrasy of your project. It should say under what conditions the project fails, if any.***

1. This code does not handle the possibility of multiple clients to serverM properly. Although, serverM will be able to accept multiple connections, it will be able to exchange messages with the most recent client at a time. 
2. This code also does not serve two queries at the same time. If a client sends a query, it will be served, and only then will a client be able to send another query.

-----
***Reused Code: Did you use code from anywhere for your project? If not, say so. If so, say what functions and where they're from. (Also identify this with a comment in the source code.)***

The code takes inspirations from the socket programming guide at [beej.us](https://beej.us/guide/bgnet/html/). Although, none of the snippets have been copied directly. 

Also some of the utility functions for string manipulation and csv handling are implemented after taking reference from Stack Overflow.

-----
