#include "stdio.h"
#include "PortableSocket.h"
#include "string.h"
#include "stdlib.h"

int main(int argc, char *argv[]){
	cpOpenNetwork();
	struct PortableSocket* socket = cpSocket(TCP,argv[1],atoi(argv[2]));
	cpConnect(socket);
	char message[256];
	while(fgets(message, 256, stdin) != NULL && cpCheckError(socket) == 0)
	{
		if(*message == '\n')
			break;
		cpSend(socket, message, 256);
	}
	cpClose(socket);
	cpCloseNetwork();
	return 0;
}
