/*
*  Start sproxy on ServerVM. It should listen on TCP port 6200.
*  sproxy 6200
*/

// I just grabbed normal *.h, but we can delete them after

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

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

  /*
  * Connection to the client proxy
  */
  struct PortableSocket* clientAcceptor = cpSocket(TCP,"localhost", atoi(argv[1]));
  cpBind(socket);
  cpListen(socket,1);
  struct PortableSocket* clientProxy = cpAccept(socket);

  /*
  * Connection to the local telnet
  */
  struct PortableSocket *telentSocket = cpSocket(TCP, "127.0.0.1", 23);
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
  cpClose(clientAcceptor);
  cpClose(clientProxy);
  cpClose(telentSocket);
  cpCloseNetwork();
  return 0;
}
