/**
  *  \file main/c2gfxgen.cpp
  *  \brief c2gfxgen utility - Graphics Generator - main function
  */

#include "afl/io/filesystem.hpp"
#include "afl/sys/environment.hpp"
#include "gfx/gen/application.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return gfx::gen::Application(env, fs).run();
}
