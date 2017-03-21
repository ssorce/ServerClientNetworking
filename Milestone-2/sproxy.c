/*
*  Start sproxy on ServerVM. It should listen on TCP port 6200.
*  sproxy 6200
*/

#include "PortableSocket.h"
#include <sys/select.h>
#define size 1024

int main(int argc, char *argv[])
{
  if (argc != 2)
    return 1;

  /*
  * Parse the inputs
  */
  int serverPort = atoi(argv[1]);

  /*
  * Open the network
  */
  cpOpenNetwork();

  printf("connecting: client\n");
  /*
  * Connection to the client proxy
  */
  struct PortableSocket *clientAcceptor = cpSocket(TCP, "localhost", atoi(argv[1]));
  cpBind(clientAcceptor);
  cpListen(clientAcceptor, 5);
  struct PortableSocket *clientProxy = cpAccept(clientAcceptor);
  printf("connecting: telnet\n");
  /*
  * Connection to the local telnet
  */
  struct PortableSocket *telnetSocket = cpSocket(TCP, "127.0.0.1", 23);
  cpConnect(telnetSocket);
  printf("connected\n");
  /*
  * Foward data from one port to another
  */
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(clientProxy->socket, &readfds);
  FD_SET(telnetSocket->socket, &readfds);
  int n = telnetSocket->socket + 1;
  char message[size];
  printf("looping server\n");
  while (cpCheckError(telnetSocket) == 0 && cpCheckError(clientProxy) == 0)
  {
    if (select(n, &readfds, NULL, NULL, 2000) <= 0)
      break;
    // foward the message
    if (FD_ISSET(telnetSocket->socket, &readfds))
    {
      // print "recieved from telnet 'message' sending to sproxy"
      cpRecv(telnetSocket, message, size);
      printf("Recieved from telnet: '%s'\n", message);
      cpSend(clientProxy, message, size);
    }
    if (FD_ISSET(clientProxy->socket, &readfds))
    {
      // print "recieved from telnet 'message' sending to cproxy"
      cpRecv(clientProxy, message, size);
      printf("Recieved from client: '%s'\n", message);
      cpSend(telnetSocket, message, size);
    }
  }

  /*
  * Close the connections
  */
  cpClose(clientAcceptor);
  cpClose(clientProxy);
  cpClose(telnetSocket);
  cpCloseNetwork();
  return 0;
}
