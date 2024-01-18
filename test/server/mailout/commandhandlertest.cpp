/**
  *  \file test/server/mailout/commandhandlertest.cpp
  *  \brief Test for server::mailout::CommandHandler
  */

#include "server/mailout/commandhandler.hpp"

#include "afl/data/segment.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"
#include "server/mailout/root.hpp"
#include "server/mailout/session.hpp"

/** Simple test.
    CommandHandler basically just dispatches to a MailQueue, so we just perform basic functionality test. */
AFL_TEST("server.mailout.CommandHandler", a)
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());
    server::mailout::Session session;
    server::mailout::CommandHandler testee(root, session);

    // Command without result, and observable effect
    a.checkNull("01. currentMessage", session.currentMessage.get());
    testee.callVoid(afl::data::Segment().pushBackString("MAIL").pushBackString("xyz"));
    a.checkNonNull("02. MAIL", session.currentMessage.get());

    // Command with result
    String_t helpText = testee.callString(afl::data::Segment().pushBackString("HELP"));
    a.check("11. HELP", helpText.size() > 0);
}
