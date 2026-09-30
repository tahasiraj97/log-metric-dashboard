#!/usr/bin/env bash
# Usage: ./build-dashboard.sh <build-name>
# Example: ./build-dashboard.sh dashboardv0.1
# Output:  build/dashboard/<build-name>-amd64 and build/dashboard/<build-name>-arm64
#
# Run from the project root (the folder containing src/ and docker-compose.yml).
# To force a clean rebuild of the static libraries:
#   docker compose exec dashboard rm -rf /opt/static-amd64 /opt/static-arm64
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

# ---------- locate project ----------
if [[ -d "$SCRIPT_DIR/src/dashboard" ]]; then
    PROJECT_ROOT="$SCRIPT_DIR"
elif [[ -d "$SCRIPT_DIR/../src/dashboard" ]]; then
    PROJECT_ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd)"
else
    printf 'Could not find src/dashboard. Put this script in the project root.\n' >&2
    exit 1
fi
DASHBOARD_SOURCE="$PROJECT_ROOT/src/dashboard"

if [[ ! -f "$DASHBOARD_SOURCE/main.c" || ! -f "$DASHBOARD_SOURCE/00_dashboard.h" ]]; then
    printf 'Dashboard source files are missing from: %s\n' "$DASHBOARD_SOURCE" >&2
    exit 1
fi

# ---------- arguments ----------
if [[ "$#" -ne 1 ]]; then
    printf 'Usage: %s <build-name>\n' "$(basename -- "$0")" >&2
    printf 'Example: %s dashboardv0.1\n' "$(basename -- "$0")" >&2
    exit 2
fi
BUILD_NAME="$1"
if [[ ! "$BUILD_NAME" =~ ^[A-Za-z0-9][A-Za-z0-9._-]*$ ]]; then
    printf 'Build name may contain only letters, numbers, dots, underscores, and hyphens, and must start with a letter or number.\n' >&2
    exit 2
fi
if (("${#BUILD_NAME}" > 240)); then
    printf 'Build name is too long.\n' >&2
    exit 2
fi

# ---------- docker checks ----------
if ! command -v docker >/dev/null 2>&1; then
    printf 'Docker is required.\n' >&2
    exit 1
fi
if ! docker info >/dev/null 2>&1; then
    printf 'Docker is installed but the engine is not running or not accessible.\n' >&2
    exit 1
fi

cd -- "$PROJECT_ROOT"

if ! docker compose ps --status running --services 2>/dev/null | grep -qx 'dashboard'; then
    printf 'The "dashboard" compose service is not running. Start it with:\n  docker compose up -d dashboard\n' >&2
    exit 1
fi

OUTPUT_DIR="$PROJECT_ROOT/build/dashboard"
mkdir -p -- "$OUTPUT_DIR"

cleanup() {
    docker compose exec -T dashboard rm -f \
        "/tmp/${BUILD_NAME}-amd64" "/tmp/${BUILD_NAME}-arm64" >/dev/null 2>&1 || true
}
trap cleanup EXIT

# ---------- build inside the dashboard container (/src == ./src/dashboard) ----------
docker compose exec -T -u root dashboard bash -s -- "$BUILD_NAME" <<'EOF'
set -euo pipefail
NAME="$1"
export DEBIAN_FRONTEND=noninteractive

# ---------- build tools + headers (both architectures) ----------
dpkg --add-architecture arm64
apt-get update
apt-get install -y gcc pkg-config cmake ninja-build meson git gperf python3 \
  gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu \
  libsdl3-dev libsdl3-ttf-dev libfontconfig1-dev libfreetype-dev libexpat1-dev libbz2-dev \
  libsdl3-dev:arm64 libexpat1-dev:arm64 libbz2-dev:arm64

# Use the same library versions the dev container already runs
SDL_VER=$(pkg-config --modversion sdl3)
TTF_VER=$(pkg-config --modversion sdl3-ttf)
FT_VER=$(dpkg-query -W -f='${Version}' libfreetype6 | sed -E 's/^([0-9]+\.[0-9]+(\.[0-9]+)?).*/\1/')
FC_VER=$(dpkg-query -W -f='${Version}' libfontconfig1 | sed -E 's/^([0-9]+\.[0-9]+\.[0-9]+).*/\1/')
FT_TAG="VER-${FT_VER//./-}"
echo "Building: SDL ${SDL_VER}, SDL_ttf ${TTF_VER}, FreeType ${FT_VER}, fontconfig ${FC_VER}"

SRC=/opt/static-src
mkdir -p "$SRC"
[ -d "$SRC/SDL" ]        || git clone --depth 1 --branch "release-${SDL_VER}" https://github.com/libsdl-org/SDL.git "$SRC/SDL"
[ -d "$SRC/SDL_ttf" ]    || git clone --depth 1 --branch "release-${TTF_VER}" https://github.com/libsdl-org/SDL_ttf.git "$SRC/SDL_ttf"
[ -d "$SRC/freetype" ]   || git clone --depth 1 --branch "${FT_TAG}" https://gitlab.freedesktop.org/freetype/freetype.git "$SRC/freetype"
[ -d "$SRC/fontconfig" ] || git clone --depth 1 --branch "${FC_VER}" https://gitlab.freedesktop.org/fontconfig/fontconfig.git "$SRC/fontconfig"

# Meson cross file for arm64
cat > /tmp/arm64.ini <<INI
[binaries]
c = 'aarch64-linux-gnu-gcc'
ar = 'aarch64-linux-gnu-ar'
strip = 'aarch64-linux-gnu-strip'
pkg-config = 'pkg-config'
[host_machine]
system = 'linux'
cpu_family = 'aarch64'
cpu = 'aarch64'
endian = 'little'
INI

