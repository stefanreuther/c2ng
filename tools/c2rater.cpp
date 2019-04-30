/**
  *  \file tools/c2rater.cpp
  *  \brief "Rater" utility - Main Function
  */

#include "afl/sys/environment.hpp"
#include "afl/io/filesystem.hpp"
#include "game/maint/raterapplication.hpp"

int main(int /*argc*/, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return game::maint::RaterApplication(env, fs).run();
}
