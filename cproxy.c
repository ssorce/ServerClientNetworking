/*
* Start the cproxy on ClientVM to listen on TCP port 5200, and specify the sproxy:
* cproxy 5200 sip 6200
* On ClientVM, telnet to cproxy:
* telnet localhost 5200
*/

// I just grabbed normal *.h, but we can delete them after

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <PortableSocket.h>

int main(int argc, char * argv[])
{
	if (argc == 4)
	{
		int ServerPort = atoi(argv[3]); // Server Port
		// SIP (Server IP)
		int ClientPort = atoi(argv[1]); // Client Port
		return 0;
	}
	else
		return 1;
}