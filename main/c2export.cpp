/**
  *  \file main/c2export.cpp
  *  \brief c2export utility - Exporter - main function
  */

#include "afl/io/filesystem.hpp"
#include "afl/sys/environment.hpp"
#include "game/interface/exportapplication.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return game::interface::ExportApplication(env, fs).run();
}
