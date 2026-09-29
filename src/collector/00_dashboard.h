#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdbool.h>

typedef struct {
    SDL_FRect rect;
    char *message;
    char time[16];
    int severity;
} Logs;

typedef struct {
    SDL_FRect top;
    SDL_FRect bottom;
    char *message;
    int cpu;
    int ram;
    int space;
    int storage;
    int time;
    bool status;
    int ttl;
} Devices;

typedef struct {
    SDL_FRect rect;
    int hourly;
    int daily;
    int weekly;
} Counter;

typedef struct screenQueue {
    int type;
    char *message;
    struct screenQueue *next;
} screenQueue;

typedef struct messageQueue {
    int type;
    char *message;
    struct messageQueue *next;
} messageQueue;

extern messageQueue *messageHead;
extern messageQueue *messageTail;
extern screenQueue *screenHead;
extern screenQueue *screenTail;

extern SDL_Mutex *messageMutex;
extern SDL_Mutex *screenMutex;
extern SDL_Condition *messageCondition;

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern TTF_Font *font1;
extern TTF_Font *font2;
extern TTF_Font *font3;
extern TTF_Font *font4;
extern TTF_Font *font5;
extern TTF_TextEngine *textEngine;

extern Logs *logs;
extern Devices *devices;
extern Counter counter;

extern int deviceCount;
extern int logCount;
extern int deviceSize;
extern int countSize;
extern int logSize;

int collector(void *data);
int analyzer(void *data);

void init();

void updateTTL();
void addDevice(char *message, int cpu, int ram, int space, int storage);
void editDevice(char *message, int cpu, int ram, int space, int storage);
bool getDevice(char *name);
void addLog(char *message, const char *time, int severity);
void addCount(int hourly, int daily, int weekly);

void tick(Devices *devices, Logs *logs, Counter counter);

#endif