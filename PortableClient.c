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
	printf("Server says %s_%d\n", message, len);
	cpClose(socket);
	cpCloseNetwork();
	return 0;
}
