#!/bin/sh
#
#  Quick build script wrapper
#
#  Just a convenience script to codify down the configurations I use to build
#  for daily development. See the 'Select target' section. Feel free to differ.
#
#  Create a build directory and invoke this script as
#     bash /path/to/source/scripts/build.sh [TARGET [MORE ARGS]]
#  where TARGET is the target we want to build for, and MORE ARGS are directly
#  passed to Make.pl. For example, 'build.sh native ninjafile' gets a build.ninja
#  for local building.
#

# Locate source
if test -z "$SRC"; then
  SRC=${0%/*}/..
  if ! test -e "$SRC/game/element.cpp"; then
    echo "Unable to locate source. Set environment variable SRC=" >&2
    exit 1
  fi
fi

# Locate build system
if test -z "$BS"; then
  for i in ../bs* ../../bs*; do
    if test -e "$i/Make.pl"; then
      BS=$i
      break
    fi
  done
fi
if test -z "$BS" || ! test -e "$BS/Make.pl"; then
  echo "Unable to locate build generator. Set environment variable BS=" >&2
  exit 1
fi

# Locate CGI (for doc)
if test -z "$CGI"; then
  for i in ../c2web* ../../c2web*; do
    if test -e "$i/api/user.cgi"; then
      CGI=$i
      break
    fi
  done
fi

# Refuse in-tree build
if test -e game/element.cpp; then
  echo "Do not build within source tree." >&2
  exit 1
fi

# Select target
arch=$1
shift
case "$arch" in
  ""|native)
    name="native"
    set -- AFL_DIR=$SRC/../afl/.build-native/result --disable-sdl
    ;;

  mingw32-cross)
    name="MinGW 32-bit"
    set -- AFL_DIR=$SRC/../afl/.build-mingw32/result TARGET=Win32 CROSS_COMPILE=i686-w64-mingw32- RUN="WINEPATH=/usr/lib/gcc/i686-w64-mingw32/4.9-win32 wine32" "$@"
    ;;

  *)
    echo "Unknown target: '$arch'" >&2
    exit 1
    ;;
esac

# Do it
echo "=========================="
echo " Target: $name"
echo " Source directory: $SRC"
echo " Build system: $BS"
echo " CGI: $CGI"
echo "=========================="
echo

perl "$BS/Make.pl" IN="$SRC" prefix="result" ENABLE_DOC=1 CGI="$CGI" "$@"
