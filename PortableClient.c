#include "stdio.h"
#include "PortableSocket.h"
#include "string.h"
#include "stdlib.h"
#define size 1024

int main(int argc, char *argv[]){
	//connect to the server
	cpOpenNetwork();
	struct PortableSocket* socket = cpSocket(TCP,argv[1],atoi(argv[2]));
	cpConnect(socket);
	char message[size];
	//collect the message from stdin
	while(fgets(message, size, stdin) != NULL && cpCheckError(socket) == 0)
	{
		if(*message != '\n')
		{
			//send the message without the newline or null charcter
			cpSend(socket, message, strlen(message)-1);
		}
	}
	//close the network
	cpClose(socket);
	cpCloseNetwork();
	return 0;
}
