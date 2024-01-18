/**
  *  \file test/server/interface/talksyntaxclienttest.cpp
  *  \brief Test for server::interface::TalkSyntaxClient
  */

#include "server/interface/talksyntaxclient.hpp"

#include "afl/data/vectorvalue.hpp"
#include "afl/net/commandhandler.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"

/** Simple test. */
AFL_TEST("server.interface.TalkSyntaxClient", a)
{
    afl::test::CommandHandler mock(a);
    server::interface::TalkSyntaxClient testee(mock);

    // SYNTAXGET
    mock.expectCall("SYNTAXGET, foo");
    mock.provideNewResult(server::makeStringValue("bar"));
    a.checkEqual("01. get", testee.get("foo"), "bar");

    // SYNTAXMGET (with wrong return value)
    String_t abc[] = {"a","b","c"};
    {
        mock.expectCall("SYNTAXMGET, a, b, c");
        mock.provideNewResult(0);
        afl::data::Vector::Ref_t result = testee.mget(abc);
        a.checkEqual("11. size", result->size(), 0U);
    }

    // SYNTAXMGET (with correct return value)
    {
        afl::data::Vector::Ref_t expectation = afl::data::Vector::create();
        expectation->pushBackString("aa");
        expectation->pushBackNew(0);
        expectation->pushBackInteger(42);
        mock.expectCall("SYNTAXMGET, a, b, c");
        mock.provideNewResult(new afl::data::VectorValue(expectation));

        afl::data::Vector::Ref_t result = testee.mget(abc);
        a.checkEqual("21. size", result->size(), 3U);
        a.checkEqual("22. result", server::toString(result->get(0)), "aa");
        a.checkNull ("23. result", result->get(1));
        a.checkEqual("24. result", server::toString(result->get(2)), "42");
    }

    mock.checkFinish();
}
