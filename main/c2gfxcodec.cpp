/**
  *  \file main/c2gfxcodec.cpp
  *  \brief c2gfxcodec - Graphics Codecs - main function
  */

#include "afl/io/filesystem.hpp"
#include "afl/sys/environment.hpp"
#include "gfx/codec/application.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return gfx::codec::Application(env, fs).run();
}
