#include "CustomPortable/PortableSocket.h"

int main(){
	cpOpenNetwork();
	struct PortableSocket* socket = cpSocket(TCP,"192.168.1.103",10003);
	cpConnect(socket);
	char message[256];
	cpRecv(socket,message,256);
	printf("Server says %s\n",message);
	cpClose(socket);
	cpCloseNetwork();
	return 0;
}
