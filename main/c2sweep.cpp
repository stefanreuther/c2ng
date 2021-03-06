/**
  *  \file main/c2sweep.cpp
  *  \brief c2sweep - "Sweep" utility - main function
  */

#include "afl/io/filesystem.hpp"
#include "afl/sys/environment.hpp"
#include "game/maint/sweepapplication.hpp"

int main(int /*argc*/, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return game::maint::SweepApplication(env, fs).run();
}
