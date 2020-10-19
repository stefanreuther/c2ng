/**
  *  \file main/c2dbexport.cpp
  *  \brief c2dbexport utility - Database Export - main function
  */

#include "afl/sys/environment.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/net/networkstack.hpp"
#include "server/dbexport/exportapplication.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    afl::net::NetworkStack& net = afl::net::NetworkStack::getInstance();
    return server::dbexport::ExportApplication(env, fs, net).run();
}
