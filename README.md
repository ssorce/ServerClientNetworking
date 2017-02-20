# ServerClientNetworking
CS425
Names: Johnathon Davis and Scott Sorce

Deadlines

Milestone 1: Monday February 20, at 5:00pm.    15% of total credit
Milestone 2: Monday March 20, at 5:00pm.         25% of total credit
Milestone 3: Thursday April 20, at 5:00pm.         60% of total credit
 
 

Milestone 1: Implement a simple TCP client-server application.

Write a TCP client and a server passing messages between them. The message format is as follows.


The length field is a 32-bit integer telling how many bytes the payload contains.
Your programs (client and server) should implement the following actions.

User starts the server program on ServerVM 

./server sport

where sport is the port to which the server program will bind. It should use address INADDR_ANY.

User starts the client program on ClientVM

./client sip sport

where sip and sport is the server IP (eth1) and port number. The client establishes a TCP connection with the server.
User types a line of text on client’s terminal and hits “return”. The client program should read this line of text from stdin, using function calls like getline( ), fgetc( ), or others of your choosing. Note that the last newline character should not be counted. You should not transmit the newline or null character; only transmit the text of user input. The maximum text length is 1024 bytes.
The client program makes a message that contains the one line of text as payload and the length of the payload. Sends it to the server program through the TCP connectivity.
The server receives the message, print two lines to the stdout, the 1st line is the length of the payload, the 2nd line is the payload text. Note that the server doesn’t know how many bytes to read from the byte-stream supplied by TCP; so the server should read the length first, which is a 4-byte number, then read that number of bytes as the payload. 
The client should keep reading from stdin line by line, sends one line at a time, until it receives EOF (i.e., ctrl-d on the command line), at which point it should close the connection and exit. The server should exit after the connection is closed by the client (check the return value of send/recv).

Deliverables: 
Source files client.c and server.c, and Makefile that will produce binaries “client” and “server”.

Milestone 2: Implement TCP proxy programs

Get familiar with telnet, which will be used to test your programs. Run telnet on ClientVM to log into ServerVM: 

telnet sip

where sip is ServerVM’s eth1 IP address. After successful login, you should get a regular Unix shell session. 

For this milestone, you will implement a client-side proxy (“cproxy) and a server-side proxy (“sproxy) that will relay traffic between the telnet program and telnet daemon. The specific actions to implement are as follows.



Start sproxy on ServerVM. It should listen on TCP port 6200.

sproxy 6200

Start the cproxy on ClientVM to listen on TCP port 5200, and specify the sproxy:

cproxy 5200 sip 6200

On ClientVM, telnet to cproxy:

telnet localhost 5200

The above command will have telnet open a TCP connection with cproxy. After accepting the connection, cproxy should open a TCP connection to sproxy. Upon accepting this connection, sproxy should open a TCP connection to the telnet daemon on localhost (address 127.0.0.1, port 23). Now, as shown in the above figure, cproxy and sproxy connect telnet and telnet daemon via three TCP connections. 

In order for telnet to work, cproxy and sproxy should relay traffic received from one side to the other side. For example, cproxy has two connected sockets. It should wait for incoming data on both sockets. If either one receives some data, cproxy should read them from the socket, and send them to the other socket. Similarly, sproxy should operate the same way. Since both proxies merely relay data between their sockets, from user’s point of view, telnet should just work the same way as if it directly connects to the daemon, e.g., type a Unix command and get results displayed on the terminal. Your cproxy and sproxy should implement the relay functionality to achieve this goal. 

One technical question is, how do cproxy and sproxy wait for input from two sockets at the same time? You’ll need select( ) for this. See Select for how to use select( ).

Another required feature is the closing behavior. When the user closes the telnet session, e.g., typing “exit”, “logout”, or ctrl-d, cproxy should close both of its connections, and sproxy should do so too. If recv( ) returns 0 or -1, it means the socket has been closed by the peer or it has an error. In either case the socket should be closed.

To isolate problems when debugging the programs, you can run telnet---cproxy---daemon first to debug cproxy, then telnet---sproxy---daemon to debug sproxy, and finally put them together as telnet---cproxy---sproxy---daemon and test.

Deliverables: 
Source files cproxy.c and sproxy.c, and Makefile that will produce binaries “cproxy” and “sproxy”.

Milestone 3: Detecting and handling address changes

