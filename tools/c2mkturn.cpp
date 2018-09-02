/**
  *  \file tools/c2mkturn.cpp
  *  \brief "Maketurn" utility - Main Function
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
