/**
  *  \file server/c2nntp.cpp
  *  \brief c2nntp-server main
  */


#include "afl/async/interrupt.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/sys/environment.hpp"
#include "server/nntp/serverapplication.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    afl::net::NetworkStack& net = afl::net::NetworkStack::getInstance();
    afl::async::Interrupt& intr = afl::async::Interrupt::getInstance();
    server::nntp::ServerApplication(env, fs, net, intr).run();
}
