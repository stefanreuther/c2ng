/**
  *  \file test/server/talk/sessiontest.cpp
  *  \brief Test for server::talk::Session
  */

#include "server/talk/session.hpp"

#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/root.hpp"
#include <stdexcept>

/** Test permission handling.
    Permissions must behave as expected, i.e. admin user can do everything, users constrained by permission string. */
AFL_TEST("server.talk.Session:permissions", a)
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
        a.check("01. admin", rootSession.hasPermission("p:userProfile1", root));
        a.check("02. admin", rootSession.hasPermission("p:userProfile0", root));
        a.check("03. admin", rootSession.hasPermission("p:other", root));
        a.check("04. admin", rootSession.hasPermission("all", root));
        a.check("05. admin", rootSession.hasPermission("-all", root));
        AFL_CHECK_SUCCEEDS(a("06. admin"), rootSession.checkPermission("p:userProfile1", root));
        AFL_CHECK_SUCCEEDS(a("07. admin"), rootSession.checkPermission("p:userProfile1", root));
        AFL_CHECK_SUCCEEDS(a("08. admin"), rootSession.checkPermission("p:other", root));
        AFL_CHECK_SUCCEEDS(a("09. admin"), rootSession.checkPermission("all", root));
        AFL_CHECK_SUCCEEDS(a("10. admin"), rootSession.checkPermission("-all", root));
    }

    // Test user
    {
        server::talk::Session userSession;
        userSession.setUser("1003");
        a.check("11. user",  userSession.hasPermission("p:userProfile1", root));
        a.check("12. user", !userSession.hasPermission("p:userProfile0", root));
        a.check("13. user", !userSession.hasPermission("p:other", root));
        a.check("14. user",  userSession.hasPermission("all", root));
        a.check("15. user", !userSession.hasPermission("-all", root));
        AFL_CHECK_SUCCEEDS(a("16. user"), userSession.checkPermission("p:userProfile1", root));
        AFL_CHECK_THROWS  (a("17. user"), userSession.checkPermission("p:userProfile0", root), std::exception);
        AFL_CHECK_THROWS  (a("18. user"), userSession.checkPermission("p:other", root), std::exception);
        AFL_CHECK_SUCCEEDS(a("19. user"), userSession.checkPermission("all", root));
        AFL_CHECK_THROWS  (a("20. user"), userSession.checkPermission("-all", root), std::exception);
    }
}

/** Test render options.
    Options must be preserved between calls. */
AFL_TEST("server.talk.Session:renderOptions", a)
{
    server::talk::Session testee;
    testee.renderOptions().setFormat("pdf");
    a.checkEqual("01", testee.renderOptions().getFormat(), "pdf");
}
