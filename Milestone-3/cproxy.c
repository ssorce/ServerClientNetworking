/*
* Start the cproxy on ClientVM to listen on TCP port 5200, and specify the sproxy:
* cproxy 5200 sip 6200
* On ClientVM, telnet to cproxy:
* telnet localhost 5200
*/


/*
* Keeps killing connection when telnet start.
*
*/



#include "PortableSocket.h"
#include <sys/select.h>
#include "message.h"
#define size 1024

int mode;
int selectValue;
int clientPort;
char *serverAddress;
int serverPort;

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
  mode = 0;
  selectValue = 0;
  if (argc == 5)
    if (strchr(argv[current++], 'd'))
      mode = 1;
  clientPort = atoi(argv[current++]);
  serverAddress = argv[current++];
  serverPort = atoi(argv[current++]);
}

//get the telnetAcceptorSocket
struct PortableSocket * getTelnetAcceptor(){
  struct PortableSocket * telnetAcceptorSocket = cpSocket(TCP, "127.0.0.1", clientPort);
  if (cpCheckError(telnetAcceptorSocket) != 0){
    fprintf(stderr, "Failed to create telnet acceptor socket \n");
    exit(1);
  }
  else if (mode == 1){
    printf("Telnet acceptor socket created\n");
  }
  cpBind(telnetAcceptorSocket);
  cpListen(telnetAcceptorSocket, 5);
  return telnetAcceptorSocket;
}

//get the telnetSocket
struct PortableSocket * getTelnet(struct PortableSocket * telnetAcceptorSocket){
  struct PortableSocket *telnetSocket = cpAccept(telnetAcceptorSocket);
  if (cpCheckError(telnetSocket) != 0){
    fprintf(stderr, "Failed to create telnet socket \n");
    exit(1);
  }
  else if (mode == 1){
    printf("Telnet socket created\n");
  }
  return telnetSocket;
}

struct PortableSocket * getSproxy(){
  struct PortableSocket *sproxySocket = cpSocket(TCP, serverAddress, serverPort);
  cpConnect(sproxySocket);
  if (cpCheckError(sproxySocket) != 0){
    fprintf(stderr, "Failed to create sproxy socket\n");
    exit(1);
  }
  else if (mode == 1){
    printf("Sproxy socket created\n");
  }
  return sproxySocket;
}

// resets the select method, to be used again
void reset(fd_set * readfds, int telnetSocket, int serverSocket){
  FD_CLR(telnetSocket, readfds);
  FD_CLR(serverSocket, readfds);
  FD_ZERO(readfds);
  FD_SET(serverSocket, readfds);
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

int getMessage(struct message * message, struct PortableSocket * sender){
  char messageAsChar[size];
  int messageSize = cpRecv(sender, messageAsChar, size);
  deserialize(messageAsChar,message);
}

int main(int argc, char *argv[]) {
  if (argc < 4)
    return 1;

  /*
  * Parse the inputs
  */
  parseInput(argc, argv);

  if (mode == 1)
    printf("Running cproxy on port %d and connecting to sproxy at %s on port %d\n", clientPort, serverAddress, serverPort);

  /*
    * Open the network
    */
  cpOpenNetwork();

  /*
  * Connection to the local telnet
  */
  if (mode == 1)
    printf("Attempting to create telnet socket\n");
  struct PortableSocket * telnetAcceptorSocket = getTelnetAcceptor();
  struct PortableSocket * telnetSocket = getTelnet(telnetAcceptorSocket);

  /*
  * Create connection to sproxy
  */
  if (mode == 1)
    printf("Attempting to create sproxy socket\n");
  struct PortableSocket *sproxySocket = getSproxy();
  /*
    * set up data for the program
    */
  fd_set readfds;
  int socketN[] = {sproxySocket->socket, telnetSocket->socket};
  int n = getN(socketN, 2);
  char message[size];
  memset(message, 0, size);
  struct message messageStruct;
  struct timeval tv;
  tv.tv_sec = 1;

  /*
  * run the program
  */
  while (cpCheckError(sproxySocket) == 0)
  {
      reset(&readfds, telnetSocket->socket, sproxySocket->socket);
      if (mode == 1)
        printf("Waiting for message \n");
      selectValue = select(n, &readfds, NULL, NULL, &tv);
      // foward the message
      if (FD_ISSET(telnetSocket->socket, &readfds)) {
        forward(telnetSocket,sproxySocket,message,"telnet");
      }
      if (FD_ISSET(sproxySocket->socket, &readfds)){
        getMessage(&messageStruct,sproxySocket);
        if(messageStruct.type == MESSAGE){
          sendMessage(telnetSocket,messageStruct.message);
        } else if (messageStruct.type == HEARTBEAT){}
          //TODO: implement heatbeat recv
        }
      }
      //TODO Implement heartbeat send

  /*
    * Close the connections
    */
  cpClose(telnetAcceptorSocket);
  cpClose(telnetSocket);
  cpClose(sproxySocket);
  cpCloseNetwork();
  if (mode == 1)
    printf("all sockets closed\n");
  return 0;
}
