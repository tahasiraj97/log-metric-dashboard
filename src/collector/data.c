#include "00_dashboard.h"
#include <stdlib.h>
#include <string.h>

Logs *logs;
Devices *devices;
Counter counter;

int deviceCount;
int logCount;
int deviceSize;
int countSize;
int logSize;

void addCount(int hourly, int daily, int weekly){
    counter.hourly = hourly;
    counter.daily = daily;
    counter.weekly = weekly;
}

void addDevice(char *message, int cpu, int ram, int space, int storage, int time, bool status, int id){
    for(int i = 0; i < deviceCount; i++){
        if(devices[i].id == -1){
            devices[i].id = id;
            devices[i].message = message;
            devices[i].cpu = cpu;
            devices[i].ram = ram;
            devices[i].space = space;
            devices[i].storage = storage;
            devices[i].time = time;
            devices[i].status = status;
            break;
        }
    }
}
void addLog(char *message, const char *time, int severity){
    int emptylog;
    for(int i = 0; i < logCount; i++){
        if(logs[i].message[0] == '\0' || i == logCount - 1){
            emptylog = i;
            break;
        }
    }
    for(int i = emptylog; i > 0; i--){
        strcpy(logs[i].message, logs[i - 1].message);
        strcpy(logs[i].time, logs[i - 1].time);
        logs[i].severity = logs[i - 1].severity;
    }
    if(strlen(message) > logSize - 16){
        message[logSize - 16] = '\0';
    }
    strcpy(logs[0].message, message);
    strcpy(logs[0].time, time);
    logs[0].severity = severity;
}