/*
* Start the cproxy on ClientVM to listen on TCP port 5200, and specify the sproxy:
* cproxy 5200 sip 6200
* On ClientVM, telnet to cproxy:
* telnet localhost 5200
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <PortableSocket.h>

int main(int argc, char *argv[]){
    if (argc != 4)
      return 1;

    /*
    * Parse the inputs
    */
    int clientPort = atoi(argv[1]);
    char* serverAddress = argv[2];
    int serverPort = atoi(argv[3]);

    /*
    * Open the network
    */
    cpOpenNetwork();

    /*
    * Connection to the local telnet
    */
  	struct PortableSocket* telnetAcceptorSocket = cpSocket(TCP, "127.0.0.1", clientPort);
    cpBind(socket);
    cpListen(socket,1);
    struct PortableSocket* telnetSocket = cpAccept(socket);

    /*
    * Create connection to sproxy
    */
    struct PortableSocket* sproxySocket = cpSocket(TCP,serverAddress,serverPort);
    cpConnect(socket);

    /*
    * Foward data from one port to another
    */
    while(cpCheckError(client) == 0) {
      //todo use select() to switch data from one socket to another
  	}

    /*
    * Close the connections
    */
    cpClose(telnetAcceptorSocket);
    cpClose(telnetSocket);
    cpClose(sproxySocket);
  	cpCloseNetwork();
  	return 0;
}
