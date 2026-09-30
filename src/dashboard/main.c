#include "00_dashboard.h"
#include <SDL3/SDL.h>
#include <stdlib.h>

int timeTick(void *data){
    while(true){
        SDL_LockMutex(screenMutex);
        updateTTL();
        tickCount();
        SDL_UnlockMutex(screenMutex);
        SDL_Delay(1000);
    }
    return 0;
}
int main(void){
    init();
    SDL_Thread *collectorThread = SDL_CreateThread(collector, "Collector Thread", NULL);
    SDL_Thread *analyzerThread = SDL_CreateThread(analyzer, "Analyzer Thread", NULL);
    SDL_Thread *timeThread = SDL_CreateThread(timeTick, "Time Thread", NULL);

    while(true){
        SDL_LockMutex(screenMutex);
        Devices *deviceCopy = malloc(deviceCount * sizeof(*deviceCopy));
        Logs *logCopy = malloc(logCount * sizeof(*logCopy));
        Counter counterCopy;
        for(int i = 0; i < deviceCount; i++){
            deviceCopy[i] = devices[i];
            deviceCopy[i].message = devices[i].message != NULL ? strdup(devices[i].message) : NULL;
        }

        for(int i = 0; i < logCount; i++){
            logCopy[i] = logs[i];
            logCopy[i].message = strdup(logs[i].message);
        }
        counterCopy = counter;
        SDL_UnlockMutex(screenMutex);
        tick(deviceCopy, logCopy, counterCopy);
        for(int i = 0; i < deviceCount; i++){
            free(deviceCopy[i].message);
        }

        for(int i = 0; i < logCount; i++){
            free(logCopy[i].message);
        }
        free(deviceCopy);
        free(logCopy);
        SDL_Delay(33);
    }
    return 0;
}