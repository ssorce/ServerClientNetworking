# Makefile for HW1 of CSC422
# Declaration of variables
CC = gcc
CC_FLAGS = -Wall -g
DEPS = PortableSocket.h

.PHONY: all clean

all: server client

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

client: PortableClient.o PortableSocket.o
	$(CC) $(CFLAGS) -o $@ $^

server: PortableServer.o PortableSocket.o
	$(CC) $(CFLAGS) -o $@ $^

#clean the object files and executables
clean:
	rm *.o client server
