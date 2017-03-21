/*
* Start the cproxy on ClientVM to listen on TCP port 5200, and specify the sproxy:
* cproxy 5200 sip 6200
* On ClientVM, telnet to cproxy:
* telnet localhost 5200
*/

#include "PortableSocket.h"
#include <sys/select.h>
#define size 1024

int main(int argc, char *argv[])
{
  if (argc != 4)
    return 1;

  /*
    * Parse the inputs
    */
  int clientPort = atoi(argv[1]);
  char *serverAddress = argv[2];
  int serverPort = atoi(argv[3]);

  /*
    * Open the network
    */
  cpOpenNetwork();

  /*
    * Connection to the local telnet
    */
  struct PortableSocket *telnetAcceptorSocket = cpSocket(TCP, "127.0.0.1", clientPort);
  cpBind(telnetAcceptorSocket);
  cpListen(telnetAcceptorSocket, 1);
  struct PortableSocket *telnetSocket = cpAccept(telnetAcceptorSocket);

  /*
    * Create connection to sproxy
    */
  struct PortableSocket *sproxySocket = cpSocket(TCP, serverAddress, serverPort);
  cpConnect(sproxySocket);

  /*
    * Foward data from one port to another
    */
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(telnetSocket->socket, &readfds);
  FD_SET(sproxySocket->socket, &readfds);
  int n = sproxySocket->socket + 1;
  char message[size];
  
  while (cpCheckError(telnetSocket) == 0 && cpCheckError(sproxySocket) == 0)
  {
    if (select(n, &readfds, NULL, NULL, NULL) <= 0)
      break;
    // foward the message
    if (FD_ISSET(telnetSocket->socket, &readfds))
    {
      // print "recieved from telnet 'message' sending to cproxy"
      cpRecv(telnetSocket, message, size);
      printf("Recieved from telnet (client): '%s'\n", message);
      cpSend(sproxySocket, message, size);
    }
    if (FD_ISSET(sproxySocket->socket, &readfds))
    {
      // print "sending from telnet 'message' sending to cproxy"
      cpRecv(sproxySocket, message, size);
      printf("serverProxy from telnet: '%s'\n", message);
      cpSend(telnetSocket, message, size);
    }
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