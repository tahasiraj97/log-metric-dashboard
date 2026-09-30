#!/bin/bash
set -e
docker compose exec -T dashboard sh -c "gcc /src/*.c -o /tmp/$1 \$(pkg-config --cflags --libs sdl3 sdl3-ttf fontconfig)"
docker compose cp "dashboard:/tmp/$1" "build/dashboard/$1"
chmod +x "build/dashboard/$1"
docker compose exec -T dashboard rm -f "/tmp/$1"