#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

int main(void){
    int socketFD = socket(AF_INET, SOCK_DGRAM, 0);

    struct addrinfo hints = {0};
    struct addrinfo *collector;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    getaddrinfo("dashboard", "5500", &hints, &collector);

    srand(time(NULL));

    while(1){
        char device1[256];
        char device2[256];
        char logMessage[256];
        char logTime[16];

        const char *messages[] = {
            "Failed",
            "Success",
            "Something"
        };

        time_t currentTime = time(NULL);
        struct tm *localTime = localtime(&currentTime);

        strftime(logTime, sizeof(logTime), "%H:%M:%S", localTime);

        snprintf(device1, sizeof(device1), "DeviceInfo|Server-01|%d|%d|40|100", rand() % 101, rand() % 101);
        snprintf(device2, sizeof(device2), "DeviceInfo|Server-02|%d|%d|40|100", rand() % 101, rand() % 101);
        snprintf(logMessage, sizeof(logMessage), "Log|%s|%s", messages[rand() % 3], logTime);

        sendto(socketFD, device1, strlen(device1), 0, collector->ai_addr, collector->ai_addrlen);
        sendto(socketFD, device2, strlen(device2), 0, collector->ai_addr, collector->ai_addrlen);
        sendto(socketFD, logMessage, strlen(logMessage), 0, collector->ai_addr, collector->ai_addrlen);

        sleep(1);
    }

    freeaddrinfo(collector);
    close(socketFD);

    return 0;
}