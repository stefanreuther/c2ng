/**
  *  \file main/c2configtool.cpp
  *  \brief c2configtool utility - Configuration Manipulation - main function
  */

#include "game/maint/configurationapplication.hpp"
#include "afl/sys/environment.hpp"
#include "afl/io/filesystem.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return game::maint::ConfigurationApplication(env, fs).run();
}
