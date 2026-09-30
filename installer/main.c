#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

int main(){
    struct utsname info;
    if(geteuid() != 0){
        printf("Run installer as root.\n");
        return 1;
    }
    uname(&info);
    system("apt-get update");
    system("apt-get install -y libsdl3-0 libsdl3-ttf0 fontconfig fonts-dejavu-core wget");
    if(strcmp(info.machine, "x86_64") == 0){
        system("wget -O /usr/local/bin/dashboard https://raw.githubusercontent.com/tahasiraj97/log-metric-dashboard/main/build/dashboard/dashboardv0.1-amd64");
    }
    else if(strcmp(info.machine, "aarch64") == 0){
        system("wget -O /usr/local/bin/dashboard https://raw.githubusercontent.com/tahasiraj97/log-metric-dashboard/main/build/dashboard/dashboardv0.1-arm64");
    }
    else{
        printf("Unsupported architecture: %s\n", info.machine);
        return 1;
    }
    system("chmod +x /usr/local/bin/dashboard");
    printf("Dashboard installed to /usr/local/bin/dashboard\n");
    return 0;
}