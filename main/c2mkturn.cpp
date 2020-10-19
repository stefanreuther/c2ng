/**
  *  \file main/c2mkturn.cpp
  *  \brief c2mkturn utility - Maketurn - main function
  */

#include "afl/io/filesystem.hpp"
#include "afl/sys/environment.hpp"
#include "game/v3/maketurnapplication.hpp"

int main(int /*argc*/, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return game::v3::MaketurnApplication(env, fs).run();
}
