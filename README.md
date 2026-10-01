
Log Metric Dashboard
Log Metric Dashboard is a lightweight Linux monitoring project written in C. It consists of a central SDL3 dashboard and a small endpoint agent that reports system metrics and SSH authentication activity over UDP.

Overview
The dashboard is designed for a small homelab or server environment where multiple Linux devices need to be monitored from one screen.

Each agent reports:

Hostname
CPU usage
RAM usage
Used and total storage
Online/offline status
Successful SSH logins
Failed SSH login attempts
Source IP address for SSH events
The dashboard listens for agent traffic on UDP port 5500.

Supported Architectures
Prebuilt binaries are available for:

AMD64 / x86_64
ARM64 / aarch64
ARM64 builds can be used on devices such as a Raspberry Pi 5.

Dashboard Requirements
The dashboard requires Linux with SDL3, SDL3_ttf, Fontconfig, and DejaVu fonts.

On Debian-based systems, the dashboard installer installs the required runtime packages automatically.

The installed dashboard binary is located at /usr/local/lib/dashboard/dashboard-bin.

The dashboard command is installed at /usr/local/bin/dashboard.

Installing the Dashboard
ARM64
Download the installer with wget -O installer-arm64 https://raw.githubusercontent.com/tahasiraj97/log-metric-dashboard/main/build/dashboard/installer-arm64
Make it executable with chmod +x installer-arm64
Run it with sudo ./installer-arm64
AMD64
Download the installer with wget -O installer-amd64 https://raw.githubusercontent.com/tahasiraj97/log-metric-dashboard/main/build/dashboard/installer-amd64
Make it executable with chmod +x installer-amd64
Run it with sudo ./installer-amd64
After installation, start the dashboard with dashboard start.

Stop it with dashboard stop.

Agent Requirements
The agent is designed for Linux systems using:

/proc/stat
/proc/meminfo
statvfs()
pthreads
UDP sockets
systemd
journalctl
ssh.service
The agent does not require SDL or any graphical libraries.

The agent should normally be installed as root because it creates files under /usr/local and reads SSH events from the system journal.

The installed agent binary is located at /usr/local/lib/agent/agent-bin.

The agent command is installed at /usr/local/bin/agent.

Installing the Agent
ARM64
Remove an older downloaded installer with rm -f agent-installer-V0.1-arm64
Download the current build with wget -O agent-installer-V0.1-arm64 https://raw.githubusercontent.com/tahasiraj97/log-metric-dashboard/main/build/agent/agent-installer-V0.1-arm64
Make it executable with chmod +x agent-installer-V0.1-arm64
Run it with sudo ./agent-installer-V0.1-arm64
Enter the dashboard IP when prompted
Remove the downloaded installer with rm -f agent-installer-V0.1-arm64
AMD64
Remove an older downloaded installer with rm -f agent-installer-V0.1-amd64
Download the current build with wget -O agent-installer-V0.1-amd64 https://raw.githubusercontent.com/tahasiraj97/log-metric-dashboard/main/build/agent/agent-installer-V0.1-amd64
Make it executable with chmod +x agent-installer-V0.1-amd64
Run it with ./agent-installer-V0.1-amd64 if already root, or sudo ./agent-installer-V0.1-amd64 otherwise
Enter the dashboard IP when prompted
Remove the downloaded installer with rm -f agent-installer-V0.1-amd64
Agent Commands
Start the installed agent with agent start.

The agent asks for the dashboard IP, then continues running in the background.

Stop the agent with agent stop.

To check whether the installed agent is running, use pgrep -af agent-bin.

Dashboard Commands
Start the dashboard with dashboard start.

Stop the dashboard with dashboard stop.

Network Requirements
Agents communicate with the dashboard over UDP port 5500.

Each endpoint must be able to reach the dashboard IP on that port.

A firewall between the agent and dashboard must allow UDP traffic to port 5500.

Device Metrics
The agent sends device information once per second.

CPU usage is calculated from /proc/stat.

RAM usage is calculated using MemTotal and MemAvailable from /proc/meminfo.

Storage usage is collected with statvfs("/").

The device packet format is DeviceInfo|hostname|cpu|ram|usedStorage|totalStorage.

An example packet is DeviceInfo|security-pi|4|31|18|236.

SSH Monitoring
The agent waits until ssh.service is active and then follows SSH events using journalctl -f -n 0 -u ssh.service -o cat.

Successful and failed SSH authentication events are sent to the dashboard with the device hostname, source IP, and timestamp.

A successful event looks like Log|(security-pi) Successful Login Attempt by: 192.168.8.101|Oct 01 26 00:24:06.

A failed event looks like Log|(security-pi) Failed Login Attempt by: 192.168.8.101|Oct 01 26 00:24:06.

Timestamps use the format Mon DD YY HH:MM:SS.

Building From Source
Build agent version 0.1 by running chmod +x build-agent.sh and then ./build-agent.sh 0.1.

This produces:

build/agent/agent-installer-V0.1-amd64
build/agent/agent-installer-V0.1-arm64
Build dashboard version dashboardv0.1 by running chmod +x build-dashboard.sh and then ./build-dashboard.sh dashboardv0.1.

This produces:

build/dashboard/dashboardv0.1-amd64
build/dashboard/dashboardv0.1-arm64
build/dashboard/installer-amd64
build/dashboard/installer-arm64
Project Structure
The main source directories are:

src/dashboard — SDL3 dashboard source
src/agent — Linux monitoring agent
src/install — dashboard installer
build/dashboard — compiled dashboard binaries and installers
build/agent — compiled agent binaries
build-dashboard.sh — dashboard build script
build-agent.sh — agent build script
compose.yaml — development environment
Installed Files

Dashboard:

/usr/local/lib/dashboard/dashboard-bin
/usr/local/bin/dashboard
Agent:

/usr/local/lib/agent/agent-bin
/usr/local/bin/agent
Repository
https://github.com/tahasiraj97/log-metric-dashboard