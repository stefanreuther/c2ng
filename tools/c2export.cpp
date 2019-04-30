/**
  *  \file tools/c2export.cpp
  *  \brief c2export Utility
  *
  *  This is the main function for the c2export (command line data exporter) utility.
  */

#include "afl/io/filesystem.hpp"
#include "afl/sys/environment.hpp"
#include "interpreter/exporter/consoleapplication.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return interpreter::exporter::ConsoleApplication(env, fs).run();
}
