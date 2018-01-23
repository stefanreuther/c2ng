/**
  *  \file server/c2file.cpp
  *  \brief c2file-server main
  */

#include "afl/async/interrupt.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/sys/environment.hpp"
#include "server/file/serverapplication.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    afl::net::NetworkStack& net = afl::net::NetworkStack::getInstance();
    afl::async::Interrupt& intr = afl::async::Interrupt::getInstance();
    server::file::ServerApplication(env, fs, net, intr).run();
}
