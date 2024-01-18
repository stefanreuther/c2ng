/**
  *  \file test/server/interface/talkaddressclienttest.cpp
  *  \brief Test for server::interface::TalkAddressClient
  */

#include "server/interface/talkaddressclient.hpp"

#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"

using afl::data::Vector;
using afl::data::VectorValue;

AFL_TEST("server.interface.TalkAddressClient", a)
{
    afl::test::CommandHandler mock(a);
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
        a.checkEqual("01. size", out.size(), 2U);
        a.checkEqual("02. result", out[0], "x");
        a.checkEqual("03. result", out[1], "y");
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
        a.checkEqual("11. size", out.size(), 3U);
        a.checkEqual("12. result", out[0], "q");
        a.checkEqual("13. result", out[1], "r");
        a.checkEqual("14. result", out[2], "s");
    }

    mock.checkFinish();
}
