#include "00_dashboard.h"
#include <sys/socket.h>
#include <netinet/in.h>

struct sockaddr_in address = {
    .sin_family = AF_INET,
    .sin_addr.s_addr = INADDR_ANY
};

char message[65508];

int collector(void *data){
    int socketFD = socket(AF_INET, SOCK_DGRAM, 0);
    address.sin_port = htons(5500);
    bind(socketFD, (struct sockaddr *)&address, sizeof(address));

    while(true){
        recv(socketFD, message, sizeof(message), 0);
    }

    return 0;
}