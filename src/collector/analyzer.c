#include "00_dashboard.h"
#include <stdio.h>
#include <stdlib.h>

int analyzer(void *data){
    while(true){
      SDL_LockMutex(messageMutex);
      while(messageHead == NULL){
          SDL_WaitCondition(messageCondition, messageMutex);
      }
      messageQueue *current = messageHead;
      messageHead = messageHead->next;
      if(messageHead == NULL){
          messageTail = NULL;
      }
      SDL_UnlockMutex(messageMutex);
      if(current->type == 1){
        char name[256];
        int cpu;
        int ram;
        int space;
        int storage;
        int time;
        if(sscanf(current->message, "DeviceInfo|%255[^|]|%d|%d|%d|%d|%d", name, &cpu, &ram, &space, &storage) == 5){
          SDL_LockMutex(screenMutex);
          if(getDevice(name)){
              editDevice(name, cpu, ram, space, storage);
          }
          else{
              addDevice(name, cpu, ram, space, storage);
          }
          SDL_UnlockMutex(screenMutex);
        }
      }
      else if(current->type == 2){

      }
      free(current->message);
      free(current);
    }
    return 0;
}

int logAnalyzer(void *data){

  return 0;
}