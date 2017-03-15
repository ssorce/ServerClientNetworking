/*
*  Start sproxy on ServerVM. It should listen on TCP port 6200.
*  sproxy 6200
*/

// I just grabbed normal *.h, but we can delete them after

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char * argv[])
{
	if (argc == 2)
	{
		int port = atoi(argv[1]); // Should be 6200
		return 0;
	}
	else
		return 1;
}