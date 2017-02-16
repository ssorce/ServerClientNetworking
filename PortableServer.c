#include "stdio.h"
#include "stdlib.h"
#include "PortableSocket.h"

int main(int argc, char *argv[]){
	char message[256];
	cpOpenNetwork();
	struct PortableSocket* socket = cpSocket(TCP,"localhost", atoi(argv[1]));
	cpBind(socket);
	cpListen(socket,5);
	struct PortableSocket* client = cpAccept(socket);
	while(fgets(message, 256, stdin) != NULL && cpCheckError(socket) == 0)
	{
		if(*message == '\n')
			break;
		cpSend(client, message, 256);
	}
	
	cpClose(client);
	cpClose(socket);
	cpCloseNetwork();
	return 0;
}
