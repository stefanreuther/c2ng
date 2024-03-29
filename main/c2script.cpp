/**
  *  \file main/c2script.cpp
  *  \brief c2script utility - Console Script Execution - main function
  */

#include "afl/io/filesystem.hpp"
#include "afl/sys/environment.hpp"
#include "game/interface/scriptapplication.hpp"

int main(int /*argc*/, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return game::interface::ScriptApplication(env, fs).run();
}
