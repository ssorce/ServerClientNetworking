#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "PortableSocket.h"
#define size 1024

int main(int argc, char *argv[]){
	int len;
	char message[size];
	//initialize the network
	cpOpenNetwork();
	//open up the socket
	struct PortableSocket* socket = cpSocket(TCP,"localhost", atoi(argv[1]));
	cpBind(socket);
	cpListen(socket,5);
	//connect to the client
	struct PortableSocket* client = cpAccept(socket);
	//print out what the client sends
	while(cpCheckError(client) == 0 || feof(stdin))
	{
		len = cpRecv(client,message,size);
		if(len == 0)
			break;
		printf("%d\n%s\n",len, message);
		//delete the message
		memset(message, 0 , size);
	}
	//close the network
	cpClose(client);
	cpClose(socket);
	cpCloseNetwork();
	return 0;
}
