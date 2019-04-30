/**
  *  \file u/t_server_interface_talkaddressclient.cpp
  *  \brief Test for server::interface::TalkAddressClient
  */

#include "server/interface/talkaddressclient.hpp"

#include "t_server_interface.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"

using afl::data::Vector;
using afl::data::VectorValue;

void
TestServerInterfaceTalkAddressClient::testIt()
{
    afl::test::CommandHandler mock("TestServerInterfaceTalkAddressClient::testIt");
    server::interface::TalkAddressClient testee(mock);

    // parse()
    {
        // Expectation
        Vector::Ref_t v = Vector::create();
        v->pushBackString("x");
        v->pushBackString("y");
        mock.expectCall("ADDRMPARSE, a, b, c");
        mock.provideNewResult(new VectorValue(v));

        // Call
        const String_t in[] = {"a", "b", "c"};
        afl::data::StringList_t out;
        testee.parse(in, out);

        // Verify
        TS_ASSERT_EQUALS(out.size(), 2U);
        TS_ASSERT_EQUALS(out[0], "x");
        TS_ASSERT_EQUALS(out[1], "y");
    }

    // render()
    {
        // Expectation
        Vector::Ref_t v = Vector::create();
        v->pushBackString("q");
        v->pushBackString("r");
        v->pushBackString("s");
        mock.expectCall("ADDRMRENDER, 1, 2");
        mock.provideNewResult(new VectorValue(v));

        // Call
        const String_t in[] = {"1", "2"};
        afl::data::StringList_t out;
        testee.render(in, out);

        // Verify
        TS_ASSERT_EQUALS(out.size(), 3U);
        TS_ASSERT_EQUALS(out[0], "q");
        TS_ASSERT_EQUALS(out[1], "r");
        TS_ASSERT_EQUALS(out[2], "s");
    }

    mock.checkFinish();
}

