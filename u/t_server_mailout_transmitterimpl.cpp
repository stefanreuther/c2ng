/**
  *  \file u/t_server_mailout_transmitterimpl.cpp
  *  \brief Test for server::mailout::TransmitterImpl
  */

#include "server/mailout/transmitterimpl.hpp"

#include "t_server_mailout.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/net/nullnetworkstack.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "server/mailout/root.hpp"

/** Test startup/shutdown.
    This is a thread, so ensuring it can be started and stopped makes sense.
    (Actual functionality will be tested in system tests.)*/
void
TestServerMailoutTransmitterImpl::testStartup()
{
    afl::net::redis::InternalDatabase db;
    afl::net::NullNetworkStack net;
    server::mailout::Root root(db, server::mailout::Configuration());
    server::mailout::TransmitterImpl testee(root,
                                            afl::io::InternalDirectory::create(""),
                                            net,
                                            afl::net::Name("127.0.0.1", "21212121"),
                                            afl::net::smtp::Configuration("hello", "from"));
}

