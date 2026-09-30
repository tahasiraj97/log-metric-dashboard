#!/bin/bash
set -e
NAME="$1"
docker compose exec -T dashboard sh -c "gcc /src/*.c -o /tmp/${NAME}-amd64 \$(pkg-config --cflags --libs sdl3 sdl3-ttf fontconfig)"
docker compose cp "dashboard:/tmp/${NAME}-amd64" "build/dashboard/${NAME}-amd64"
chmod +x "build/dashboard/${NAME}-amd64"

docker compose exec -T -u root dashboard sh -c "dpkg --add-architecture arm64 && apt-get update && apt-get install -y gcc-aarch64-linux-gnu libsdl3-dev:arm64 libsdl3-ttf-dev:arm64 libfontconfig1-dev:arm64"

docker compose exec -T dashboard sh -c "PKG_CONFIG_LIBDIR=/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig aarch64-linux-gnu-gcc /src/*.c -o /tmp/${NAME}-arm64 \$(PKG_CONFIG_LIBDIR=/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig pkg-config --cflags --libs sdl3 sdl3-ttf fontconfig)"
docker compose cp "dashboard:/tmp/${NAME}-arm64" "build/dashboard/${NAME}-arm64"
chmod +x "build/dashboard/${NAME}-arm64"

docker compose exec -T dashboard rm -f "/tmp/${NAME}-amd64" "/tmp/${NAME}-arm64"