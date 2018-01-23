/**
  *  \file server/tools/c2console.cpp
  *  \brief c2console main
  */

#include "afl/sys/environment.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/net/networkstack.hpp"
#include "server/console/consoleapplication.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    afl::net::NetworkStack& net = afl::net::NetworkStack::getInstance();
    return server::console::ConsoleApplication(env, fs, net).run();
}
