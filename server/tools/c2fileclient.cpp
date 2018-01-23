/**
  *  \file server/tools/c2fileclient.cpp
  *  \brief c2fileclient main
  */

#include "afl/io/filesystem.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/sys/environment.hpp"
#include "server/file/clientapplication.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    afl::net::NetworkStack& net = afl::net::NetworkStack::getInstance();
    return server::file::ClientApplication(env, fs, net).run();
}
