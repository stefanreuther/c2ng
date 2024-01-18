/**
  *  \file test/server/interface/talkaddressservertest.cpp
  *  \brief Test for server::interface::TalkAddressServer
  */

#include "server/interface/talkaddressserver.hpp"

#include "afl/data/access.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/talkaddress.hpp"
#include "server/interface/talkaddressclient.hpp"
#include <stdexcept>

using afl::string::Format;
using afl::data::Segment;
using afl::data::Access;

namespace {
    String_t join(afl::base::Memory<const String_t> in, const char* delim)
    {
        String_t result;
        if (const String_t* p = in.eat()) {
            result += *p;
            while (const String_t* q = in.eat()) {
                result += delim;
                result += *q;
            }
        }
        return result;
    }

    class TalkAddressMock : public server::interface::TalkAddress, public afl::test::CallReceiver {
     public:
        TalkAddressMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual void parse(afl::base::Memory<const String_t> in, afl::data::StringList_t& out)
            {
                checkCall(Format("parse(%s)", join(in, ",")));
                out = consumeReturnValue<afl::data::StringList_t>();
            }

        virtual void render(afl::base::Memory<const String_t> in, afl::data::StringList_t& out)
            {
                checkCall(Format("render(%s)", join(in, ",")));
                out = consumeReturnValue<afl::data::StringList_t>();
            }
    };
}


AFL_TEST("server.interface.TalkAddressServer:commands", a)
{
    TalkAddressMock mock(a);
    server::interface::TalkAddressServer testee(mock);

    // parse
    {
        afl::data::StringList_t result;
        result.push_back("r1");
        result.push_back("r2");
        mock.expectCall("parse(kk,ll,mm)");
        mock.provideReturnValue(result);

        std::auto_ptr<server::Value_t> p(testee.call(Segment().pushBackString("ADDRMPARSE").pushBackString("kk").pushBackString("ll").pushBackString("mm")));
        Access ap(p);
        a.checkEqual("01. getArraySize", ap.getArraySize(), 2U);
        a.checkEqual("02. result", ap[0].toString(), "r1");
        a.checkEqual("03. result", ap[1].toString(), "r2");
    }

    // render
    {
        afl::data::StringList_t result;
        result.push_back("q1");
        result.push_back("q2");
        result.push_back("q3");
        mock.expectCall("render(e,f,g,h)");
        mock.provideReturnValue(result);

        std::auto_ptr<server::Value_t> p(testee.call(Segment().pushBackString("ADDRMRENDER").pushBackString("e").pushBackString("f").pushBackString("g").pushBackString("h")));
        Access ap(p);
        a.checkEqual("11. getArraySize", ap.getArraySize(), 3U);
        a.checkEqual("12. result", ap[0].toString(), "q1");
        a.checkEqual("13. result", ap[1].toString(), "q2");
        a.checkEqual("14. result", ap[2].toString(), "q3");
    }

    // Variants
    {
        mock.expectCall("render()");
        mock.provideReturnValue(afl::data::StringList_t());

        std::auto_ptr<server::Value_t> p(testee.call(Segment().pushBackString("ADDRMRENDER")));
        a.checkEqual("21. addrmrender", Access(p).getArraySize(), 0U);
    }
    {
        mock.expectCall("render()");
        mock.provideReturnValue(afl::data::StringList_t());

        std::auto_ptr<server::Value_t> p(testee.call(Segment().pushBackString("addRmRendeR")));
        a.checkEqual("31. addrmrender", Access(p).getArraySize(), 0U);
    }

    mock.checkFinish();
}

AFL_TEST("server.interface.TalkAddressServer:errors", a)
{
    TalkAddressMock mock(a);
    server::interface::TalkAddressServer testee(mock);

    Segment empty;
    AFL_CHECK_THROWS(a("01. empty"), testee.callVoid(empty), std::exception);
    AFL_CHECK_THROWS(a("02. bad verb"), testee.callVoid(Segment().pushBackString("")), std::exception);
    AFL_CHECK_THROWS(a("03. bad verb"), testee.callVoid(Segment().pushBackString("foo")), std::exception);
}

AFL_TEST("server.interface.TalkAddressServer:roundtrip", a)
{
    TalkAddressMock mock(a);
    server::interface::TalkAddressServer level1(mock);
    server::interface::TalkAddressClient level2(level1);
    server::interface::TalkAddressServer level3(level2);
    server::interface::TalkAddressClient level4(level3);

    // parse
    {
        afl::data::StringList_t result;
        result.push_back("r1");
        result.push_back("r2");
        mock.expectCall("parse(kk,ll,mm)");
        mock.provideReturnValue(result);

        const String_t in[] = {"kk","ll","mm"};
        afl::data::StringList_t out;
        level4.parse(in, out);

        a.checkEqual("01. size", out.size(), 2U);
        a.checkEqual("02. result", out[0], "r1");
        a.checkEqual("03. result", out[1], "r2");
    }

    // render
    {
        afl::data::StringList_t result;
        result.push_back("q1");
        result.push_back("q2");
        result.push_back("q3");
        mock.expectCall("render(e,f,g,h)");
        mock.provideReturnValue(result);

        const String_t in[] = {"e","f","g","h"};
        afl::data::StringList_t out;
        level4.render(in, out);

        a.checkEqual("11. size", out.size(), 3U);
        a.checkEqual("12. result", out[0], "q1");
        a.checkEqual("13. result", out[1], "q2");
        a.checkEqual("14. result", out[2], "q3");
    }

    mock.checkFinish();
}
