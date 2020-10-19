/**
  *  \file main/c2unpack.cpp
  *  \brief c2unpack - "Unpack" utility - main function
  */

#include "afl/io/filesystem.hpp"
#include "afl/sys/environment.hpp"
#include "game/v3/unpackapplication.hpp"

int main(int /*argc*/, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return game::v3::UnpackApplication(env, fs).run();
}
