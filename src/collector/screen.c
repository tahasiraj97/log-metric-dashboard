#include "00_dashboard.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <string.h>
#include <stdio.h>

void drawLogs(Logs *logs){
    int width, height;
    SDL_GetRenderOutputSize(renderer, &width, &height);
    SDL_FRect logArea = {logs[0].rect.x + 1, logs[0].rect.y, logs[0].rect.w - 1, height - logs[0].rect.y};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &logArea);
    for(int i = 0; i < logCount; i++){
        if(logs[i].message[0] == '\0'){
            continue;
        }
        int messageLength = strlen(logs[i].message);
        int timeLength = strlen(logs[i].time);
        int dotCount = logSize - messageLength - timeLength;
        char line[logSize + 1];
        int position = 0;
        strcpy(line, logs[i].message);
        position += messageLength;
        for(int j = 0; j < dotCount; j++){
            line[position++] = '.';
        }
        strcpy(line + position, logs[i].time);
        if(logs[i].severity == 1){
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        }
        else if(logs[i].severity == 2){
            SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255);
        }
        else if(logs[i].severity == 3){
            SDL_SetRenderDrawColor(renderer, 0, 180, 0, 255);
        }
        else{
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        }

        float x = logs[i].rect.x + 10;
        float y = logs[i].rect.y + ((logs[i].rect.h - 8) / 2.0f);

        SDL_RenderDebugText(renderer, x, y, line);
    }
}

void drawDevices(Devices *devices){
    SDL_FRect deviceArea = {  devices[0].top.x, devices[0].top.y, devices[0].top.w, devices[deviceCount - 1].bottom.y + devices[deviceCount - 1].bottom.h };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &deviceArea);

    for(int i = 0; i < deviceCount; i++){
        if(devices[i].message == NULL){
            continue;
        }
        int textWidth;
        int cpuWidth;
        int ramWidth;
        int statusWidth;

        TTF_Text *deviceName = TTF_CreateText(textEngine, font1, devices[i].message, 0);
        TTF_SetTextColor(deviceName, 0, 0, 0, 255);
        TTF_DrawRendererText( deviceName,devices[i].top.x + 10, devices[i].top.y + 10 );
        TTF_DestroyText(deviceName);

        TTF_Text *cpuTitle = TTF_CreateText(textEngine, font2, "CPU", 0);
        TTF_GetTextSize(cpuTitle, &cpuWidth, NULL);
        TTF_SetTextColor(cpuTitle, 70, 70, 70, 255);
        TTF_DrawRendererText(cpuTitle, devices[i].top.x + (devices[i].top.w * 0.24f), devices[i].top.y + 10);
        TTF_DestroyText(cpuTitle);

        TTF_Text *ramTitle = TTF_CreateText(textEngine, font2, "RAM", 0);
        TTF_GetTextSize(ramTitle, &ramWidth, NULL);
        TTF_SetTextColor(ramTitle, 70, 70, 70, 255);
        TTF_DrawRendererText(ramTitle, devices[i].top.x + (devices[i].top.w * 0.24f) + cpuWidth + 55, devices[i].top.y + 10);
        TTF_DestroyText(ramTitle);

        TTF_Text *storageTitle = TTF_CreateText(textEngine, font2, "SPACE LEFT", 0);
        TTF_SetTextColor(storageTitle, 70, 70, 70, 255);
        TTF_DrawRendererText(storageTitle, devices[i].top.x + (devices[i].top.w * 0.24f) + cpuWidth + 55 + ramWidth + 55, devices[i].top.y + 10);
        TTF_DestroyText(storageTitle);

        TTF_Text *statusTitle = TTF_CreateText(textEngine, font2, "STATUS", 0);
        TTF_GetTextSize(statusTitle, &statusWidth, NULL);
        TTF_SetTextColor(statusTitle, 70, 70, 70, 255);
        TTF_DrawRendererText(statusTitle, devices[i].top.x + devices[i].top.w - statusWidth - 10, devices[i].top.y + 10);
        TTF_DestroyText(statusTitle);

        char cpuText[5];
        char ramText[5];
        char spaceText[12];
        char timeText[12];

        snprintf(timeText, sizeof(timeText), "%d:%02d:%02d", devices[i].time / 3600, (devices[i].time % 3600) / 60, devices[i].time % 60);
        if(devices[i].status){
            snprintf(cpuText, sizeof(cpuText), "%d%%", devices[i].cpu);
            snprintf(ramText, sizeof(ramText), "%d%%", devices[i].ram);
            snprintf(spaceText, sizeof(spaceText), "%dG", devices[i].storage - devices[i].space);
        }
        else{
            strcpy(cpuText, "N/A");
            strcpy(ramText, "N/A");
            strcpy(spaceText, "N/A");
        }

        TTF_Text *cpuValue = TTF_CreateText(textEngine, font2, cpuText, 0);
        if(!devices[i].status || devices[i].cpu < 50){
            TTF_SetTextColor(cpuValue, 0, 0, 0, 255);
        }
        else if(devices[i].cpu < 75){
            TTF_SetTextColor(cpuValue, 255, 200, 0, 255);
        }
        else{
            TTF_SetTextColor(cpuValue, 255, 0, 0, 255);
        }
        TTF_DrawRendererText(cpuValue, devices[i].top.x + (devices[i].top.w * 0.24f), devices[i].bottom.y - 5);
        TTF_DestroyText(cpuValue);

        TTF_Text *ramValue = TTF_CreateText(textEngine, font2, ramText, 0);
        if(!devices[i].status || devices[i].ram < 50){
            TTF_SetTextColor(ramValue, 0, 0, 0, 255);
        }
        else if(devices[i].ram < 75){
            TTF_SetTextColor(ramValue, 255, 200, 0, 255);
        }
        else{
            TTF_SetTextColor(ramValue, 255, 0, 0, 255);
        }
        TTF_DrawRendererText(ramValue, devices[i].top.x + (devices[i].top.w * 0.24f) + cpuWidth + 55, devices[i].bottom.y - 5);
        TTF_DestroyText(ramValue);

        TTF_Text *spaceValue = TTF_CreateText(textEngine, font2, spaceText, 0);
        if(!devices[i].status || ((devices[i].space * 100) / devices[i].storage) < 75){
            TTF_SetTextColor(spaceValue, 0, 0, 0, 255);
        }
        else if(((devices[i].space * 100) / devices[i].storage) < 85){
            TTF_SetTextColor(spaceValue, 255, 200, 0, 255);
        }
        else{
            TTF_SetTextColor(spaceValue, 255, 0, 0, 255);
        }
        TTF_DrawRendererText(spaceValue, devices[i].top.x + (devices[i].top.w * 0.24f) + cpuWidth + 55 + ramWidth + 55, devices[i].bottom.y - 5);
        TTF_DestroyText(spaceValue);

        TTF_Text *statusValue = TTF_CreateText(textEngine, font2, devices[i].status ? timeText : "OFFLINE", 0);
        TTF_GetTextSize(statusValue, &textWidth, NULL);
        if(devices[i].status){
            TTF_SetTextColor(statusValue, 0, 0, 0, 255);
        }
        else{
            TTF_SetTextColor(statusValue, 255, 0, 0, 255);
        }
        TTF_DrawRendererText(statusValue, devices[i].bottom.x + devices[i].bottom.w - textWidth - 10, devices[i].bottom.y - 5);
        TTF_DestroyText(statusValue);
    }
}

