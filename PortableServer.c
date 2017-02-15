#include "stdio.h"
#include "PortableSocket.h"

int main(){
	char message[256];
	cpOpenNetwork();
	struct PortableSocket* socket = cpSocket(TCP,"localhost",10003);
	cpBind(socket);
	cpListen(socket,5);
	struct PortableSocket* client = cpAccept(socket);
	scanf("%s", message);
	cpSend(client, message,256);
	cpClose(client);
	cpClose(socket);
	cpCloseNetwork();
	return 0;
}
