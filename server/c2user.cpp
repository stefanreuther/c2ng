/**
  *  \file server/c2user.cpp
  *  \brief c2user-server main
  */

#include "afl/async/interrupt.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/sys/environment.hpp"
#include "server/user/serverapplication.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    afl::net::NetworkStack& net = afl::net::NetworkStack::getInstance();
    afl::async::Interrupt& intr = afl::async::Interrupt::getInstance();
    server::user::ServerApplication(env, fs, net, intr).run();
}
