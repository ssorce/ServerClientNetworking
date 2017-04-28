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
int heartbeatsSinceLastReply;
struct PortableSocket * telnetAcceptorSocket;
struct PortableSocket * telnetSocket;
struct PortableSocket * sproxySocket;
int n;

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
  FD_CLR(telnetAcceptorSocket->socket, readfds);
  FD_ZERO(readfds);
  FD_SET(serverSocket, readfds);
  FD_SET(telnetSocket, readfds);
  FD_SET(telnetAcceptorSocket->socket, readfds);
}

//forwards a message from the sender socket to the reciever socket
int forward(struct PortableSocket * sender, struct PortableSocket * reciever, char * message, char * senderName){
  // print "recieved from telnet 'message' sending to sproxy"
  int messageSize = cpRecv(sender, message, size);
  if(cpCheckError(sender) != 0)
    return -1;
  if (mode == 1)
    printf("Recieved %d bytes from %s: %s\n", messageSize, senderName, message);
  struct message messageStruct;
  initMessageStruct(&messageStruct,MESSAGE,messageSize,message);
  sendMessageStruct(&messageStruct,reciever);
  return messageSize;
}

//forwards a message from the sender socket to the reciever socket
int sendMessage(struct PortableSocket * reciever, char * message, int messageSize){
  if(mode == 1)
    printf("Sending '%s' to telnet\n",message);
  cpSend(reciever, message, messageSize);
  memset(message, 0, messageSize);
  return 0;
}

int recvMessage(struct PortableSocket * sender, struct PortableSocket * reciever){
  struct message messageStruct;
  char message[size];
  messageStruct.payload = message;
  recvMessageStruct(&messageStruct, sender);
  if(messageStruct.type == MESSAGE){
    sendMessage(reciever,messageStruct.payload,messageStruct.length);
  } else if (messageStruct.type == HEARTBEAT){
    printf("recived heartbeat reply\n");
    heartbeatsSinceLastReply = 0;
    return 1;
  }
  return messageStruct.length;
}

void sendHeartbeat(struct PortableSocket * reciever){
  heartbeatsSinceLastReply++;
  struct message messageStruct;
  char empty[0];
  empty[0] = '\0';
  initMessageStruct(&messageStruct,HEARTBEAT,0,empty);
  sendMessageStruct(&messageStruct,reciever);
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
  telnetAcceptorSocket = getTelnetAcceptor();
  telnetSocket = getTelnet(telnetAcceptorSocket);

  /*
  * Create connection to sproxy
  */
  if (mode == 1)
    printf("Attempting to create sproxy socket\n");
  sproxySocket = getSproxy();
  struct message newConnectStruct;
  char empty[0];
  empty[0] = '\0';
  initMessageStruct(&newConnectStruct,NEW_CONNECTION,0,empty);
  sendMessageStruct(&newConnectStruct, sproxySocket);
  /*
  * set up data for the program
  */
  fd_set readfds;
  int socketN[] = {sproxySocket->socket, telnetSocket->socket, telnetAcceptorSocket->socket};
  n = getN(socketN, 3);
  char message[size];
  memset(message, 0, size);
  struct timeval tv = {1, 0};
  /*
  * run the program
  */
  while (cpCheckError(sproxySocket) == 0 && cpCheckError(telnetSocket) == 0){
      reset(&readfds, telnetSocket->socket, sproxySocket->socket);
      if (mode == 1)
        printf("Waiting for message \n");
      struct timeval tv2 = {1, 0};
      selectValue = select(n, &readfds, NULL, NULL, &tv);
      if(selectValue == 0){
        if (mode == 1)
          printf("Sending heartbeat \n");
        sendHeartbeat(sproxySocket);
        tv = tv2;
      }
      // foward the message
      if (FD_ISSET(telnetSocket->socket, &readfds)) {
        int result = forward(telnetSocket,sproxySocket,message,"telnet");
        if(result <= 0)
          break;
      }
      if (FD_ISSET(sproxySocket->socket, &readfds)){
        int result = recvMessage(sproxySocket,telnetSocket);
        if(result <= 0)
          break;
      }
      if(FD_ISSET(telnetAcceptorSocket->socket, &readfds)){
        printf("Detected new telnet session");
        cpClose(telnetSocket);
        telnetSocket = getTelnet(telnetAcceptorSocket);
        struct message reconnectStruct;
        char empty[0];
        empty[0] = '\0';
        initMessageStruct(&reconnectStruct,NEW_CONNECTION,0,empty);
        sendMessageStruct(&reconnectStruct, sproxySocket);
        int socketN[] = {sproxySocket->socket, telnetSocket->socket, telnetAcceptorSocket->socket};
        n = getN(socketN, 3);
      }
      if(heartbeatsSinceLastReply > 3){
        if(mode == 1)
          printf("Attempting reconnect\n");
        cpClose(sproxySocket);
        sproxySocket = getSproxy();
        struct PortableSocket *sproxySocket = getSproxy();
        struct message reconnectStruct;
        char empty[0];
        empty[0] = '\0';
        initMessageStruct(&reconnectStruct,RECONNECT,0,empty);
        sendMessageStruct(&reconnectStruct, sproxySocket);
        heartbeatsSinceLastReply = 0;
      }
  }

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
