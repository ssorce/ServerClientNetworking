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
  if (argc == 3)
    if (strchr(argv[current++], 'd'))
      mode = 1;
  int serverPort = atoi(argv[current++]);

  if (mode == 1)
    printf("Running sproxy on port %d\n", serverPort);

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
  if (cpCheckError(clientAcceptor) != 0)
  {
    fprintf(stderr, "Failed to create client acceptor socket \n");
    exit(1);
  }
  else if (mode == 1)
  {
    printf("Client acceptor socket created\n");
  }
  cpBind(clientAcceptor);
  cpListen(clientAcceptor, 5);
  struct PortableSocket *clientProxy = cpAccept(clientAcceptor);
  if (cpCheckError(clientProxy) != 0)
  {
    fprintf(stderr, "Failed to create client socket \n");
    exit(1);
  }
  else if (mode == 1)
  {
    printf("Client socket created\n");
  }

  /*
  * Connection to the local telnet
  */
  if (mode == 1)
    printf("connecting: client\n");
  struct PortableSocket *telnetSocket = cpSocket(TCP, "127.0.0.1", 23);
  cpConnect(telnetSocket);
  if (cpCheckError(telnetSocket) != 0)
  {
    fprintf(stderr, "Failed to create telnet acceptor socket \n");
    exit(1);
  }
  else if (mode == 1)
  {
    printf("Telnet socket created\n");
  }
  /*
  * Foward data from one port to another
  */
  fd_set readfds;
  int n = 0;
  if (clientProxy->socket > telnetSocket->socket)
    n = clientProxy->socket + 1;
  else
    n = telnetSocket->socket + 1;
  char message[size];
  memset(message, 0, size);
  while (cpCheckError(clientProxy) == 0)
  {
    if (mode == 1)
      printf("Address telnet(server): %d\n", telnetSocket->address.sin_addr.s_addr);
    if (cpCheckError(telnetSocket) == 0)
    {
      FD_ZERO(&readfds);
      FD_SET(clientProxy->socket, &readfds);
      FD_SET(telnetSocket->socket, &readfds);
      if (mode == 1)
        printf("Waiting for message \n");
      select(n, &readfds, NULL, NULL, NULL);
      if (mode == 1)
        printf("Got message\n");
      // foward the message
      if (FD_ISSET(telnetSocket->socket, &readfds))
      {
        // print "recieved from telnet 'message' sending to sproxy"
        int messageSize = cpRecv(telnetSocket, message, size);
        if (messageSize <= 0)
          break;
        if (mode == 1)
          printf("Recieved from telnet: '%s'\n", message);
        cpSend(clientProxy, message, messageSize);
        memset(message, 0, messageSize);
      }
      if (FD_ISSET(clientProxy->socket, &readfds))
      {
        // print "recieved from telnet 'message' sending to cproxy"
        int messageSize = cpRecv(clientProxy, message, size);
        if (messageSize <= 0)
          break;
        if (mode == 1)
          printf("Recieved from client: '%s'\n", message);
        cpSend(telnetSocket, message, messageSize);
        memset(message, 0, messageSize);
      }
      FD_CLR(telnetSocket->socket, &readfds);
      FD_CLR(clientProxy->socket, &readfds);
    }
    else
    {
      printf("Socket break\n");
      break;
    }
  }

  /*
  * Close the connections
  */
  cpClose(clientAcceptor);
  cpClose(clientProxy);
  cpClose(telnetSocket);
  cpCloseNetwork();
  if (mode == 1)
    printf("all sockets closed\n");
  return 0;
}
