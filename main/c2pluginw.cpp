/**
  *  \file main/c2pluginw.cpp
  *  \brief c2pluginw utility - Plugin Manager Dialog Application - main function
  */

#include "afl/sys/environment.hpp"
#include "afl/io/filesystem.hpp"
#include "util/plugin/dialogapplication.hpp"
#include "afl/sys/dialog.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    afl::sys::Dialog& dialog = afl::sys::Dialog::getSystemInstance();
    return util::plugin::DialogApplication(env, fs, dialog).run();
}
