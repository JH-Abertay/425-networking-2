CMP425 / CMP501
Lab 2 TCP/UDP multi-client server example - by Andrei Boiko
Built using SFML Sockets

### Server

This server can communicate with multiple clients using either TCP or UDP.

TCP version waits for connections of MAX_TCP_CLIENTS. When all connections are made,
it asks for names and saves them. The server then waits for input before disconnecting
all clients and exiting.

UDP version recieves messages from "clients" and tracks them by IP/Port combination.
If more than MAX_UDP_CLIENTS attempt to talk to the server, it will respond, saying it is busy.

### Client

This client can communicate with the server using either TCP or UDP.

TCP version connects to the server, waits for others to connect,
and then provides a name to the server.

UDP version sends introduction message to server and gets a reply.

### How to Run

First run `cmake -S . -B build -G Ninja`.

Then run ninja in build directory.
