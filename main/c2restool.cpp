/**
  *  \file main/c2restool.cpp
  *  \brief c2restool utility - Resource Files - main function
  */

#include "afl/io/filesystem.hpp"
#include "afl/sys/environment.hpp"
#include "util/resourcefileapplication.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return util::ResourceFileApplication(env, fs).run();
}
