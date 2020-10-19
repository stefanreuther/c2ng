/**
  *  \file main/c2plugin.cpp
  *  \brief c2plugin utility - Plugin Manager - main function
  */

#include "afl/io/filesystem.hpp"
#include "afl/sys/environment.hpp"
#include "util/plugin/consoleapplication.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return util::plugin::ConsoleApplication(env, fs).run();
}
