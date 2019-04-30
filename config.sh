#!/bin/bash
#
#  Configuration for c2ng
#
#  Variables to generate:
#    CXXTESTDIR
#    PERL
#    CXX, CXXFLAGS
#    LIBS, LDFLAGS, [LIBDEPEND]
#    AR
#    RUN
#
#  Macros to generate:
#    HAVE_SDL
#    HAVE_SDL_IMAGE
#

conf_user_help() {
    echo "Packages:
  sdl
  sdl_image

Variables:
  AR
  CXX
  CXXFLAGS
  CXXTESTDIR
  LDFLAGS
  LIBDEPEND
  LIBS
  PERL
  RUN
"
}

case "$0" in
    */*) . ${0%/*}/config.lib ;;
    *)   .         config.lib ;;
esac

##
##  Programs
##

# Find compiler
conf_log_start "Looking for C++ compiler..."
conf_compile_set_program "int main() { }"
if test -n "$CXX"; then
    conf_compile_try CXX=$CXX || conf_log_die "The configured compiler, \"$CXX\", does not work." >&2
else
    for i in ${CROSS_COMPILE}g++ ${CROSS_COMPILE}c++; do
        conf_compile_try CXX=$i && break
    done
    test -z "$CXX" && conf_log_die "No C++ compiler."
fi
conf_log_result "$CXX"

# Find compiler options
conf_log_start "Looking for C++ compiler flags..."
conf_var_set CXXFLAGS "$CXXFLAGS"
for i in -O2 -ansi -pedantic -fmessage-length=0 -Wno-long-long -Wconversion -mthreads -W -Wall; do
    # build it backwards so user options are at the end and can override ours
    conf_compile_try CXXFLAGS="$i $CXXFLAGS"
done
conf_log_result "$CXXFLAGS"

# Find archiver
conf_log_start "Looking for archiver..."
conf_var_set AR "${AR:-ar}"
conf_run $AR cr __tmp.a || conf_log_die "Does not work."
rm -f __tmp.a
conf_log_result "$AR"

# Set RUN variable (no guess)
conf_var_publish RUN

# Find perl
conf_log_start "Checking for perl..."
conf_var_set PERL "${PERL:-perl}"
conf_run $PERL -e 'use 5;' || conf_log_die "Does not work."
conf_log_result "$PERL"

###
###  Libraries
###

conf_var_publish LIBS LDFLAGS LIBDEPEND GUILIBS

# CxxTest
conf_log_start "Looking for CxxTest..."
conf_file_set cxxtest/TestSuite.h cxxtestgen.pl
if test -z "$CXXTESTDIR"; then
    for i in $CXXTESTDIR ../cxxtest* ../../cxxtest* ../../../cxxtest*  ../software/cxxtest* ../../software/cxxtest* ../../../software/cxxtest*; do
        if conf_file_check_dir "$i"; then
            conf_var_set CXXTESTDIR "$i"
            conf_var_set HAVE_CXXTEST yes
            conf_log_result "$CXXTESTDIR"
            break
        fi
    done
    if test -z "$CXXTESTDIR"; then
        conf_log_result "not found"
        conf_var_set CXXTESTDIR ""
        conf_var_set HAVE_CXXTEST no
    fi
else
    conf_file_check_dir "$CXXTESTDIR" || conf_log_die "The specified directory \"$CXXTESTDIR\" does not work."
    conf_log_result "OK"
    conf_var_publish CXXTESTDIR
    conf_var_set HAVE_CXXTEST yes
fi

# afl
conf_log_start "Looking for afl..."
conf_file_set afl/base/memory.hpp
if test -z "$AFL_DIR"; then
    for i in $AFL_DIR ../afl* ../../afl* ../../../afl*  ../software/afl* ../../software/afl* ../../../software/afl*; do
        if conf_file_check_dir "$i"; then
            conf_var_set AFL_DIR "$i"
            conf_log_result "$AFL_DIR"
            break
        fi
    done
    test -n "$AFL_DIR" || conf_log_die "afl not found. Use \"AFL_DIR=...\" to specify the path."
else
    conf_file_check_dir "$AFL_DIR" || conf_log_die "The specified directory \"$AFL_DIR\" does not work."
    conf_var_publish AFL_DIR
    conf_log_result "OK"
fi

# We're testing GUI libraries in the following, so swap out libs...
NONGUILIBS=$LIBS
LIBS=$GUILIBS
conf_var_set HAVE_GUILIB no

# SDL
conf_log_start "Looking for SDL..."
conf_var_set HAVE_SDL no
conf_compile_set_program "
  #include <SDL.h>
  #undef main
  int main() { SDL_Init(0); }"
case $(conf_arg_pkg_status sdl) in
    required)
        libdir="$conf_pkg_dir_sdl"
        if test -n "$libdir"; then
            conf_link_try CXXFLAGS="$CXXFLAGS -I$libdir" LDFLAGS="$LDFLAGS -L$libdir" ||
                conf_link_try CXXFLAGS="$CXXFLAGS -I$libdir/include" LDFLAGS="$LDFLAGS -L$libdir/lib" ||
                conf_log_die "The configured directory, \"$libdir\", does not work."
        else
            conf_link_try || conf_link_try_pkgconfig sdl ||
                conf_log_die "This required package could not be found."
        fi
        conf_var_set HAVE_SDL yes
        conf_var_set HAVE_GUILIB yes
        conf_macro_set HAVE_SDL
        conf_log_result "OK"
        ;;
    disabled)
        conf_log_result "disabled"
        ;;
    *)
        if conf_link_try || conf_link_try_pkgconfig sdl; then
            conf_var_set HAVE_SDL yes
            conf_var_set HAVE_GUILIB yes
            conf_macro_set HAVE_SDL
            conf_log_result "OK"
        else
            conf_log_result "not found"
        fi
        ;;
esac

# SDL_image
if ${conf_macro_set_HAVE_SDL:-false}; then
    conf_log_start "Looking for SDL_image..."
    conf_compile_set_program "
  #include <SDL_image.h>
  #undef main
  int main() { IMG_Load_RW(0, 0); }"
    case $(conf_arg_pkg_status sdl_image) in
        required)
            libdir="$conf_pkg_dir_sdl_image"
            if test -n "$libdir"; then
                conf_link_try CXXFLAGS="$CXXFLAGS -I$libdir" LDFLAGS="$LDFLAGS -L$libdir" LIBS="$LIBS -lSDL_image"||
                    conf_link_try CXXFLAGS="$CXXFLAGS -I$libdir/include" LDFLAGS="$LDFLAGS -L$libdir/lib" LIBS="$LIBS -lSDL_image" ||
                    conf_log_die "The configured directory, \"$libdir\", does not work."
            else
                conf_link_try || conf_link_try LIBS="$LIBS -lSDL_image" || conf_link_try_pkgconfig SDL_image ||
                    conf_log_die "This required package could not be found."
            fi
            conf_var_set HAVE_SDL_IMAGE yes
            conf_macro_set HAVE_SDL_IMAGE
            conf_log_result "OK"
            ;;
        disabled)
            conf_log_result "disabled"
            ;;
        *)
            if conf_link_try LIBS="$LIBS -lSDL_image" || conf_link_try_pkgconfig SDL_image; then
                conf_var_set HAVE_SDL_IMAGE yes
                conf_macro_set HAVE_SDL_IMAGE
                conf_log_result "OK"
            else
                conf_log_result "not found"
            fi
            ;;
    esac
fi

# SDL2
if ! ${conf_macro_set_HAVE_SDL:-false}; then
    conf_log_start "Looking for SDL2..."
    conf_var_set HAVE_SDL2 no
    conf_compile_set_program "
  #include <SDL.h>
  #undef main
  int main() { SDL_Init(0); }"
    case $(conf_arg_pkg_status sdl2) in
        required)
            libdir="$conf_pkg_dir_sdl2"
            if test -n "$libdir"; then
                conf_link_try CXXFLAGS="$CXXFLAGS -I$libdir" LDFLAGS="$LDFLAGS -L$libdir" ||
                    conf_link_try CXXFLAGS="$CXXFLAGS -I$libdir/include" LDFLAGS="$LDFLAGS -L$libdir/lib" ||
                    conf_log_die "The configured directory, \"$libdir\", does not work."
            else
                conf_link_try || conf_link_try_pkgconfig sdl2 ||
                    conf_log_die "This required package could not be found."
            fi
            conf_var_set HAVE_SDL2 yes
            conf_var_set HAVE_GUILIB yes
            conf_macro_set HAVE_SDL2
            conf_log_result "OK"
            ;;
        disabled)
            conf_log_result "disabled"
            ;;
        *)
            if conf_link_try || conf_link_try_pkgconfig sdl2; then
                conf_var_set HAVE_SDL2 yes
                conf_var_set HAVE_GUILIB yes
                conf_macro_set HAVE_SDL2
                conf_log_result "OK"
            else
                conf_log_result "not found"
            fi
            ;;
    esac
fi

# SDL2_image
if ${conf_macro_set_HAVE_SDL2:-false}; then
    conf_log_start "Looking for SDL2_image..."
    conf_compile_set_program "
  #include <SDL_image.h>
  #undef main
  int main() { IMG_Load_RW(0, 0); }"
    case $(conf_arg_pkg_status sdl2_image) in
        required)
            libdir="$conf_pkg_dir_sdl2_image"
            if test -n "$libdir"; then
                conf_link_try CXXFLAGS="$CXXFLAGS -I$libdir" LDFLAGS="$LDFLAGS -L$libdir" LIBS="$LIBS -lSDL2_image"||
                    conf_link_try CXXFLAGS="$CXXFLAGS -I$libdir/include" LDFLAGS="$LDFLAGS -L$libdir/lib" LIBS="$LIBS -lSDL2_image" ||
                    conf_log_die "The configured directory, \"$libdir\", does not work."
            else
                conf_link_try || conf_link_try LIBS="$LIBS -lSDL2_image" || conf_link_try_pkgconfig SDL2_image ||
                    conf_log_die "This required package could not be found."
            fi
            conf_var_set HAVE_SDL2_IMAGE yes
            conf_macro_set HAVE_SDL2_IMAGE
            conf_log_result "OK"
            ;;
        disabled)
            conf_log_result "disabled"
            ;;
        *)
            if conf_link_try LIBS="$LIBS -lSDL2_image" || conf_link_try_pkgconfig SDL2_image; then
                conf_var_set HAVE_SDL2_IMAGE yes
                conf_macro_set HAVE_SDL2_IMAGE
                conf_log_result "OK"
            else
                conf_log_result "not found"
            fi
            ;;
    esac
fi

# Swap settings back
GUILIBS=$LIBS
LIBS=$NONGUILIBS


##
##  Generate output
##

conf_var_write_makefile CONFIG_C2NG_ | conf_update_file config.mk
conf_macro_write_header              | conf_update_file config.h
conf_cleanup
