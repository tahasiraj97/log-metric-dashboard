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

void addCount(){
    counter.hourly++;
}

void tickCount(){
    counter.htime++;
    if(counter.htime >= 3600){
        counter.daily[counter.dtime] = counter.hourly;
        counter.hourly = 0;
        counter.htime = 0;
        counter.dtime++;
        if(counter.dtime >= 24){
            int total = 0;
            for(int i = 0; i < 24; i++){
                total += counter.daily[i];
            }
            for(int i = 0; i < 6; i++){
                counter.weekly[i] = counter.weekly[i + 1];
            }
            counter.weekly[6] = total;
            for(int i = 0; i < 24; i++){
                counter.daily[i] = 0;
            }
            counter.dtime = 0;
        }
    }
}
void updateTTL(){
    for(int i = 0; i < deviceCount; i++){
        if(devices[i].message == NULL){
            continue;
        }
        devices[i].ttl++;
        if(devices[i].status){
            devices[i].time++;
        }
        if(devices[i].status == true && devices[i].ttl > 3){
            devices[i].status = false;
            devices[i].time = 0;
            devices[i].ttl = 0;
        }
        else if(devices[i].status == false && devices[i].ttl >= 604800){
            free(devices[i].message);
            devices[i].message = NULL;
            devices[i].cpu = 0;
            devices[i].ram = 0;
            devices[i].space = 0;
            devices[i].storage = 0;
            devices[i].time = 0;
            devices[i].ttl = 0;
        }
    }
}
void addDevice(char *message, int cpu, int ram, int space, int storage){
    for(int i = 0; i < deviceCount; i++){
        if(devices[i].message == NULL){
            devices[i].message = strdup(message);
            devices[i].cpu = cpu;
            devices[i].ram = ram;
            devices[i].space = space;
            devices[i].storage = storage;
            devices[i].time = 0;
            devices[i].status = true;
            devices[i].ttl = 0;
            break;
        }
    }
}
void editDevice(char *message, int cpu, int ram, int space, int storage){
    for(int i = 0; i < deviceCount; i++){
        if(devices[i].message != NULL && strcmp(devices[i].message, message) == 0){
            devices[i].cpu = cpu;
            devices[i].ram = ram;
            devices[i].space = space;
            devices[i].storage = storage;
            devices[i].ttl = 0;
            if(devices[i].status == false){
                devices[i].status = true;
                devices[i].time = 0;
            }
            break;
        }
    }
}
bool getDevice(char *name){
    for(int i = 0; i < deviceCount; i++){
        if(devices[i].message != NULL && strcmp(devices[i].message, name) == 0){
            return true;
        }
    }
    return false;
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