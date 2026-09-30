#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

int main(){
    struct utsname info;
    char command[1024];

    if(geteuid() != 0){
        printf("Run installer as root.\n");
        return 1;
    }

    uname(&info);

    system("apt-get update");
    system("apt-get install -y libsdl3-0 libsdl3-ttf0 fontconfig fonts-dejavu-core wget");
    system("mkdir -p /usr/local/lib/dashboard");

    if(strcmp(info.machine, "x86_64") == 0){
        snprintf(command, sizeof(command), "wget -O /usr/local/lib/dashboard/dashboard-bin https://raw.githubusercontent.com/tahasiraj97/log-metric-dashboard/main/build/dashboard/%s-amd64", DASHBOARD_VERSION);
    }
    else if(strcmp(info.machine, "aarch64") == 0){
        snprintf(command, sizeof(command), "wget -O /usr/local/lib/dashboard/dashboard-bin https://raw.githubusercontent.com/tahasiraj97/log-metric-dashboard/main/build/dashboard/%s-arm64", DASHBOARD_VERSION);
    }
    else{
        printf("Unsupported architecture: %s\n", info.machine);
        return 1;
    }

    system(command);
    system("chmod +x /usr/local/lib/dashboard/dashboard-bin");

    FILE *file = fopen("/usr/local/bin/dashboard", "w");

    fprintf(file,
        "#!/bin/bash\n"
        "if [ \"$1\" = \"start\" ]; then\n"
        "    DISPLAY=:0 nohup /usr/local/lib/dashboard/dashboard-bin >/dev/null 2>&1 &\n"
        "elif [ \"$1\" = \"stop\" ]; then\n"
        "    pkill -9 -f /usr/local/lib/dashboard/dashboard-bin\n"
        "else\n"
        "    echo \"Usage: dashboard start|stop\"\n"
        "fi\n"
    );

    fclose(file);

    system("chmod +x /usr/local/bin/dashboard");

    printf("Dashboard %s installed.\n", DASHBOARD_VERSION);

    return 0;
}