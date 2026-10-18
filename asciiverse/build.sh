#!/bin/sh
# Builds the curses viewer into out/mandelbrot. Upstream's Makefile builds it
# as `make curses` (its default target also wants viewer.c for the GLSL
# viewer, which is not in the repository); this script builds only the curses
# program, with the compiler and curses library from the environment, so one
# script serves every target (see .github/workflows/release.yml):
#
#   asciiverse/build.sh                                         # Linux
#   LIBS="-lncursesw -lm" LDFLAGS="-static -s" asciiverse/build.sh   # Alpine, static
#   CFLAGS="-O2 -arch arm64" LDFLAGS="-arch arm64" LIBS="-lncurses" asciiverse/build.sh  # macOS
#   CC=x86_64-w64-mingw32-gcc CFLAGS="-O2 -I$PDC" \
#     LIBS="$PDC/wincon/pdcurses.a" LDFLAGS="-static -s" EXE=.exe asciiverse/build.sh
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
CFLAGS="${CFLAGS:--O2}"
LDFLAGS="${LDFLAGS:-}"
LIBS="${LIBS:-$(pkg-config --libs ncursesw 2>/dev/null || echo -lncursesw) -lm}"
EXE="${EXE:-}"
mkdir -p out
rm -f "out/mandelbrot$EXE"
# shellcheck disable=SC2086
nice -n 10 $CC $CFLAGS -W -Wall main.c $LDFLAGS $LIBS -o "out/mandelbrot$EXE"
echo "built mandelbrot$EXE"
