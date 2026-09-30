#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <utmpx.h>
#include <time.h>

void *deviceInfo(void *data){
    int socketFD = socket(AF_INET, SOCK_DGRAM, 0);

    struct addrinfo hints = {0};
    struct addrinfo *dashboard;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    getaddrinfo("dashboard", "5500", &hints, &dashboard);

    char hostname[256];
    gethostname(hostname, sizeof(hostname));

    static unsigned long long previousIdle = 0;
    static unsigned long long previousTotal = 0;

    while(1){
        char message[256];

        FILE *cpuFile = fopen("/proc/stat", "r");

        unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;

        fscanf(cpuFile, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
            &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);

        fclose(cpuFile);

        unsigned long long currentIdle = idle + iowait;
        unsigned long long currentTotal = user + nice + system + idle + iowait + irq + softirq + steal;

        int cpu = 0;

        if(previousTotal != 0){
            unsigned long long totalDifference = currentTotal - previousTotal;
            unsigned long long idleDifference = currentIdle - previousIdle;

            cpu = 100 * (totalDifference - idleDifference) / totalDifference;
        }

        previousIdle = currentIdle;
        previousTotal = currentTotal;

        FILE *ramFile = fopen("/proc/meminfo", "r");

        char name[64];
        unsigned long value;
        unsigned long ramTotal = 0;
        unsigned long ramAvailable = 0;

        while(fscanf(ramFile, "%63s %lu kB\n", name, &value) == 2){
            if(strcmp(name, "MemTotal:") == 0){
                ramTotal = value;
            }
            else if(strcmp(name, "MemAvailable:") == 0){
                ramAvailable = value;
            }
        }

        fclose(ramFile);

        int ram = ((ramTotal - ramAvailable) * 100) / ramTotal;

        struct statvfs disk;
        statvfs("/", &disk);

        unsigned long long total = ((unsigned long long)disk.f_blocks * disk.f_frsize) / 1073741824;
        unsigned long long used = ((unsigned long long)(disk.f_blocks - disk.f_bfree) * disk.f_frsize) / 1073741824;

        snprintf(message, sizeof(message), "DeviceInfo|%s|%d|%d|%llu|%llu", hostname, cpu, ram, used, total);

        sendto(socketFD, message, strlen(message), 0, dashboard->ai_addr, dashboard->ai_addrlen);

        sleep(1);
    }

    freeaddrinfo(dashboard);
    close(socketFD);

    return NULL;
}

void *logs(void *data){
    int socketFD = socket(AF_INET, SOCK_DGRAM, 0);

    struct addrinfo hints = {0};
    struct addrinfo *dashboard;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    getaddrinfo("dashboard", "5500", &hints, &dashboard);

    pid_t sessions[256] = {0};

    while(1){
        struct utmpx *entry;
        setutxent();
        while((entry = getutxent()) != NULL){
            if(entry->ut_type == USER_PROCESS && entry->ut_host[0] != '\0'){
                int exists = 0;
                for(int i = 0; i < 256; i++){
                    if(sessions[i] == entry->ut_pid){
                        exists = 1;
                        break;
                    }
                }
                if(!exists){
                    for(int i = 0; i < 256; i++){
                        if(sessions[i] == 0){
                            sessions[i] = entry->ut_pid;
                            break;
                        }
                    }
                    char logMessage[512];
                    char logTime[16];
                    time_t loginTime = entry->ut_tv.tv_sec;
                    struct tm *localTime = localtime(&loginTime);
                    strftime(logTime, sizeof(logTime), "%H:%M:%S", localTime);

                    snprintf(logMessage, sizeof(logMessage), "Log|SSH Login: %s - %s|%s", entry->ut_user, entry->ut_host, logTime);

                    sendto(socketFD, logMessage, strlen(logMessage), 0, dashboard->ai_addr, dashboard->ai_addrlen);
                }
            }
        }
        endutxent();
        sleep(1);
    }
    freeaddrinfo(dashboard);
    close(socketFD);
    return NULL;
}

int main(void){
    pthread_t deviceThread;
    pthread_t logThread;
    pthread_create(&deviceThread, NULL, deviceInfo, NULL);
    pthread_create(&logThread, NULL, logs, NULL);
    pthread_join(deviceThread, NULL);
    pthread_join(logThread, NULL);
    return 0;
}