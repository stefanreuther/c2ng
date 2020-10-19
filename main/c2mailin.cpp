/**
  *  \file main/c2mailin.cpp
  *  \brief c2mailin utility - Parse Mail and Push into Services - main function
  */

#include "afl/sys/environment.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/net/networkstack.hpp"
#include "server/mailin/mailinapplication.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    afl::net::NetworkStack& net = afl::net::NetworkStack::getInstance();
    return server::mailin::MailInApplication(env, fs, net).run();
}
