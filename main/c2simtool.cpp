/**
  *  \file main/c2simtool.cpp
  *  \brief c2simtool utility - Battle Simulation - main function
  */

#include "game/sim/consoleapplication.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/sys/environment.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return game::sim::ConsoleApplication(env, fs).run();
}

