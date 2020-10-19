/**
  *  \file main/c2mgrep.cpp
  *  \brief c2mgrep utility - Message Search - main function
  */

#include "game/maint/messagesearchapplication.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/sys/environment.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return game::maint::MessageSearchApplication(env, fs).run();
}
