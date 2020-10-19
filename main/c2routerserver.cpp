/**
  *  \file main/c2routerserver.cpp
  *  \brief c2router-server - Play Server Session Multiplexer - main function
  */

#include "afl/async/interrupt.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/sys/environment.hpp"
#include "server/router/serverapplication.hpp"

#ifdef TARGET_OS_POSIX
# include "util/process/posixfactory.hpp"
typedef util::process::PosixFactory SubprocessFactory_t;
#else
// Non-functional fallback for non-POSIX to make it compile
# include "util/process/nullfactory.hpp"
typedef util::process::NullFactory SubprocessFactory_t;
#endif

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    afl::net::NetworkStack& net = afl::net::NetworkStack::getInstance();
    afl::async::Interrupt& intr = afl::async::Interrupt::getInstance();
    SubprocessFactory_t factory;
    server::router::ServerApplication(env, fs, net, intr, factory).run();
}
