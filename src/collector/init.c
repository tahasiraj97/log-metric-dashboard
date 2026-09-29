#include "00_dashboard.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <fontconfig/fontconfig.h>
#include <stdlib.h>

SDL_Window *window;
SDL_Renderer *renderer;

SDL_Mutex *messageMutex;
SDL_Mutex *screenMutex;

TTF_Font *font1;
TTF_Font *font2;
TTF_Font *font3;
TTF_Font *font4;
TTF_Font *font5;
TTF_TextEngine *textEngine;

messageQueue *messageHead = NULL;
messageQueue *messageTail = NULL;
screenQueue *screenHead = NULL;
screenQueue *screenTail = NULL;
SDL_Condition *messageCondition;

static TTF_Font *loadFont(const char *name, float size){
    FcPattern *pattern = FcNameParse((const FcChar8 *)name);
    FcConfigSubstitute(NULL, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);
    FcResult result;
    FcPattern *match = FcFontMatch(NULL, pattern, &result);
    FcChar8 *fontPath;
    FcPatternGetString(match, FC_FILE, 0, &fontPath);
    TTF_Font *loadedFont = TTF_OpenFont((const char *)fontPath, size);
    FcPatternDestroy(match);
    FcPatternDestroy(pattern);
    return loadedFont;
}

void init(){
    int width, height;
    float middle;

    messageMutex = SDL_CreateMutex();
    screenMutex = SDL_CreateMutex();
    messageCondition = SDL_CreateCondition();

    SDL_Init(SDL_INIT_VIDEO);
    SDL_DisplayID display = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode *displayMode = SDL_GetCurrentDisplayMode(display);
    SDL_CreateWindowAndRenderer("Security GUI", displayMode->w, displayMode->h, 0, &window, &renderer);
    SDL_GetRenderOutputSize(renderer, &width, &height);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    TTF_Init();
    FcInit();
    font1 = loadFont("DejaVu Sans:style=Bold", 18);
    font2 = loadFont("DejaVu Sans", 17);
    font3 = loadFont("DejaVu Sans:style=Bold", 12);
    font4 = loadFont("DejaVu Sans", 15);
    font5 = loadFont("DejaVu Sans:style=Bold", 17);
    textEngine = TTF_CreateRendererTextEngine(renderer);


    middle = width / 2.0f;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderLine(renderer, middle, 0, middle, height);

    int deviceRowHeight = 40;
    int deviceCellCount = height / deviceRowHeight;
    deviceCount = deviceCellCount / 2;
    int deviceRemainder = height % deviceRowHeight;
    float deviceCellHeight = deviceRowHeight + ((float)deviceRemainder / deviceCellCount);
    deviceSize = (middle - 20) / 8;
    devices = malloc(deviceCount * sizeof(*devices));
    for(int i = 0; i < deviceCount; i++){
        devices[i].top.x = 0;
        devices[i].top.y = (i * 2) * deviceCellHeight;
        devices[i].top.w = middle;
        devices[i].top.h = deviceCellHeight;
        devices[i].bottom.x = 0;
        devices[i].bottom.y = ((i * 2) + 1) * deviceCellHeight;
        devices[i].bottom.w = middle;
        devices[i].bottom.h = deviceCellHeight;
        devices[i].ttl = 0;
        devices[i].message = NULL;
        devices[i].cpu = 0;
        devices[i].ram = 0;
        devices[i].space = 0;
        devices[i].storage = 0;
        devices[i].time = 0;
        devices[i].status = false;
        
    }

    float counterHeight = 35;
    int logRowHeight = 28;
    float logHeight = height - counterHeight;
    logCount = logHeight / logRowHeight;
    int logRemainder = (int)logHeight % logRowHeight;
    counterHeight += logRemainder;
    counter.rect.x = middle;
    counter.rect.y = 0;
    counter.rect.w = width - middle;
    counter.rect.h = counterHeight;
    counter.hourly = 0;
    counter.daily = 0;
    counter.weekly = 0;
    countSize = ((width - middle) - 20) / 8;
    logSize = ((width - middle) - 20) / 8;
    logs = malloc(logCount * sizeof(*logs));
    for(int i = 0; i < logCount; i++){
        logs[i].rect.x = middle;
        logs[i].rect.y = counterHeight + (i * logRowHeight);
        logs[i].rect.w = width - middle;
        logs[i].rect.h = logRowHeight;
        logs[i].message = malloc(logSize + 1 - 16);
        logs[i].message[0] = '\0';
        logs[i].time[0] = '\0';
        logs[i].severity = 0;
    }

    SDL_SetRenderDrawColor(renderer, 210, 210, 210, 255);
    SDL_RenderFillRect(renderer, &counter.rect);
    SDL_FRect counterBottomLine = {counter.rect.x, counter.rect.y + counter.rect.h - 3, counter.rect.w, 3};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &counterBottomLine);
    SDL_RenderLine(renderer, middle, 0, middle, height);
    SDL_RenderPresent(renderer);
}