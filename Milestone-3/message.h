#ifndef MESSAGE_HEADER
#define MESSAGE_HEADER
#define HEARTBEAT -1
#define MESSAGE -2
#define NEW_CONNECTION -3
#define RECONNECT -4

#include <string.h>
#include <stdio.h>

typedef struct message
{
  int type;
  char message[1024];
} message;

//Serializes the message in order to send over the network
void serialize(struct message *message, char *output);

//Deserializes the message back into a struct
void deserialize(char *message, struct message *output);

#endif
