/**
  *  \file test/server/talk/accesscheckertest.cpp
  *  \brief Test for server::talk::AccessChecker
  */

#include "server/talk/accesschecker.hpp"

#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/message.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "server/talk/topic.hpp"
#include <stdexcept>

AFL_TEST("server.talk.AccessChecker:basics", a)
{
    // Infrastructure
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, server::talk::Configuration());

    // Database content
    // - forum 1: readable by user 1001
    server::talk::Forum f1(root, 1);
    f1.readPermissions().set("u:1001");
    root.allForums().add(1);

    // - topic 11: readable by user 1002
    server::talk::Topic t11(root, 11);
    t11.readPermissions().set("u:1002");
    t11.forumId().set(1);
    f1.topics().add(11);

    // - topic 12: no permissions set
    server::talk::Topic t12(root, 12);
    t12.forumId().set(1);
    f1.topics().add(12);

    // - message 21: in topic 11
    server::talk::Message m21(root, 21);
    m21.topicId().set(11);
    t11.messages().add(21);
    f1.messages().add(21);

    // - message 22: in topic 11
    server::talk::Message m22(root, 22);
    m22.topicId().set(12);
    t12.messages().add(22);
    f1.messages().add(22);

    // Check root access
    {
        server::talk::Session s;
        server::talk::AccessChecker testee(root, s);
        a.check("01", testee.isAllowed(t11));
        a.check("02", testee.isAllowed(m21));
        a.check("03", testee.isAllowed(m22));
        a.check("04", testee.isAllowed(t12));

        AFL_CHECK_SUCCEEDS(a("05. t11"), testee.checkTopic(t11));
        AFL_CHECK_SUCCEEDS(a("06. m21"), testee.checkMessage(m21));
        AFL_CHECK_SUCCEEDS(a("07. m22"), testee.checkMessage(m22));
        AFL_CHECK_SUCCEEDS(a("08. t12"), testee.checkTopic(t12));
    }

    // Check user 1001 access: can read the second topic
    {
        server::talk::Session s;
        s.setUser("1001");
        server::talk::AccessChecker testee(root, s);
        a.check("11", !testee.isAllowed(t11));
        a.check("12", !testee.isAllowed(m21));
        a.check("13", testee.isAllowed(m22));
        a.check("14", testee.isAllowed(t12));

        AFL_CHECK_THROWS(a("15. t11"), testee.checkTopic(t11), std::runtime_error);
        AFL_CHECK_THROWS(a("16. m21"), testee.checkMessage(m21), std::runtime_error);
        AFL_CHECK_SUCCEEDS(a("17. m22"), testee.checkMessage(m22));
        AFL_CHECK_SUCCEEDS(a("18. t12"), testee.checkTopic(t12));
    }

    // Check user 1002 access: can read the second topic
    {
        server::talk::Session s;
        s.setUser("1002");
        server::talk::AccessChecker testee(root, s);
        a.check("21", testee.isAllowed(t11));
        a.check("22", testee.isAllowed(m21));
        a.check("23", !testee.isAllowed(m22));
        a.check("24", !testee.isAllowed(t12));
    }

    // Check user 1003 access: cannot read anything
    {
        server::talk::Session s;
        s.setUser("1003");
        server::talk::AccessChecker testee(root, s);
        a.check("31", !testee.isAllowed(t11));
        a.check("32", !testee.isAllowed(m21));
        a.check("33", !testee.isAllowed(m22));
        a.check("34", !testee.isAllowed(t12));

        AFL_CHECK_THROWS(a("41. checkMessage"), testee.checkMessage(m21), std::exception);
        AFL_CHECK_THROWS(a("42. checkTopic"), testee.checkTopic(t11), std::exception);
    }
}
