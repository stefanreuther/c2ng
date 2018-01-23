/**
  *  \file u/t_server_host_session.cpp
  *  \brief Test for server::host::Session
  */

#include <stdexcept>
#include "server/host/session.hpp"

#include "t_server_host.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "server/host/game.hpp"
#include "server/host/root.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "util/processrunner.hpp"

/** Simple test. */
void
TestServerHostSession::testIt()
{
    // Environment
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler null;
    server::interface::MailQueueClient mail(null);
    util::ProcessRunner runner;
    afl::io::NullFileSystem fs;
    server::host::Root root(db, null, null, mail, runner, fs, server::host::Configuration());

    // Prepare database
    afl::net::redis::IntegerSetKey(db, "game:all").add(9);
    afl::net::redis::StringKey(db, "game:9:owner").set("a");

    // Prepare game
    server::host::Game g(root, 9);

    // Tests
    server::host::Session testee;
    TS_ASSERT_THROWS_NOTHING(testee.checkPermission(g, g.AdminPermission));
    TS_ASSERT_THROWS_NOTHING(testee.checkPermission(g, g.ReadPermission));

    testee.setUser("a");
    TS_ASSERT_THROWS_NOTHING(testee.checkPermission(g, g.AdminPermission));

    testee.setUser("b");
    TS_ASSERT_THROWS(testee.checkPermission(g, g.AdminPermission), std::exception);
}
