/**
  *  \file test/server/user/commandhandlertest.cpp
  *  \brief Test for server::user::CommandHandler
  */

#include "server/user/commandhandler.hpp"

#include "afl/data/segment.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"
#include "server/common/numericalidgenerator.hpp"
#include "server/user/classicencrypter.hpp"
#include "server/user/root.hpp"

using afl::data::Segment;

/** Simple test.
    Call once into every child element to make sure command routing works. */
AFL_TEST("server.user.CommandHandler", a)
{
    // Environment
    afl::net::redis::InternalDatabase db;
    server::common::NumericalIdGenerator gen;
    server::user::ClassicEncrypter enc("foo");
    server::user::Root root(db, gen, enc, server::user::Configuration());

    // Testee
    server::user::CommandHandler testee(root);

    // - Basic commands
    a.checkEqual("01. ping", testee.callString(Segment().pushBackString("PING")), "PONG");
    a.check("02. help", testee.callString(Segment().pushBackString("HELP")).size() > 20U);
    a.check("03. help", testee.callString(Segment().pushBackString("HELP").pushBackString("TOKEN")).size() > 20U);

    // - User
    String_t id = testee.callString(Segment().pushBackString("addUser").pushBackString("a").pushBackString("pw"));
    a.checkDifferent("11. adduser", id, "");
    a.checkEqual("12. name", testee.callString(Segment().pushBackString("name").pushBackString(id)), "a");

    // - Token
    String_t tok = testee.callString(Segment().pushBackString("MAKETOKEN").pushBackString(id).pushBackString("login"));
    a.checkDifferent("21. maketoken", tok, "");
    a.checkEqual("22. maketoken", tok, testee.callString(Segment().pushBackString("MAKETOKEN").pushBackString(id).pushBackString("login")));

    // - User data
    a.checkEqual("31. uget", testee.callString(Segment().pushBackString("UGET").pushBackString("u").pushBackString("k")), "");
    AFL_CHECK_SUCCEEDS(a("32. uset"), testee.callString(Segment().pushBackString("USET").pushBackString("u").pushBackString("k").pushBackString("x")));
    a.checkEqual("33. uget", testee.callString(Segment().pushBackString("UGET").pushBackString("u").pushBackString("k")), "x");

    // Some errors
    Segment empty;
    AFL_CHECK_THROWS(a("41. bad verb"), testee.callVoid(Segment().pushBackString("WHATEVER")), std::exception);
    AFL_CHECK_THROWS(a("42. bad verb"), testee.callVoid(Segment().pushBackString("huh?")), std::exception);
    AFL_CHECK_THROWS(a("43. no verb"), testee.callVoid(empty), std::exception);
}
