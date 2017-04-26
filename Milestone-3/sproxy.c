/*
*  Start sproxy on ServerVM. It should listen on TCP port 6200.
*  sproxy 6200
*/

/*
* Keeps killing connection when telnet start.
*
Attempting to create sproxy socket
Failed to create sproxy socket
*/

#include "PortableSocket.h"
#include <sys/select.h>
#include "message.h"
#define size 1024

int mode = 0;
int selectValue = 0;
int serverPort = 0;
int clientConnected = 0;

// resets the select method, to be used again
void reset(fd_set *readfds, int telnetSocket, int clientSocket, int clientAcceptor)
{
  FD_CLR(telnetSocket, readfds);
  if (clientConnected == 1)
    FD_CLR(clientSocket, readfds);
  else
    FD_CLR(clientAcceptor, readfds);
  FD_ZERO(readfds);
  if (clientConnected == 1)
    FD_SET(clientSocket, readfds);
  else
    FD_SET(clientAcceptor, readfds);
  FD_SET(telnetSocket, readfds);
}

//forwards a message from the sender socket to the reciever socket
int forward(struct PortableSocket *sender, struct PortableSocket *reciever, char *message, char *senderName)
{
  // print "recieved from telnet 'message' sending to sproxy"
  int messageSize = cpRecv(sender, message, size);
  if (cpCheckError(sender) != 0)
    return -1;
  if (mode == 1)
    printf("Recieved %d bytes from %s: %s\n", messageSize, senderName, message);
  if (clientConnected == 1)
  {
    char type[10];
    memset(type, 0 , 10);
    sprintf(type,"%d %d",MESSAGE,messageSize);
    cpSend(reciever, type, 10);
    int sent = cpSend(reciever, message, messageSize);
    if(mode == 1 && sent < 0){
      printf("Sending error, storing value\n");
    }
  }
  memset(message, 0, messageSize);
  return messageSize;
}

//forwards a message from the sender socket to the reciever socket
int sendMessage(struct PortableSocket *reciever, char *message, int messageSize)
{
  if (mode == 1)
    printf("Sending '%s' to telnet\n", message);
  cpSend(reciever, message, messageSize);
  memset(message, 0, size);
  return 0;
}

void sendHeartbeat(struct PortableSocket *reciever)
{
  char type[10];
  type[0]='0';
  cpSend(reciever, type, 1);
}

int recvMessage(struct PortableSocket *sender, struct PortableSocket *reciever)
{
  char messageAsChar[size];
  char typeS[10];
  memset(messageAsChar, 0, size);
  memset(typeS, 0, 10);
  int messageSize = cpRecv(sender, typeS, 10);
  int type;
  sscanf(typeS,"%d",&type);
  if (mode == 1)
    printf("Recived message %s of type = %d\n", messageAsChar, type);
  if (type == MESSAGE)
  {
    int length;
    sscanf(typeS,"%d %d",&type,&length);
    if(mode == 1)
      printf("recived message of type MESSAGE of length = %d\n", length);
    messageSize = cpRecv(sender, messageAsChar, length);
    if (messageSize == 0)
      return 0;
    sendMessage(reciever, messageAsChar, messageSize);
  }
  else if (type == HEARTBEAT)
  {
    sendHeartbeat(sender);
  }
  return messageSize;
}

//gets the clientAcceptor socket
struct PortableSocket *getClientAcceptor(int serverPort)
{
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
  return clientAcceptor;
}

//gets the client socket
struct PortableSocket *getClient(struct PortableSocket *clientAcceptor)
{
  struct PortableSocket *client = cpAccept(clientAcceptor);
  if (cpCheckError(client) != 0)
  {
    fprintf(stderr, "Failed to create client socket \n");
    exit(1);
  }
  else if (mode == 1)
  {
    printf("Client socket created\n");
  }
  clientConnected = 1;
  return client;
}

//gets the client socket
struct PortableSocket *getTelnet()
{
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
  return telnetSocket;
}

//gets the value of n for select
int getN(int socket[], int numberOfSockets)
{
  int max = -1;
  int i = 0;
  for (i = 0; i < numberOfSockets; i++)
  {
    if (socket[i] > max)
      max = socket[i];
  }
  return max + 1;
}

//parses the input
void parseInput(int argc, char *argv[])
{
  int current = 1;
  selectValue = 0;
  if (argc == 3)
    if (strchr(argv[current++], 'd'))
      mode = 1;
  serverPort = atoi(argv[current++]);
}

int main(int argc, char *argv[])
{
  if (argc < 2)
    return 1;

  /*
  * Parse the inputs
  */
  parseInput(argc, argv);
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
  struct PortableSocket *clientAcceptor = getClientAcceptor(serverPort);
  struct PortableSocket *clientProxy = getClient(clientAcceptor);

  /*
  * Connection to the local telnet
  */
  if (mode == 1)
    printf("connecting: telnet\n");
  struct PortableSocket *telnetSocket = getTelnet();

  /*
  * set up data for program
  */
  fd_set readfds;
  int socketN[] = {clientProxy->socket, telnetSocket->socket};
  int n = getN(socketN, 2);
  char message[size];
  memset(message, 0, size);
  struct timeval tv = {3, 0};

  /*
  * run the program
  */
  while (cpCheckError(clientProxy) == 0 && cpCheckError(telnetSocket) == 0)
  {
    reset(&readfds, telnetSocket->socket, clientProxy->socket, clientAcceptor->socket);
    if (mode == 1)
      printf("Waiting for message \n");
    struct timeval tv2 = {3, 0};
    struct timeval tv3 = {30, 0};
    selectValue = select(n, &readfds, NULL, NULL, &tv);
    // foward the message
    if (FD_ISSET(telnetSocket->socket, &readfds))
    {
      int result = forward(telnetSocket, clientProxy, message, "telnet");
      if (result <= 0)
        break;
    }
    if (clientConnected == 1 && FD_ISSET(clientProxy->socket, &readfds))
    {
      int result = recvMessage(clientProxy, telnetSocket);
      if (result <= 0)
        break;
      tv = tv2;
    }
    if (clientConnected == 0 && FD_ISSET(clientAcceptor->socket, &readfds))
    {
      clientProxy = getClient(clientAcceptor);
      clientConnected = 1;
      int socketN[] = {telnetSocket->socket, clientProxy->socket};
      n = getN(socketN, 2);
      if (mode == 1)
      {
        printf("established new connection with client\n");
      }
    }
    if (selectValue == 0 && clientConnected == 0)
    {
      printf("Timedout\n");
      break;
    }
    if (selectValue == 0 && clientConnected == 1)
    {
      if (mode == 1)
        printf("Server connection timed out\n");
      cpClose(clientProxy);
      clientConnected = 0;
      int socketN[] = {telnetSocket->socket, clientAcceptor->socket};
      n = getN(socketN, 2);
      tv = tv3;
    }
  }
  if (cpCheckError(clientProxy) != 0 && mode == 1)
  {
    printf("cproxySocket gave check error\n");
  }
  else if (cpCheckError(telnetSocket) != 0 && mode == 1)
  {
    printf("telnetSocket gave check error\n");
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