Let’s first establish a telnet session, change IP address, and see what happens.

Telnet from ClientVM directly to ServerVM, then start a ping command (on ServerVM):

ping localhost

This is a trivial way to generate some continuous traffic. It keeps displaying the result of each ping at one-second interval with increasing icmp_seq number. The command is executed on ServerVM, but the results are transmitted to and displayed on ClientVM via the telnet session. As long as the session is working, you should see the ping result keeps showing up and there should be no gap in icmp_seq, i.e., no data loss.

Open another terminal on ClientVM, remove the Client’s eth1 address, say, 1.2.3.4

sudo ip addr del 1.2.3.4/24 dev eth1

This removes the current IP address from the eth1 interface. You can use the following command to show the interface information before and after changes:

ip addr show eth1

Once the current address is removed, the ping results in the telnet session will stop immediately. Actually the ping program is still running on ServerVM, but because the TCP connection has been severed by the address removal, the results cannot be transferred to ClientVM. Similarly, no input from the ClientVM can be transferred to ServerVM, so the telnet session appears hanging. 

Add a new address to Client’s eth1, say 1.2.3.5:

sudo ip addr add 1.2.3.5/24 dev eth1

In our setup, the new address must have the same first 3 numbers as the old address. The last number of the new address must be chosen between 1 and 254, excluding the numbers already taken by the ServerVM and the old ClientVM addresses.

Because the existing telnet session was established using the old address, adding a new address doesn’t resume the session, which will still be hanging, not responding to any key stroke. That’s the problem we want to fix. 


Cleanup: to terminate the hanging telnet session, type “ctrl ]” then “ctrl d”. You should also kill the ping process at the ServerVM.

Our goal is to resume the telnet session after the new address has been added, and do so without data loss. The idea is to use your cproxy and sproxy, and have a protocol between these proxies to detect and handle address changes. With cproxy and sproxy, the telnet session consists of three TCP connections. When the ClientVM’s address changes, it only breaks the connection between cproxy and sproxy; the other two connections are within a host and use address 127.0.0.1 (meaning localhost), which doesn’t change no matter how a host moves. Therefore, the concern is only with the cproxy---sproxy connection.

In order to detect the address removal, cproxy and sproxy send each other a heartbeat message every second. Both proxy programs treat the missing of three consecutive heartbeats as the indication of connection loss. They should close the disconnected sockets. Use select( ) to implement the one-second timer.

cproxy should try connecting to sproxy again. When the new address is added, such a connection request will go through and the connection between cproxy and sproxy is reestablished using new sockets. Application data should now flow again. In our example, ping results will show up again on ClientVM’s terminal. 

To ensure the new address is available before calling connect( ), you can put the address removal command and address add command in one line separated by a semicolon (;) and run them back to back quickly.

From sproxy’s point of view, when a new connection with cproxy is established, there are two possibilities: (i) it is a brand new telnet session, or (ii) it is the continuation of a previous session after the ClientVM has got a new address. In the former case, sproxy should close the existing connection with telnet daemon, and open a new connection with the daemon to have a new telnet session. In the latter case, sproxy should keep using the existing connection to the daemon. To differentiate these two cases, cproxy needs to send a special message to sproxy right after they are connected, telling sproxy which case it is, and sproxy handles it accordingly.

There should be no data loss, i.e., no gap in icmp_seq, during the entire process. Data that were sent after the address removal but before the socket closure are not delivered. Therefore, cproxy and sproxy need a mechanism similar to TCP’s sequence number and acknowledge number to keep track of who has received what, and also buffer data that have been sent but have not been acknowledged. After connection is established, they first exchange each other’s acknowledgement number and retransmit data if needed.

To address all these issues, you need to define a packet format for messages between cproxy and sproxy. There are three types of messages: heartbeat, session initiation, and application data. Every message should have a header and a payload. The header may identify the type of the message, the length of the payload, the sequence and acknowledgement numbers, and any other information you think useful. 

Based on the message format, you need to define the protocol actions between cproxy and sproxy, i.e., how each packet is being processed, and implement them.

A successful project should make a telnet session survive host mobility (i.e., address change) using your proxy programs. Optionally you may test it with other applications such as ssh.

Deliverables

A project report that documents the design of the packet format and protocol actions. 
Source files cproxy.c and sproxy.c, and Makefile that can produce binaries called “cproxy” and “sproxy”.
