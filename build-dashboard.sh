#!/bin/bash

docker compose exec -T dashboard sh -c "gcc /src/*.c -o /tmp/$1-amd64 \$(pkg-config --cflags --libs sdl3 sdl3-ttf fontconfig)"
docker compose cp "dashboard:/tmp/$1-amd64" "build/dashboard/$1-amd64"
chmod +x "build/dashboard/$1-amd64"

docker compose exec -T -e PKG_CONFIG_LIBDIR=/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig dashboard sh -c "aarch64-linux-gnu-gcc /src/*.c -o /tmp/$1-arm64 \$(pkg-config --cflags --libs sdl3 sdl3-ttf fontconfig)"
docker compose cp "dashboard:/tmp/$1-arm64" "build/dashboard/$1-arm64"
chmod +x "build/dashboard/$1-arm64"

gcc -DDASHBOARD_VERSION="\"$1\"" src/install/main.c -o build/dashboard/installer-amd64
aarch64-linux-gnu-gcc -DDASHBOARD_VERSION="\"$1\"" src/install/main.c -o build/dashboard/installer-arm64

chmod +x build/dashboard/installer-amd64
chmod +x build/dashboard/installer-arm64