/*
 * PortableSocket.c
 * A socket interface compatiable on Windows and Ubuntu
 *
 *  Created on: Jan 23, 2017
 *      Author: Jonathon Davis
 */

#include "PortableSocket.h"

//private functions
struct PortableSocket * newTCPSocket(char * address, int port);
int cpCheckError(struct PortableSocket * socket);

/*
 * openNetwork is the first function that must be called
 * whenever sockets are going to be used. This function initializes
 * certain parameters that are system dependent. This function only ever
 * needs to be called once, and will allow the creation and use of sockets
 * throughout the remainder of the programs runtime.
 */
int cpOpenNetwork() {
	//Intializes the WSAStartup for winsock2 sockets on windows machines.
	#ifdef __WIN32__
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != NO_ERROR)
			return 1;
	#endif
	return 0;
}

/*
 * newPortableSocket creates a new portable socket and returns the socket
 * for use. The first parameter protocol is either TCP or UDP in which the
 * portocol for that socket is set up. The second parameter address is a string
 * representation of the address the socket will connect to ex: "192.168.1.1".
 * The final parameter is an int which is the port number the socket will be open on.
 */
struct PortableSocket * cpSocket(int protocol, char * address, int port) {
	if(protocol == TCP)
		return newTCPSocket(address, port);
	return NULL;
}

/*
 * newTCPSocket is a private method that creates a new TCP socket. The first parameter
 * address is a string representation of the address the socket will connect
 * to ex: "192.168.1.1". The final parameter is an int which is the port number
 * the socket will be open on.
 */
struct PortableSocket * newTCPSocket(char * address, int port){
	//create the TCP socket
	int newsocket = socket(PF_INET, SOCK_STREAM, 0);

	//define server address
	struct sockaddr_in sock_address;
	memset(&sock_address, 0, sizeof(sock_address));
	sock_address.sin_family = AF_INET;
	sock_address.sin_port = htons(port);
	#ifdef __WIN32__
		if(strcmp(address,"localhost") == 0){
			sock_address.sin_addr.S_un.S_addr = inet_addr(inet_ntoa(*(struct in_addr *)*gethostbyname("")->h_addr_list));
		} else {
			sock_address.sin_addr.S_un.S_addr = inet_addr(address);
		}
	#else
		if(strcmp(address,"localhost") == 0){
			sock_address.sin_addr.s_addr = INADDR_ANY;
		} else {
			sock_address.sin_addr.s_addr = inet_addr(address);
		}
	#endif

	//create the return type
	struct PortableSocket* newPS = malloc(sizeof(struct PortableSocket));
	newPS->socket = newsocket;
	newPS->address = sock_address;
	newPS->error = newsocket;
	newPS->error = cpCheckError(newPS);
	// returns the new socket
	return newPS;
}

/*
 * Binds the socket to the associated address space, allowing it to accept
 * connections from that address. The return value is 0 if the bind was successful
 * and -1 otherwise, the error value is updated accordingly.
 */
int cpBind(struct PortableSocket * socket){
	socket->error = bind(socket->socket, (struct sockaddr *) &socket->address, sizeof(socket->address));
	return cpCheckError(socket);
}

/*
 * The passiveListen function sets up the socket to passively listen for connections
 * to accept. The first parameter is the socket that will be set to passively listen
 * for connections, The second parameter is the size of the buffer, the number of connections
 * that can be queued at any given time.
 */
int cpListen(struct PortableSocket * socket, int bufferSize){
	socket->error = listen(socket->socket, bufferSize);
	return cpCheckError(socket);
}

/*
 * the acceptConnection function is a blocking call that will accept
 * a connection to the socket, this will create a new PortableSocket that
 * can then be used to communicate will the client socket.
 */
struct PortableSocket * cpAccept(struct PortableSocket * socket){
	int clientSocket = accept(socket->socket,NULL,NULL);
	struct PortableSocket* newPS = malloc(sizeof(struct PortableSocket));
	newPS->socket = clientSocket;
	newPS->address = socket->address;
	newPS->error = clientSocket;
	newPS->error = cpCheckError(newPS);
	return newPS;
}

/*
 * The connect function will have the socket connect to the address that it is associated
 * with. The connect function takes one parameter, which is the socket. The function returns
 * 0 if the connections was established, and returns an error code otherwise.
 */
int cpConnect(struct PortableSocket * socket){
	socket->error = connect(socket->socket, (struct sockaddr *) &socket->address, sizeof(socket->address));
	return cpCheckError(socket);
}

/*
 * The transmit function will transmit a message over the network through
 * the socket. The first parameter is the socket that will be transmiting the message.
 * The second parameter will be the message that is being transmitted.
 */
int cpSend(struct PortableSocket * socket, char* message, int messageSize){
	socket->error = send(socket->socket, message, messageSize,0);
	return cpCheckError(socket);
}

/*
 * The receive function will receive an incoming transmition. The first parameter is the
 * socket that will be receiving the message. The second parameter will be the
 * the variable the message being recieved will be stored. The third parameter is
 * The bufferSize of the message.
 */
int cpRecv(struct PortableSocket * socket, char* message, int bufferSize){
	socket->error = recv(socket->socket, message, bufferSize,0);
	return cpCheckError(socket);
}

/*
 * The closeSocket function will close the inputed socket, and free the memory used
 * by the socket. returns 0 if successful, error code otherwise.
 */
int cpClose(struct PortableSocket* socket){
	#ifdef __WIN32__
		socket->error = closesocket(socket->socket);
	#else
		socket->error = close(socket->socket);
	#endif
	int error = cpCheckError(socket);
	free(socket);
	return error;
}

/*
 * The closeNetwork function must be called whenever the application is finished using
 * all sockets and will no longer need acsess to the network. This function will clean
 * up any additional resources that may be used by the platform specific implementation.
 */
int cpCloseNetwork(){
	#ifdef __WIN32__
	WSACleanup();
	#endif
	return 0;
}

/*
 * checks to see if an error has occured and returns the error code, also sets
 * the Portable Sockets error field.
 */
int cpCheckError(struct PortableSocket * socket){
	if(socket->error < 0){
	#ifdef __WIN32__
		socket->error = WSAGetLastError();
	#endif
	return socket->error;
	} else {
		return 0;
	}
}
