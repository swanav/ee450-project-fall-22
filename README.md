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



-----
***The format of all the messages exchanged.***



-----
***Any idiosyncrasy of your project. It should say under what conditions the project fails, if any.***

1. This code does not handle the possibility of multiple clients to serverM properly. Although, serverM will be able to accept multiple connections, it will be able to exchange messages with the most recent client at a time. 

-----
***Reused Code: Did you use code from anywhere for your project? If not, say so. If so, say what functions and where they're from. (Also identify this with a comment in the
source code.)***

The code takes inspirations from the socket programming guide at [beej.us](https://beej.us/guide/bgnet/html/). Also some of the utility functions for string manipulation and csv handling are implemented using reference from Stack Overflow.

-----
