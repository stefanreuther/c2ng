/**
  *  \file u/t_server_mailout_commandhandler.cpp
  *  \brief Test for server::mailout::CommandHandler
  */

#include "server/mailout/commandhandler.hpp"

#include "t_server_mailout.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "server/mailout/root.hpp"
#include "server/mailout/session.hpp"
#include "afl/data/segment.hpp"

/** Simple test.
    CommandHandler basically just dispatches to a MailQueue, so we just perform basic functionality test. */
void
TestServerMailoutCommandHandler::testIt()
{
    afl::net::redis::InternalDatabase db;
    server::mailout::Root root(db, server::mailout::Configuration());
    server::mailout::Session session;
    server::mailout::CommandHandler testee(root, session);

    // Command without result, and observable effect
    TS_ASSERT(session.currentMessage.get() == 0);
    testee.callVoid(afl::data::Segment().pushBackString("MAIL").pushBackString("xyz"));
    TS_ASSERT(session.currentMessage.get() != 0);

    // Command with result
    String_t helpText = testee.callString(afl::data::Segment().pushBackString("HELP"));
    TS_ASSERT(helpText.size() > 0);
}

