/**
  *  \file main/c2docmanager.cpp
  *  \brief c2docmanager utility - Documentation Manager - main function
  */

#include "afl/io/filesystem.hpp"
#include "afl/sys/environment.hpp"
#include "util/doc/application.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return util::doc::Application(env, fs).run();
}
