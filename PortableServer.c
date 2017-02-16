#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "PortableSocket.h"

int main(int argc, char *argv[]){
	int len;
	char message[256];
	cpOpenNetwork();
	struct PortableSocket* socket = cpSocket(TCP,"localhost", atoi(argv[1]));
	cpBind(socket);
	cpListen(socket,5);
	struct PortableSocket* client = cpAccept(socket);
	cpRecv(client,message,256);
	len = strlen(message);
	while(cpCheckError(client) == 0 || feof(stdin))
	{
		if(len == 0)
			break;
		printf("%d\n%s",(len-1), message);
		*message = '\0';
		cpRecv(client,message,256);
		len = strlen(message);
	}
	cpClose(client);
	cpClose(socket);
	cpCloseNetwork();
	return 0;
}
