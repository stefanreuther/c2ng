/**
  *  \file tools/c2script.cpp
  *  \brief c2script Utility - Script-Related Actions
  *
  */

#include "afl/io/filesystem.hpp"
#include "afl/sys/environment.hpp"
#include "interpreter/consoleapplication.hpp"

int main(int /*argc*/, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return interpreter::ConsoleApplication(env, fs).run();
}
