/**
  *  \file main/c2dump.cpp
  *  \brief c2dump - "Dump" utility - main function
  */

#include "afl/io/filesystem.hpp"
#include "afl/sys/environment.hpp"
#include "game/maint/dump/application.hpp"

int main(int /*argc*/, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return game::maint::dump::Application(env, fs).run();
}
