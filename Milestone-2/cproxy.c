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
  if (argc < 4)
    return 1;

  /*
  * Parse the inputs
  */
  int current = 1;
  int mode = 0;
  if(argc == 5)
    if(strchr(argv[current++], 'd'))
      mode = 1;
  int clientPort = atoi(argv[current++]);
  char *serverAddress = argv[current++];
  int serverPort = atoi(argv[current++]);

  if(mode == 1)
    printf("Running cproxy on port %d and connecting to sproxy at %s on port %d\n",clientPort,serverAddress,serverPort);

  /*
    * Open the network
    */
  cpOpenNetwork();

  /*
  * Connection to the local telnet
  */
  if(mode == 1)
    printf("Attempting to create telnet socket\n");
  struct PortableSocket *telnetAcceptorSocket = cpSocket(TCP, "127.0.0.1", clientPort);
  if(cpCheckError(telnetAcceptorSocket) != 0){
    fprintf(stderr, "Failed to create telnet acceptor socket \n");
    exit(1);
  } else if (mode == 1){
    printf("Telnet acceptor socket created\n");
  }
  cpBind(telnetAcceptorSocket);
  cpListen(telnetAcceptorSocket, 5);
  struct PortableSocket *telnetSocket = cpAccept(telnetAcceptorSocket);
  if(cpCheckError(telnetSocket) != 0){
    fprintf(stderr, "Failed to create telnet socket \n");
    exit(1);
  } else if (mode == 1){
    printf("Telnet socket created\n");
  }

  /*
  * Create connection to sproxy
  */
  if(mode == 1)
    printf("Attempting to create sproxy socket\n");
  struct PortableSocket *sproxySocket = cpSocket(TCP, serverAddress, serverPort);
  cpConnect(sproxySocket);
  if(cpCheckError(sproxySocket) != 0){
    fprintf(stderr, "Failed to create sproxy socket\n");
    exit(1);
  } else if (mode == 1){
    printf("Sproxy socket created\n");
  }
  /*
    * Foward data from one port to another
    */
  fd_set readfds;
  FD_ZERO(&readfds);
  int n = 0;
  if(sproxySocket->socket > telnetSocket->socket)
    n = sproxySocket->socket + 1;
  else
    n = telnetSocket->socket + 1;
  char message[size];
  memset(message, 0 , size);
  while (cpCheckError(telnetSocket) == 0 && cpCheckError(sproxySocket) == 0)
  {
    FD_SET(telnetSocket->socket, &readfds);
    FD_SET(sproxySocket->socket, &readfds);
    if(mode == 1)
      printf("Waiting for message \n");
    if (select(n, &readfds, NULL, NULL, NULL) <= 0)
      break;
    // foward the message
    if (FD_ISSET(telnetSocket->socket, &readfds))
    {
      // print "recieved from telnet 'message' sending to cproxy"
      cpRecv(telnetSocket, message, size);
      if(mode == 1)
        printf("Recieved from telnet (client): '%s'\n", message);
      cpSend(sproxySocket, message, size);
    }
    if (FD_ISSET(sproxySocket->socket, &readfds))
    {
      // print "sending from telnet 'message' sending to cproxy"
      cpRecv(sproxySocket, message, size);
      if(mode == 1)
        printf("Recieved from server: '%s'\n", message);
      cpSend(telnetSocket, message, size);
    }
    FD_CLR(telnetSocket->socket, &readfds);
    FD_CLR(sproxySocket->socket, &readfds);
  }

  /*
    * Close the connections
    */
  cpClose(telnetAcceptorSocket);
  cpClose(telnetSocket);
  cpClose(sproxySocket);
  cpCloseNetwork();
  if(mode == 1)
    printf("all sockets closed\n");
  return 0;
}
