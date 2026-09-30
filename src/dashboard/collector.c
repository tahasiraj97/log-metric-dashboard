#include "00_dashboard.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

struct sockaddr_in address = {
    .sin_family = AF_INET,
    .sin_addr.s_addr = INADDR_ANY
};

char message[65508];

int collector(void *data){
  int socketFD = socket(AF_INET, SOCK_DGRAM, 0);
  address.sin_port = htons(5500);
  bind(socketFD, (struct sockaddr *)&address, sizeof(address));

  while(true){
      int size = recv(socketFD, message, sizeof(message) - 1, 0);
      message[size] = '\0';
      if(strncmp(message, "DeviceInfo|", 11) == 0){
        SDL_LockMutex(messageMutex);
          if(messageHead == NULL){
            messageQueue *newMessage = malloc(sizeof(messageQueue));
            newMessage->message = strdup(message);
            newMessage->type = 1;
            newMessage->next = NULL;
            messageHead = newMessage;
            messageTail = newMessage;
          }
          else{
            messageQueue *newMessage = malloc(sizeof(messageQueue));
            newMessage->message = strdup(message);
            newMessage->type = 1;
            newMessage->next = NULL;
            messageTail->next = newMessage;
            messageTail = newMessage;
          }
        SDL_UnlockMutex(messageMutex);
      }
      else if(strncmp(message, "Log|", 4) == 0){
        SDL_LockMutex(messageMutex);
          if(messageHead == NULL){
            messageQueue *newMessage = malloc(sizeof(messageQueue));
            newMessage->message = strdup(message);
            newMessage->type = 2;
            newMessage->next = NULL;
            messageHead = newMessage;
            messageTail = newMessage;
          }
          else{
            messageQueue *newMessage = malloc(sizeof(messageQueue));
            newMessage->message = strdup(message);
            newMessage->type = 2;
            newMessage->next = NULL;
            messageTail->next = newMessage;
            messageTail = newMessage;
          }
        SDL_UnlockMutex(messageMutex);

      }
      SDL_SignalCondition(messageCondition);
  }

  return 0;
}