# "-lfoo" -> "-l:libfoo.a" (static), except core glibc libs
static_libs() {
  for f in $(cat); do
    case "$f" in
      -lm|-ldl|-lpthread|-lrt|-lc|-lgcc_s) echo "$f" ;;
      -l*) echo "-l:lib${f#-l}.a" ;;
      *)   echo "$f" ;;
    esac
  done
}

# build_arch <label> <cc> <readelf> <system-pc-dirs> <meson-cross-args> <cmake-extra-args>
build_arch() {
  local ARCH="$1" CC="$2" READELF="$3" SYS_PC="$4" MESON_X="$5" CMAKE_X="$6"
  local PREFIX="/opt/static-${ARCH}"
  local B="/tmp/build-${ARCH}"
  mkdir -p "$PREFIX" "$B"
  export PKG_CONFIG_LIBDIR="${PREFIX}/lib/pkgconfig:${SYS_PC}"

  # --- FreeType (static) ---
  if [ ! -f "${PREFIX}/.freetype" ]; then
    rm -rf "$B/ft"
    meson setup "$B/ft" "$SRC/freetype" $MESON_X --wrap-mode=nofallback \
      --prefix="$PREFIX" --libdir=lib --buildtype=release --default-library=static \
      -Dharfbuzz=disabled -Dzlib=disabled -Dpng=disabled -Dbzip2=disabled -Dbrotli=disabled
    meson install -C "$B/ft"
    touch "${PREFIX}/.freetype"
  fi

  # --- fontconfig (static, reads /etc/fonts at runtime) ---
  if [ ! -f "${PREFIX}/.fontconfig" ]; then
    rm -rf "$B/fc" "$B/fc-dest"
    meson setup "$B/fc" "$SRC/fontconfig" $MESON_X --wrap-mode=nofallback \
      --prefix="$PREFIX" --libdir=lib --sysconfdir=/etc --localstatedir=/var \
      --buildtype=release --default-library=static \
      -Ddoc=disabled -Dnls=disabled -Dtests=disabled -Dtools=disabled \
      -Dcache-build=disabled
    DESTDIR="$B/fc-dest" meson install -C "$B/fc"
    cp -a "$B/fc-dest${PREFIX}/." "$PREFIX/"
    touch "${PREFIX}/.fontconfig"
  fi

  # --- SDL3 (static + shared; the shared copy only exists so SDL_ttf's CMake
  #     find_package succeeds. The final link uses the .a files only.) ---
  if [ ! -f "${PREFIX}/.sdl" ]; then
    rm -rf "$B/sdl"
    cmake -S "$SRC/SDL" -B "$B/sdl" -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_COMPILER="$CC" $CMAKE_X \
      -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_INSTALL_LIBDIR=lib \
      -DSDL_SHARED=ON -DSDL_STATIC=ON -DSDL_TESTS=OFF -DSDL_INSTALL_TESTS=OFF
    cmake --build "$B/sdl"
    cmake --install "$B/sdl"
    touch "${PREFIX}/.sdl"
  fi

  # --- SDL3_ttf (static only, uses our FreeType) ---
  if [ ! -f "${PREFIX}/.ttf" ]; then
    rm -rf "$B/ttf"
    cmake -S "$SRC/SDL_ttf" -B "$B/ttf" -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_COMPILER="$CC" $CMAKE_X \
      -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_INSTALL_LIBDIR=lib \
      -DCMAKE_PREFIX_PATH="$PREFIX" \
      -DBUILD_SHARED_LIBS=OFF -DSDLTTF_VENDORED=OFF \
      -DSDLTTF_HARFBUZZ=OFF -DSDLTTF_PLUTOSVG=OFF -DSDLTTF_SAMPLES=OFF
    cmake --build "$B/ttf"
    cmake --install "$B/ttf"
    touch "${PREFIX}/.ttf"
  fi

  # --- the dashboard itself ---
  local CFLAGS LIBS
  CFLAGS=$(pkg-config --cflags sdl3 sdl3-ttf fontconfig)
  LIBS=$(pkg-config --static --libs sdl3-ttf sdl3 fontconfig | static_libs)
  "$CC" -O2 /src/*.c -o "/tmp/${NAME}-${ARCH}" $CFLAGS $LIBS -l:libbz2.a -static-libgcc

  echo "== ${ARCH}: shared-library dependencies left =="
  "$READELF" -d "/tmp/${NAME}-${ARCH}" | grep NEEDED || true
}

build_arch amd64 gcc readelf \
  "/usr/lib/x86_64-linux-gnu/pkgconfig:/usr/share/pkgconfig" "" ""

build_arch arm64 aarch64-linux-gnu-gcc aarch64-linux-gnu-readelf \
  "/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig" \
  "--cross-file /tmp/arm64.ini" \
  "-DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64"
EOF

# ---------- copy results to build/dashboard ----------
for ARCH in amd64 arm64; do
    docker compose cp "dashboard:/tmp/${BUILD_NAME}-${ARCH}" "$OUTPUT_DIR/${BUILD_NAME}-${ARCH}"
    chmod +x "$OUTPUT_DIR/${BUILD_NAME}-${ARCH}"
done

printf 'Done:\n  %s\n  %s\n' \
    "$OUTPUT_DIR/${BUILD_NAME}-amd64" \
    "$OUTPUT_DIR/${BUILD_NAME}-arm64"