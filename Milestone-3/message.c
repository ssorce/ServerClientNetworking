#include "message.h"

const int HEARTBEAT = 0, MESSAGE = 1, NEW_CONNECTION = 2, RECONNECT = 3;
//Serializes the message in order to send over the network
void serialize(struct message * message, char * output){
  sprintf(output, "%d%s",message->type,message->message);
}

//Deserializes the message back into a struct
void deserialize(char * serializedMessage, struct message * output){
  output->type = serializedMessage[0] - '0';
  memcpy(output->message, &serializedMessage[1], strlen(serializedMessage)-1);
}
