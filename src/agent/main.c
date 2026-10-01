#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include <stdlib.h>

void *deviceInfo(void *data){
    FILE *file;
    struct addrinfo *dashboard;
    struct statvfs disk;
    struct addrinfo hints = {0};
    char message[256], hostname[256], name[64];
    char *dashboardIP = data;
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, currentIdle, currentTotal, storage, space;
    unsigned long long previousIdle = 0, previousTotal = 0;
    unsigned long value, total, available;
    int cpu, ram;
    int socketFD = socket(AF_INET, SOCK_DGRAM, 0);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    getaddrinfo(dashboardIP, "5500", &hints, &dashboard);
    gethostname(hostname, sizeof(hostname));

    while(true){
        file = fopen("/proc/stat", "r");
        fscanf(file, "cpu %llu %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
        fclose(file);
        currentIdle = idle + iowait;
        currentTotal = user + nice + system + idle + iowait + irq + softirq + steal;
        cpu = 0;
        if(previousTotal != 0){
            cpu = 100 * ((currentTotal - previousTotal) - (currentIdle - previousIdle)) / (currentTotal - previousTotal);
        }
        previousIdle = currentIdle;
        previousTotal = currentTotal;

        file = fopen("/proc/meminfo", "r");
        total = 0;
        available = 0;
        while(fscanf(file, "%63s %lu kB\n", name, &value) == 2){
            if(strcmp(name, "MemTotal:") == 0){
                total = value;
            }
            else if(strcmp(name, "MemAvailable:") == 0){
                available = value;
            }
        }
        fclose(file);
        ram = ((total - available) * 100) / total;
        statvfs("/", &disk);
        storage = ((unsigned long long)disk.f_blocks * disk.f_frsize) / 1073741824;
        space = ((unsigned long long)(disk.f_blocks - disk.f_bfree) * disk.f_frsize) / 1073741824;
        snprintf(message, sizeof(message), "DeviceInfo|%s|%d|%d|%llu|%llu", hostname, cpu, ram, space, storage);
        sendto(socketFD, message, strlen(message), 0, dashboard->ai_addr, dashboard->ai_addrlen);
        sleep(1);
    }
    freeaddrinfo(dashboard);
    close(socketFD);

    return NULL;
}

void *logs(void *data){
    FILE *file;
    struct addrinfo *dashboard;
    struct addrinfo hints = {0};
    struct tm *localTime;
    char line[1024], message[512], logTime[64], ip[64], hostname[256], *from;
    char *dashboardIP = data;
    time_t now;
    int socketFD = socket(AF_INET, SOCK_DGRAM, 0);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    getaddrinfo(dashboardIP, "5500", &hints, &dashboard);
    gethostname(hostname, sizeof(hostname));

    file = popen("journalctl -f -n 0 -u ssh.service -o cat", "r");
    while(fgets(line, sizeof(line), file) != NULL){
        if(strstr(line, "Failed password") != NULL || strstr(line, "Failed publickey") != NULL){
            from = strstr(line, " from ");
            if(from != NULL){
                sscanf(from + 6, "%63s", ip);
                now = time(NULL);
                localTime = localtime(&now);
                strftime(logTime, sizeof(logTime), "%B %d %Y %H:%M", localTime);

                snprintf(message, sizeof(message), "Log|(%s) Failed Login Attempt by: %s|%s", hostname, ip, logTime);
                sendto(socketFD, message, strlen(message), 0, dashboard->ai_addr, dashboard->ai_addrlen);
            }
        }
        else if(strstr(line, "Accepted password") != NULL || strstr(line, "Accepted publickey") != NULL){
            from = strstr(line, " from ");
            if(from != NULL){
                sscanf(from + 6, "%63s", ip);
                now = time(NULL);
                localTime = localtime(&now);
                strftime(logTime, sizeof(logTime), "%B %d %Y %H:%M", localTime);
                snprintf(message, sizeof(message), "Log|(%s) Successful Login Attempt by: %s|%s", hostname, ip, logTime);
                sendto(socketFD, message, strlen(message), 0, dashboard->ai_addr, dashboard->ai_addrlen);
            }
        }
    }
    pclose(file);
    freeaddrinfo(dashboard);
    close(socketFD);
    return NULL;
}

int main(void){
    FILE *file;
    char dashboardIP[64];
    printf("Dashboard IP: ");
    scanf("%63s", dashboardIP);
    pthread_t deviceThread, logThread;
    file = fopen("/usr/local/bin/agent", "w");
    fprintf(file,
        "#!/bin/bash\n"
        "if [ \"$1\" = \"start\" ]; then\n"
        "    nohup /usr/local/lib/agent/agent-bin >/dev/null 2>&1 &\n"
        "elif [ \"$1\" = \"stop\" ]; then\n"
        "    pkill -9 -f /usr/local/lib/agent/agent-bin\n"
        "else\n"
        "    echo \"Usage: agent start|stop\"\n"
        "fi\n"
    );
    fclose(file);
    system("chmod +x /usr/local/bin/agent");
    pthread_create(&deviceThread, NULL, deviceInfo, dashboardIP);

    while(system("systemctl is-active --quiet ssh.service") != 0){
        sleep(1);
    }

    pthread_create(&logThread, NULL, logs, dashboardIP);
    pthread_join(deviceThread, NULL);
    pthread_join(logThread, NULL);
    return 0;
}