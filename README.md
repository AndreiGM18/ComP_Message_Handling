**Name: Mitran Andrei-Gabriel**
**Group: 323CA**

## Homework #2 (Client-server TCP and UDP application for message handling)

### Organization:
* The objective of this project is to implement the server and the subscriber
programs. The server handles TCP and UDP clients, and the subscriber program is
a TCP client.

#### Custom structures
* A simple linked list data structure is implemented.
* There are structs for all packets: one for receiving by the server from a TCP
client, one for receiving by the TCP client from the server, and one for
receiving by the server from the UDP clients.
* A client structure is used to keep all relevant information
relating to clients, as they do not get erased when disconnecting. In it,
there are two lists, one for all topics that the client is subscribed to,
and one for all unsent messages that are relevant to the client.
* A topic structure is used to store both the name and its sf (store and
forward) parameter.
* A sockets structure is used to store relevant information relating to the
server sockets and addresses.

#### Server
* Two sockets are opened, and the TCP one is listening, awaiting connections
from TCP clients.
* Using a pollfd vector, stdin and the sockets are stored. Then poll is called
in a loop, which is broken when "exit" is received from stdin.
* If a packet from a client is received, the server will act according to
the type: if subscribing or unsubscribing, the client's list of topics is
updated and if exiting, the client is marked as offline and the fd is closed.
* If a connection from the TCP socket is received, it checks if the client is
new or already exists (and whether it is online or offline). Depending on the
scenario, the server adds a client to the clients' list, marks it as online
(updating the fd), or simply closes the connection as it is already established.
If the client was offline and is now online, all unsent messages that are
relevant are sent. It prints relevant information regarding the client that is
attempting to connect, regardless of the scenario.
* If a connection from the UDP socket is received, we break down the packet
and re-encapsulate it in a different form (udp_msg -> tcp_msg) and forward it
to all clients that are subscribed to the newly posted about topic. If they
are offline, and have the sf parameter marked as 1, the message is stored.
All content conversions are done here.

#### Subscriber
* A TCP socket is opened for connecting to the server.
* Using a pollfd vector, stdin and the socket are stored. Then poll is called
in a loop, which is broken when "exit" is received from stdin.
* If a valid command from stdin is received, a packet is created containing
relevant information relating to it, and is then forwarded to the server.
* If a message is received from the server, it is printed according to the
specified format in the homework description.

### Implementation:
* Every functionality required for this homework was implemented.

### Compilation:
* To compile, use:
```
make
```

### Resources:
* Everything provided by the ComP team
* [Linux Manual](https://www.man7.org/linux/man-pages/index.html)
