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

// resets the select method, to be used again
void reset(fd_set * readfds, int telnetSocket, int clientSocket){
  FD_CLR(telnetSocket, readfds);
  FD_CLR(clientSocket, readfds);
  FD_ZERO(readfds);
  FD_SET(clientSocket, readfds);
  FD_SET(telnetSocket, readfds);
}

//forwards a message from the sender socket to the reciever socket
int forward(struct PortableSocket * sender, struct PortableSocket * reciever, char * message, char * senderName){
  // print "recieved from telnet 'message' sending to sproxy"
  struct message messageStruct;
  messageStruct.type = MESSAGE;
  strcpy(messageStruct.message,message);
  char serialized[size];
  serialize(&messageStruct, serialized);
  int messageSize = cpRecv(sender, serialized, size);
  if (mode == 1)
    printf("Recieved from %s: '%s'\n", senderName, message);
  cpSend(reciever, message, messageSize);
  memset(message, 0, messageSize);
  return 0;
}

//forwards a message from the sender socket to the reciever socket
int sendMessage(struct PortableSocket * reciever, char * message){
  cpSend(reciever, message, size);
  memset(message, 0, size);
  return 0;
}

void getMessage(struct message * message, struct PortableSocket * sender){
  char messageAsChar[size];
  int messageSize = cpRecv(sender, messageAsChar, size);
  deserialize(messageAsChar,message);
}

//gets the clientAcceptor socket
struct PortableSocket * getClientAcceptor(int serverPort){
  struct PortableSocket * clientAcceptor = cpSocket(TCP, "localhost", serverPort);
  if (cpCheckError(clientAcceptor) != 0) {
    fprintf(stderr, "Failed to create client acceptor socket \n");
    exit(1);
  }
  else if (mode == 1) {
    printf("Client acceptor socket created\n");
  }
  cpBind(clientAcceptor);
  cpListen(clientAcceptor, 5);
  return clientAcceptor;
}

//gets the client socket
struct PortableSocket * getClient(struct PortableSocket * clientAcceptor){
  struct PortableSocket * client = cpAccept(clientAcceptor);
  if (cpCheckError(client) != 0){
    fprintf(stderr, "Failed to create client socket \n");
    exit(1);
  }
  else if (mode == 1){
    printf("Client socket created\n");
  }
  return client;
}

//gets the client socket
struct PortableSocket * getTelnet(){
  struct PortableSocket *telnetSocket = cpSocket(TCP, "127.0.0.1", 23);
  cpConnect(telnetSocket);
  if (cpCheckError(telnetSocket) != 0){
    fprintf(stderr, "Failed to create telnet acceptor socket \n");
    exit(1);
  }
  else if (mode == 1){
    printf("Telnet socket created\n");
  }
  return telnetSocket;
}

//gets the value of n for select
int getN(int socket[], int numberOfSockets){
  int max = -1;
  int i = 0;
  for(i = 0; i < numberOfSockets; i++){
    if(socket[i] > max)
      max = socket[i];
  }
  return max + 1;
}

//parses the input
void parseInput(int argc, char *argv[]){
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
  struct PortableSocket * clientAcceptor = getClientAcceptor(serverPort);
  struct PortableSocket * clientProxy = getClient(clientAcceptor);

  /*
  * Connection to the local telnet
  */
  if (mode == 1)
    printf("connecting: telnet\n");
    struct PortableSocket * telnetSocket = getTelnet();

  /*
  * set up data for program
  */
  fd_set readfds;
  int socketN[] = {clientProxy->socket, telnetSocket->socket};
  int n = getN(socketN, 2);
  struct message messageStruct;
  char message[size];
  memset(message, 0, size);
  struct timeval tv;
  tv.tv_sec = 3;

  /*
  * run the program
  */
  while (cpCheckError(telnetSocket) == 0) {
    reset(&readfds, telnetSocket->socket, clientProxy->socket);
    if (mode == 1)
      printf("Waiting for message \n");
    selectValue = select(n, &readfds, NULL, NULL, &tv);
    // foward the message
    if (FD_ISSET(telnetSocket->socket, &readfds)) {
      forward(telnetSocket, clientProxy, message,"telnet");
    }
    if (FD_ISSET(clientProxy->socket, &readfds)) {
      getMessage(&messageStruct,clientProxy);
      if(messageStruct.type == MESSAGE){
        sendMessage(telnetSocket,messageStruct.message);
      } else if (messageStruct.type == HEARTBEAT){}
        //TODO: implement heatbeat recv
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