void drawCount(Counter counter){
    SDL_SetRenderDrawColor(renderer, 225, 225, 225, 255);
    SDL_RenderFillRect(renderer, &counter.rect);

    int textWidth;
    int daily = counter.hourly;
    int weekly = counter.hourly;
    char hourlyText[12];
    char dailyText[12];
    char weeklyText[12];

    for(int i = 0; i < 24; i++){
        daily += counter.daily[i];
        weekly += counter.daily[i];
    }

    for(int i = 0; i < 7; i++){
        weekly += counter.weekly[i];
    }

    snprintf(hourlyText, sizeof(hourlyText), "%d", counter.hourly);
    snprintf(dailyText, sizeof(dailyText), "%d", daily);
    snprintf(weeklyText, sizeof(weeklyText), "%d", weekly);

    TTF_Text *hourlyTitle = TTF_CreateText(textEngine, font4, "HOURLY LOGS:", 0);
    TTF_GetTextSize(hourlyTitle, &textWidth, NULL);
    TTF_SetTextColor(hourlyTitle, 65, 65, 65, 255);
    TTF_DrawRendererText(hourlyTitle, counter.rect.x + 20, counter.rect.y + 8);

    TTF_Text *hourlyCount = TTF_CreateText(textEngine, font5, hourlyText, 0);
    TTF_SetTextColor(hourlyCount, 20, 20, 20, 255);
    TTF_DrawRendererText(hourlyCount, counter.rect.x + 20 + textWidth + 12, counter.rect.y + 7);

    TTF_DestroyText(hourlyTitle);
    TTF_DestroyText(hourlyCount);

    TTF_Text *dailyTitle = TTF_CreateText(textEngine, font4, "DAILY LOGS:", 0);
    TTF_GetTextSize(dailyTitle, &textWidth, NULL);
    TTF_SetTextColor(dailyTitle, 65, 65, 65, 255);
    TTF_DrawRendererText(dailyTitle, counter.rect.x + (counter.rect.w * 0.36f), counter.rect.y + 8);

    TTF_Text *dailyCount = TTF_CreateText(textEngine, font5, dailyText, 0);
    TTF_SetTextColor(dailyCount, 20, 20, 20, 255);
    TTF_DrawRendererText(dailyCount, counter.rect.x + (counter.rect.w * 0.36f) + textWidth + 12, counter.rect.y + 7);

    TTF_DestroyText(dailyTitle);
    TTF_DestroyText(dailyCount);

    TTF_Text *weeklyTitle = TTF_CreateText(textEngine, font4, "WEEKLY LOGS:", 0);
    TTF_GetTextSize(weeklyTitle, &textWidth, NULL);
    TTF_SetTextColor(weeklyTitle, 65, 65, 65, 255);
    TTF_DrawRendererText(weeklyTitle, counter.rect.x + (counter.rect.w * 0.70f), counter.rect.y + 8);

    TTF_Text *weeklyCount = TTF_CreateText(textEngine, font5, weeklyText, 0);
    TTF_SetTextColor(weeklyCount, 20, 20, 20, 255);
    TTF_DrawRendererText(weeklyCount, counter.rect.x + (counter.rect.w * 0.70f) + textWidth + 12, counter.rect.y + 7);

    TTF_DestroyText(weeklyTitle);
    TTF_DestroyText(weeklyCount);

    SDL_FRect counterBottomLine = {counter.rect.x, counter.rect.y + counter.rect.h - 3, counter.rect.w, 3};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &counterBottomLine);
}
void tick(Devices *devices, Logs *logs, Counter counter){
    drawDevices(devices);
    drawCount(counter);
    drawLogs(logs);
    SDL_RenderPresent(renderer);
}