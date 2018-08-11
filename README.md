# UDP-client-server
A GO server that relays UDP packets between ports with C clients


Server:
The server relays UDP packets between ports.

The server listens for clients on two ports ­ port Pin and port Pout. On port Pin it waits for a single client type A, and once connected it starts sending any packet received from that client to ALL clients type B connected on port Pout.

Client type A:
Sends a 1K byte every 100ms to port Pin

Client type B:
Connects to the server on port Pout by sending it the command:

CONNECT\r\n

and disconnects from the server by sending it the command:

DISCONNECT\r\n

Client B also measures the amount of data received per second.

You should be able to run a server, ONE client A and as many clients B as possible until you see that transmission rate drops on clients B.

The server is written in Go, the clients are written C language (linux)
