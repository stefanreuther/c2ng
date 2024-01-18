/**
  *  \file test/server/interface/talksyntaxservertest.cpp
  *  \brief Test for server::interface::TalkSyntaxServer
  */

#include "server/interface/talksyntaxserver.hpp"

#include "afl/data/access.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/talksyntax.hpp"
#include "server/interface/talksyntaxclient.hpp"
#include "server/types.hpp"
#include <memory>
#include <stdexcept>

namespace {
    class TalkSyntaxMock : public server::interface::TalkSyntax, public afl::test::CallReceiver {
     public:
        TalkSyntaxMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual String_t get(String_t key)
            {
                checkCall("get " + key);
                return consumeReturnValue<String_t>();
            }

        virtual afl::base::Ref<afl::data::Vector> mget(afl::base::Memory<const String_t> keys)
            {
                String_t command = "mget";
                for (size_t i = 0; i < keys.size(); ++i) {
                    command += " ";
                    command += *keys.at(i);
                }
                checkCall(command);
                return consumeReturnValue<afl::data::Vector::Ref_t>();
            }
    };
}

AFL_TEST("server.interface.TalkSyntaxServer:commands", a)
{
    TalkSyntaxMock mock(a);
    server::interface::TalkSyntaxServer testee(mock);

    // SYNTAXGET
    {
        mock.expectCall("get a.b.info");
        mock.provideReturnValue(String_t("a.b.result"));
        std::auto_ptr<afl::data::Value> result(testee.call(afl::data::Segment().pushBackString("SYNTAXGET").pushBackString("a.b.info")));
        a.checkEqual("01. syntaxget", server::toString(result.get()), "a.b.result");
    }

    // Same thing, lower case
    {
        mock.expectCall("get lower");
        mock.provideReturnValue(String_t("lower result"));
        std::auto_ptr<afl::data::Value> result(testee.call(afl::data::Segment().pushBackString("syntaxGet").pushBackString("lower")));
        a.checkEqual("11. syntaxget", server::toString(result.get()), "lower result");
    }

    // SYNTAXMGET
    {
        mock.expectCall("mget qa qb");
        afl::data::Vector::Ref_t expect = afl::data::Vector::create();
        expect->pushBackString("aa");
        expect->pushBackString("ab");
        mock.provideReturnValue(expect);
        std::auto_ptr<afl::data::Value> result(testee.call(afl::data::Segment().pushBackString("SYNTAXMGET").pushBackString("qa").pushBackString("qb")));
        a.checkEqual("21. size",   afl::data::Access(result).getArraySize(), 2U);
        a.checkEqual("22. result", afl::data::Access(result)[0].toString(), "aa");
        a.checkEqual("23. result", afl::data::Access(result)[1].toString(), "ab");
    }

    // Syntax errors. Those do not end up at the mock.
    AFL_CHECK_THROWS(a("31. bad verb"),      testee.callVoid(afl::data::Segment().pushBackString("whatever")), std::exception);
    AFL_CHECK_THROWS(a("32. missing arg"),   testee.callVoid(afl::data::Segment().pushBackString("SYNTAXGET")), std::exception);
    AFL_CHECK_THROWS(a("33. too many args"), testee.callVoid(afl::data::Segment().pushBackString("SYNTAXGET").pushBackString("a").pushBackString("b")), std::exception);

    afl::data::Segment empty;
    interpreter::Arguments args(empty, 0, 0);
    std::auto_ptr<afl::data::Value> p;
    a.checkEqual("41. bad verb", testee.handleCommand("huhu", args, p), false);

    mock.checkFinish();
}

/** Test roundtrip. */
AFL_TEST("server.interface.TalkSyntaxServer:roundtrip", a)
{
    TalkSyntaxMock mock(a);
    server::interface::TalkSyntaxServer level1(mock);
    server::interface::TalkSyntaxClient level2(level1);
    server::interface::TalkSyntaxServer level3(level2);
    server::interface::TalkSyntaxClient level4(level3);

    // get
    {
        mock.expectCall("get aa");
        mock.provideReturnValue(String_t("bb"));
        a.checkEqual("01. get", level4.get("aa"), "bb");
    }

    // mget
    {
        String_t q[] = {"q1","q2"};
        mock.expectCall("mget q1 q2");
        afl::data::Vector::Ref_t expect = afl::data::Vector::create();
        expect->pushBackString("a1");
        expect->pushBackString("a2");
        mock.provideReturnValue(expect);

        afl::data::Vector::Ref_t result = level4.mget(q);
        a.checkEqual("11. size", result->size(), 2U);
        a.checkEqual("12. result", server::toString(result->get(0)), "a1");
        a.checkEqual("13. result", server::toString(result->get(1)), "a2");
    }
}
