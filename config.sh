#!/bin/sh
#
#  Make a "config.mk" file
#
#  A trivial low-fat "configure" replacement.
#  Creates a config.mk (for make) and config.h (for the compiler).
#
#  Options:
#     VAR=VALUE         Set 'make' variable (instead of guessing it).
#                       Variables: PERL LDFLAGS LIBS CXX CXXFLAGS CXXTESTDIR
#     --with-PACKAGE    Assume package is present (instead of guessing)
#     --without-PACKAGE Assume package is not present and build without it (instead of guessing)
#                       Packages: openssl, zlib
#
#  If a package is forced present, the respective variables must also be forced completely:
#     ./config.sh --with-zlib LIBS="-lpthread -lrt -lz"
#

# Handling provided variables
needVariable() {
  var="$1"; shift
  eval $var=\"\"
  vars="$vars $var"
  for i; do
    if test "${i%%=*}" = "$var"; then
      echo "Using $i from command line."
      val="${i#*=}"
      eval $var=\$val
      return 1
    fi
  done
  return 0
}

# Handling provided packages
needPackage() {
  package="$1"; shift
  symbol="$1"; shift
  option=$(echo "$symbol" | tr A-Z_ a-z-)
  for i; do
    if test ":$i" = ":--with-$option"; then
      echo "Assuming $package already configured."
      addSymbol HAVE_$symbol
      addVariable HAVE_$symbol yes
      return 1
    fi
    if test ":$i" = ":--without-$option"; then
      echo "Assuming $package not present."
      return 1
    fi
  done
  return 0
}

# Add a preprocessor symbol
addSymbol() {
  defines="$defines $@"
}

# Add a variable
addVariable() {
  var="$1"; shift
  eval $var=\$1
  vars="$vars $var"
}

# Compile
compile() {
  echo > __tmp$$.cpp
  while test -n "$1" && test ":$1" != "::"; do
    echo "$1" >> __tmp$$.cpp; shift
  done
  if test ":$1" = "::"; then shift; fi
  result=false
  case "$1" in
    -[Ll]*) $CXX $CXXFLAGS __tmp$$.cpp "$@" -o __tmp$$ 2>/dev/null >/dev/null && result=true ;;
    *)      $CXX $CXXFLAGS "$@" __tmp$$.cpp -o __tmp$$ 2>/dev/null >/dev/null && result=true ;;
  esac
  rm -f __tmp$$.cpp
  rm -f __tmp$$
  $result
}

# Initial parameters
vars=""
defines=""


#
#  Actual Application Configuration:
#

# Trivial variables
if needVariable PERL "$@"; then
  PERL=perl
fi
needVariable LDFLAGS "$@"
needVariable GUILIBS "$@"

# Find compiler
if needVariable CXX "$@"; then
  echo -n "Looking for compiler..."
  for i in g++-3.3 g++ c++; do
    CXX=$i
    if compile 'int main() { }' : -c; then 
      echo " $CXX"
      break
    fi
    CXX=""
  done
  if test -z "$CXX"; then
    echo " NONE FOUND."
    echo "   Use 'CXX=xxx' to define a compiler."
    exit 1
  fi
fi

# Find command line options
if needVariable CXXFLAGS "$@"; then
  echo -n "Looking for command line options..."
  result=""
  for i in -O2 -W -Wall -ansi -pedantic -fmessage-length=0 -Wno-long-long; do
    CXXFLAGS="$result $i"
    if compile 'int main() { }'; then
      result="$CXXFLAGS"
    fi
  done
  printf '%s\n' "$CXXFLAGS"
fi

# Find afl
if needVariable AFL_DIR "$@"; then
  echo -n "Looking for afl..."
  result=""
  for i in ../afl* ../../afl* ../../../afl*  ../software/afl* ../../software/afl* ../../../software/afl*; do
    if test -f "$i/afl/base/memory.hpp"; then
      AFL_DIR=$i
      echo " $i"
      break
    fi
  done
  if test -z "$AFL_DIR"; then
    echo " NOT FOUND."
    echo "   Use 'AFL_DIR=...' to define the directory."
    exit 1
  fi
fi

# Find CxxTest
if needVariable CXXTESTDIR "$@"; then
  echo -n "Looking for CxxTest..."
  for i in ../cxxtest* ../../cxxtest* ../../../cxxtest*  ../software/cxxtest* ../../software/cxxtest* ../../../software/cxxtest*; do
    if test -f "$i/cxxtest/TestSuite.h"; then
      CXXTESTDIR=$i
      echo " $i"
      break
    fi
  done
  if test -z "$CXXTESTDIR"; then
    echo " NOT FOUND. You'll not be able to build tests."
    echo "   Use 'CXXTESTDIR=xxx' to define the directory."
  fi
fi

# Find zlib
if needPackage sdl SDL "$@"; then
  echo -n "Looking for SDL..."
  if compile '#include <SDL.h>' 'int main() { SDL_Init(0); }' : -lSDL; then
    echo " found."
    addSymbol HAVE_SDL
    addVariable HAVE_SDL yes
    GUILIBS="$GUILIBS -lSDL"
  elif pkg-config --exists sdl && compile '#include <SDL.h>' 'int main() { SDL_Init(0); }' : $(pkg-config --cflags-only-I sdl) $(pkg-config --libs sdl); then
    echo " found (pkg-config)"
    addSymbol HAVE_SDL
    addVariable HAVE_SDL yes
    GUILIBS="$GUILIBS $(pkg-config --libs sdl)"
    CXXFLAGS="$CXXFLAGS $(pkg-config --cflags-only-I sdl)"
  else
    echo " not found."
    echo "   Use GUILIBS= to configure the compiler."
    addVariable HAVE_SDL no
  fi
fi

if needPackage sdl-image SDL_IMAGE "$@"; then
  echo -n "Looking for SDL_image..."
  if compile '#include <SDL_image.h>' 'int main() { IMG_Load_RW(0, 0); }' : -lSDL_image; then
    echo " found."
    addSymbol HAVE_SDL_IMAGE
    GUILIBS="$GUILIBS -lSDL_image"
  elif pkg-config --exists SDL_image && compile '#include <SDL_image.h>' 'int main() { IMG_Load_RW(0, 0); }' : $(pkg-config --cflags-only-I SDL_image) $(pkg-config --libs SDL_image); then
    echo " found (pkg-config)"
    addSymbol HAVE_SDL_IMAGE
    addVariable HAVE_SDL_IMAGE yes
    GUILIBS="$GUILIBS $(pkg-config --libs SDL_image)"
    CXXFLAGS="$CXXFLAGS $(pkg-config --cflags-only-I SDL_image)"
  else
    echo " not found."
    echo "   Use GUILIBS= to configure the compiler."
  fi
fi

# Output
echo "config.sh invoked with parameters '$@'" > config.log
for i in $vars; do
  eval echo CONFIG_C2NG_\$i = \$$i
done > config.mk
for i in $defines; do
  echo "#define $i 1"
done > config.h
