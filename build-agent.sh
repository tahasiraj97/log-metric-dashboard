#!/bin/bash

gcc src/agent/main.c -o "build/agent/agent-installer-V$1-amd64" -pthread
aarch64-linux-gnu-gcc src/agent/main.c -o "build/agent/agent-installer-V$1-arm64" -pthread

chmod +x "build/agent/agent-installer-V$1-amd64"
chmod +x "build/agent/agent-installer-V$1-arm64"