/**
  *  \file server/c2play.cpp
  *  \brief c2play-server main
  */

#include "afl/io/filesystem.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/sys/environment.hpp"
#include "server/play/consoleapplication.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    afl::net::NetworkStack& net = afl::net::NetworkStack::getInstance();
    server::play::ConsoleApplication(env, fs, net).run();
}
