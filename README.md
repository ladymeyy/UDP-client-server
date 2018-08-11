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

How to run:
============ 

Server: 
-------
1. Make sure you have golang installed on your computer 
you can download it from here: https://golang.org/dl/
2. run this command at the same directory as the server file:   go run server.go
3. ports:  
   - clientA: port_in = ":5001"
   - clientB: port_out = ":7777"

clientA: (linux distro) 
--------
gcc clientA.c -o clientA
Usage: ./clientA <server name> <port number> <rate (Mb/s)>
  
clientB: (linux distro) 
---------
gcc clientB.c -o clientB -lpthread
Usage: ./clientB <server name> <port number>
example: ./clientB 192.168.1.6 7777

  

Further notes: 
===============
* The first client connected on Pin is the chosen client Type A - Any attempt to connect to Pin after the first one is set - will be denied. 
* It is assumed that each client will always connect to it's matching port in the server. There are many ways pto add a security check for each connection in order to identify the type of the client. (checksum for example).
