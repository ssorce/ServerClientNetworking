/*
*  Start sproxy on ServerVM. It should listen on TCP port 6200.
*  sproxy 6200
*/

#include "PortableSocket.h"
#include <sys/select.h>
#define size 1024

int main(int argc, char *argv[])
{
  if (argc < 2)
    return 1;

  /*
  * Parse the inputs
  */
  int mode = 0;
  int current = 1;
  if(argc == 3)
    if(strchr(argv[current++], 'd'))
      mode = 1;
  int serverPort = atoi(argv[current++]);

  if(mode == 1)
    printf("Running sproxy on port %d\n",serverPort);

  /*
  * Open the network
  */
  cpOpenNetwork();

  /*
  * Connection to the client proxy
  */
  if (mode == 1)
    printf("connecting: client\n");
  struct PortableSocket *clientAcceptor = cpSocket(TCP, "localhost", serverPort);
  if(cpCheckError(clientAcceptor) != 0){
    fprintf(stderr, "Failed to create client acceptor socket \n");
    exit(1);
  } else if (mode == 1){
    printf("Client acceptor socket created\n");
  }
  cpBind(clientAcceptor);
  cpListen(clientAcceptor, 5);
  struct PortableSocket *clientProxy = cpAccept(clientAcceptor);
  if(cpCheckError(clientProxy) != 0){
    fprintf(stderr, "Failed to create client socket \n");
    exit(1);
  } else if (mode == 1){
    printf("Client socket created\n");
  }

  /*
  * Connection to the local telnet
  */
  if (mode == 1)
    printf("connecting: client\n");
  struct PortableSocket *telnetSocket = cpSocket(TCP, "127.0.0.1", 23);
  cpConnect(telnetSocket);
  if(cpCheckError(telnetSocket) != 0){
    fprintf(stderr, "Failed to create telnet acceptor socket \n");
    exit(1);
  } else if (mode == 1){
    printf("Telnet socket created\n");
  }
  /*
  * Foward data from one port to another
  */
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(clientProxy->socket, &readfds);
  FD_SET(telnetSocket->socket, &readfds);
  int n = telnetSocket->socket + 1;
   char message[size];
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 20000;
  while (cpCheckError(telnetSocket) == 0 && cpCheckError(clientProxy) == 0)
  {
    if (select(n, &readfds, NULL, NULL, &tv) <= 0)
      break;
    // foward the message
    if (FD_ISSET(telnetSocket->socket, &readfds))
    {
      // print "recieved from telnet 'message' sending to sproxy"
      cpRecv(telnetSocket, message, size);
      if(mode == 1)
        printf("Recieved from telnet: '%s'\n", message);
      cpSend(clientProxy, message, size);
    }
    if (FD_ISSET(clientProxy->socket, &readfds))
    {
      // print "recieved from telnet 'message' sending to cproxy"
      cpRecv(clientProxy, message, size);
      if(mode == 1)
        printf("Recieved from client: '%s'\ncpCheck(telnet): %d\ncpCheck (client): %d\n", message, cpCheckError(telnetSocket), cpCheckError(clientProxy));
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
  if(mode == 1)
    printf("all sockets closed\n");
  return 0;
}
