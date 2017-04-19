# Makefile for HW1 of CSC422
# Declaration of variables
CC = gcc
CC_FLAGS = -Wall -g
DEPS = PortableSocket.h

.PHONY: all clean

all: sproxy cproxy

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

cproxy: cproxy.o PortableSocket.o
	$(CC) $(CFLAGS) -o $@ $^

sproxy: sproxy.o PortableSocket.o
	$(CC) $(CFLAGS) -o $@ $^

#clean the object files and executables
clean:
	rm *.o sproxy cproxy
