/**
  *  \file u/t_server_interface_talksyntaxclient.cpp
  *  \brief Test for server::interface::TalkSyntaxClient
  */

#include "server/interface/talksyntaxclient.hpp"

#include "t_server_interface.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/net/commandhandler.hpp"
#include "afl/test/commandhandler.hpp"
#include "server/types.hpp"

/** Simple test. */
void
TestServerInterfaceTalkSyntaxClient::testIt()
{
    afl::test::CommandHandler mock("testIt");
    server::interface::TalkSyntaxClient testee(mock);

    // SYNTAXGET
    mock.expectCall("SYNTAXGET, foo");
    mock.provideNewResult(server::makeStringValue("bar"));
    TS_ASSERT_EQUALS(testee.get("foo"), "bar");

    // SYNTAXMGET (with wrong return value)
    String_t abc[] = {"a","b","c"};
    {
        mock.expectCall("SYNTAXMGET, a, b, c");
        mock.provideNewResult(0);
        afl::data::Vector::Ref_t result = testee.mget(abc);
        TS_ASSERT_EQUALS(result->size(), 0U);
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
        TS_ASSERT_EQUALS(result->size(), 3U);
        TS_ASSERT_EQUALS(server::toString(result->get(0)), "aa");
        TS_ASSERT(result->get(1) == 0);
        TS_ASSERT_EQUALS(server::toString(result->get(2)), "42");
    }

    mock.checkFinish();
}

