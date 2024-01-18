/**
  *  \file test/server/host/sessiontest.cpp
  *  \brief Test for server::host::Session
  */

#include "server/host/session.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/test/testrunner.hpp"
#include "server/host/game.hpp"
#include "server/host/root.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "util/processrunner.hpp"
#include <stdexcept>

/** Simple test. */
AFL_TEST("server.host.Session", a)
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
    AFL_CHECK_SUCCEEDS(a("01. AdminPermission"), testee.checkPermission(g, g.AdminPermission));
    AFL_CHECK_SUCCEEDS(a("02. ReadPermission"), testee.checkPermission(g, g.ReadPermission));

    testee.setUser("a");
    AFL_CHECK_SUCCEEDS(a("11. AdminPermission"), testee.checkPermission(g, g.AdminPermission));

    testee.setUser("b");
    AFL_CHECK_THROWS(a("21. AdminPermission"), testee.checkPermission(g, g.AdminPermission), std::exception);
}
