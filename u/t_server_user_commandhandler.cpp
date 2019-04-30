/**
  *  \file u/t_server_user_commandhandler.cpp
  *  \brief Test for server::user::CommandHandler
  */

#include "server/user/commandhandler.hpp"

#include "t_server_user.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/data/segment.hpp"
#include "server/common/numericalidgenerator.hpp"
#include "server/user/classicencrypter.hpp"
#include "server/user/root.hpp"

using afl::data::Segment;

/** Simple test.
    Call once into every child element to make sure command routing works. */
void
TestServerUserCommandHandler::testIt()
{
    // Environment
    afl::net::redis::InternalDatabase db;
    server::common::NumericalIdGenerator gen;
    server::user::ClassicEncrypter enc("foo");
    server::user::Root root(db, gen, enc, server::user::Configuration());

    // Testee
    server::user::CommandHandler testee(root);

    // - Basic commands
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("PING")), "PONG");
    TS_ASSERT(testee.callString(Segment().pushBackString("HELP")).size() > 20U);
    TS_ASSERT(testee.callString(Segment().pushBackString("HELP").pushBackString("TOKEN")).size() > 20U);

    // - User
    String_t id = testee.callString(Segment().pushBackString("addUser").pushBackString("a").pushBackString("pw"));
    TS_ASSERT_DIFFERS(id, "");
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("name").pushBackString(id)), "a");

    // - Token
    String_t tok = testee.callString(Segment().pushBackString("MAKETOKEN").pushBackString(id).pushBackString("login"));
    TS_ASSERT_DIFFERS(tok, "");
    TS_ASSERT_EQUALS(tok, testee.callString(Segment().pushBackString("MAKETOKEN").pushBackString(id).pushBackString("login")));

    // - User data
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("UGET").pushBackString("u").pushBackString("k")), "");
    TS_ASSERT_THROWS_NOTHING(testee.callString(Segment().pushBackString("USET").pushBackString("u").pushBackString("k").pushBackString("x")));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("UGET").pushBackString("u").pushBackString("k")), "x");

    // Some errors
    Segment empty;
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("WHATEVER")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("huh?")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);
}

