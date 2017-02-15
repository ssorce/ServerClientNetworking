#include "stdio.h"
#include "PortableSocket.h"
#include "string.h"

int main(){
	int len;
	cpOpenNetwork();
	struct PortableSocket* socket = cpSocket(TCP,"localhost",10003);
	cpConnect(socket);
	char message[256];
	cpRecv(socket,message,256);
	len = strlen(message);
	while(cpCheckError(socket) == 0)
	{
		if(len == 0)
			break;
		printf("%d\n%s\n",len, message);
		*message = '\0';
		cpRecv(socket,message,256);
		len = strlen(message);
	}
	cpClose(socket);
	cpCloseNetwork();
	return 0;
}
