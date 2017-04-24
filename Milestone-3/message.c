#include "message.h"

//Serializes the message in order to send over the network
void serialize(struct message *message, char *output)
{
  sprintf(output, "%d%s", message->type, message->message);
}

//Deserializes the message back into a struct
void deserialize(char *serializedMessage, struct message *output)
{
  printf("Sent: %d\n", serializedMessage[0]);\
  if (strlen(serializedMessage) > 0)
  {
    output->type = serializedMessage[0] - '0';
    memcpy(output->message, &serializedMessage[1], (strlen(serializedMessage) - 1));
  }
  else
  {
    output->type = MESSAGE;
    memset(output->message, 0, 1024);
  }
}
