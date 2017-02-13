#include "CustomPortable/PortableSocket.h"

int main(){
	cpOpenNetwork();
	struct PortableSocket* socket = cpSocket(TCP,"localhost",10003);
	cpBind(socket);
	cpListen(socket,5);
	struct PortableSocket* client = cpAccept(socket);
	cpSend(client, "Hello World!",256);
	cpClose(client);
	cpClose(socket);
	cpCloseNetwork();
	return 0;
}
