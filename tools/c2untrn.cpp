/**
  *  \file tools/c2untrn.cpp
  *  \brief c2untrn Utility - Turn File Dumper and Manipulator
  */

#include "afl/io/filesystem.hpp"
#include "afl/sys/environment.hpp"
#include "game/v3/trn/dumperapplication.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return game::v3::trn::DumperApplication(env, fs).run();
}
