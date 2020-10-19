/**
  *  \file main/c2rater.cpp
  *  \brief c2rater utility - Configuration Rating - main function
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
