/**
  *  \file main/c2mailoutserver.cpp
  *  \brief c2mailout-server - Mail Transmitter - main function
  */

#include "afl/async/interrupt.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/sys/environment.hpp"
#include "server/mailout/serverapplication.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    afl::net::NetworkStack& net = afl::net::NetworkStack::getInstance();
    afl::async::Interrupt& intr = afl::async::Interrupt::getInstance();
    server::mailout::ServerApplication(env, fs, net, intr).run();
}
