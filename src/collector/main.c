#include <SDL3/SDL.h>
#include "00_dashboard.h"


int analyzer(void *data){
    return 0;
}

int main(void){
    init();
    SDL_Thread *collectorThread = SDL_CreateThread(collector, "Collector Thread", NULL);
    SDL_Thread *analyzerThread = SDL_CreateThread(analyzer, "Analyzer Thread", NULL);

    while(true){
        SDL_Delay(1);
    }

    return 0;
}