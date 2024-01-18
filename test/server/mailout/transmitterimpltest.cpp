/**
  *  \file test/server/mailout/transmitterimpltest.cpp
  *  \brief Test for server::mailout::TransmitterImpl
  */

#include "server/mailout/transmitterimpl.hpp"

#include "afl/io/internaldirectory.hpp"
#include "afl/net/nullnetworkstack.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"
#include "server/mailout/root.hpp"

/** Test startup/shutdown.
    This is a thread, so ensuring it can be started and stopped makes sense.
    (Actual functionality will be tested in system tests.)*/
AFL_TEST_NOARG("server.mailout.TransmitterImpl:startup")
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
