/**
  *  \file u/t_server_talk_session.cpp
  *  \brief Test for server::talk::Session
  */

#include <stdexcept>
#include "server/talk/session.hpp"

#include "t_server_talk.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "server/talk/root.hpp"
#include "afl/net/nullcommandhandler.hpp"

/** Test permission handling.
    Permissions must behave as expected, i.e. admin user can do everything, users constrained by permission string. */
void
TestServerTalkSession::testPermission()
{
    // Create a preloaded internal database
    using afl::data::Segment;
    afl::net::redis::InternalDatabase db;
    db.callVoid(Segment().pushBackString("hset").pushBackString("user:1003:profile").pushBackString("userProfile1").pushBackString("1"));
    db.callVoid(Segment().pushBackString("hset").pushBackString("user:1003:profile").pushBackString("userProfile0").pushBackString("0"));

    // Surroundings
    afl::net::NullCommandHandler null;
    server::talk::Root root(db, null, server::talk::Configuration());

    // Test admin
    {
        server::talk::Session rootSession;
        TS_ASSERT(rootSession.hasPermission("p:userProfile1", root));
        TS_ASSERT(rootSession.hasPermission("p:userProfile0", root));
        TS_ASSERT(rootSession.hasPermission("p:other", root));
        TS_ASSERT(rootSession.hasPermission("all", root));
        TS_ASSERT(rootSession.hasPermission("-all", root));
        TS_ASSERT_THROWS_NOTHING(rootSession.checkPermission("p:userProfile1", root));
        TS_ASSERT_THROWS_NOTHING(rootSession.checkPermission("p:userProfile1", root));
        TS_ASSERT_THROWS_NOTHING(rootSession.checkPermission("p:other", root));
        TS_ASSERT_THROWS_NOTHING(rootSession.checkPermission("all", root));
        TS_ASSERT_THROWS_NOTHING(rootSession.checkPermission("-all", root));
    }

    // Test user
    {
        server::talk::Session userSession;
        userSession.setUser("1003");
        TS_ASSERT( userSession.hasPermission("p:userProfile1", root));
        TS_ASSERT(!userSession.hasPermission("p:userProfile0", root));
        TS_ASSERT(!userSession.hasPermission("p:other", root));
        TS_ASSERT( userSession.hasPermission("all", root));
        TS_ASSERT(!userSession.hasPermission("-all", root));
        TS_ASSERT_THROWS_NOTHING(userSession.checkPermission("p:userProfile1", root));
        TS_ASSERT_THROWS        (userSession.checkPermission("p:userProfile0", root), std::exception);
        TS_ASSERT_THROWS        (userSession.checkPermission("p:other", root), std::exception);
        TS_ASSERT_THROWS_NOTHING(userSession.checkPermission("all", root));
        TS_ASSERT_THROWS        (userSession.checkPermission("-all", root), std::exception);
    }
}

/** Test render options.
    Options must be preserved between calls. */
void
TestServerTalkSession::testRenderOptions()
{
    server::talk::Session testee;
    testee.renderOptions().setFormat("pdf");
    TS_ASSERT_EQUALS(testee.renderOptions().getFormat(), "pdf");
}

