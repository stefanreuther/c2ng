/**
  *  \file main/c2check.cpp
  *  \brief c2check utility - Turn Checker - main function
  */

#include "game/v3/check/application.hpp"
#include "afl/sys/environment.hpp"
#include "afl/io/filesystem.hpp"

int main(int, char** argv)
{
    return game::v3::check::Application(afl::sys::Environment::getInstance(argv),
                                        afl::io::FileSystem::getInstance()).run();
}
