#include "stdio.h"
#include "PortableSocket.h"
#include "string.h"
#include "stdlib.h"

int main(int argc, char *argv[]){
	int len; // Will get rid of latter
	cpOpenNetwork();
	struct PortableSocket* socket = cpSocket(TCP,argv[1],atoi(argv[2]));
	cpConnect(socket);
	char message[256];
	cpRecv(socket,message,256);
	len = strlen(message);
	while(cpCheckError(socket) == 0 || feof(stdin))
	{
		if(len == 0)
			break;
		printf("%d\n%s\n",(len-1), message);
		*message = '\0';
		cpRecv(socket,message,256);
		len = strlen(message);
	}
	cpClose(socket);
	cpCloseNetwork();
	return 0;
}
