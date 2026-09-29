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

        snprintf(device1, sizeof(device1), "DeviceInfo|Server-01|%d|%d|40|100", rand() % 101, rand() % 101);
        snprintf(device2, sizeof(device2), "DeviceInfo|Server-02|%d|%d|40|100", rand() % 101, rand() % 101);

        sendto(socketFD, device1, strlen(device1), 0, collector->ai_addr, collector->ai_addrlen);
        sendto(socketFD, device2, strlen(device2), 0, collector->ai_addr, collector->ai_addrlen);

        sleep(5);
    }

    freeaddrinfo(collector);
    close(socketFD);

    return 0;
}