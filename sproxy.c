/*
*  Start sproxy on ServerVM. It should listen on TCP port 6200.
*  sproxy 6200
*/

// I just grabbed normal *.h, but we can delete them after

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char *argv[])
{
    if (argc == 2)
    {
	cpOpenNetwork();
	// (address 127.0.0.1, port 23 )

	/*
	*  I was thinking it goes from Server to Client then that is Proxysocket then is connected.
	*  I binded then connected.
	*/
	struct PortableSocket* socket = cpSocket(TCP,"localhost", atoi(argv[1]));
	cpBind(socket);
	struct PortableSocket *Proxysocket = cpSocket(TCP, "127.0.0.1", 23);
	cpConnect(socket);
	struct PortableSocket* client = cpAccept(socket);
	cpListen(socket, 5);
	return 0;
    }
    else
	return 1;
}