/**
  *  \file u/t_server_interface_talkaddressserver.cpp
  *  \brief Test for server::interface::TalkAddressServer
  */

#include <stdexcept>
#include "server/interface/talkaddressserver.hpp"

#include "t_server_interface.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/data/access.hpp"
#include "server/interface/talkaddressclient.hpp"
#include "server/interface/talkaddress.hpp"

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


void
TestServerInterfaceTalkAddressServer::testIt()
{
    TalkAddressMock mock("TestServerInterfaceTalkAddressServer::testIt");
    server::interface::TalkAddressServer testee(mock);

    // parse
    {
        afl::data::StringList_t result;
        result.push_back("r1");
        result.push_back("r2");
        mock.expectCall("parse(kk,ll,mm)");
        mock.provideReturnValue(result);

        std::auto_ptr<server::Value_t> p(testee.call(Segment().pushBackString("ADDRMPARSE").pushBackString("kk").pushBackString("ll").pushBackString("mm")));
        Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
        TS_ASSERT_EQUALS(a[0].toString(), "r1");
        TS_ASSERT_EQUALS(a[1].toString(), "r2");
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
        Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 3U);
        TS_ASSERT_EQUALS(a[0].toString(), "q1");
        TS_ASSERT_EQUALS(a[1].toString(), "q2");
        TS_ASSERT_EQUALS(a[2].toString(), "q3");
    }

    // Variants
    {
        mock.expectCall("render()");
        mock.provideReturnValue(afl::data::StringList_t());

        std::auto_ptr<server::Value_t> p(testee.call(Segment().pushBackString("ADDRMRENDER")));
        TS_ASSERT_EQUALS(Access(p).getArraySize(), 0U);
    }
    {
        mock.expectCall("render()");
        mock.provideReturnValue(afl::data::StringList_t());

        std::auto_ptr<server::Value_t> p(testee.call(Segment().pushBackString("addRmRendeR")));
        TS_ASSERT_EQUALS(Access(p).getArraySize(), 0U);
    }

    mock.checkFinish();
}

void
TestServerInterfaceTalkAddressServer::testErrors()
{
    TalkAddressMock mock("TestServerInterfaceTalkAddressServer::testErrors");
    server::interface::TalkAddressServer testee(mock);

    Segment empty;
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("foo")), std::exception);
}

void
TestServerInterfaceTalkAddressServer::testRoundtrip()
{
    TalkAddressMock mock("TestServerInterfaceTalkAddressServer::testRoundtrip");
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

        TS_ASSERT_EQUALS(out.size(), 2U);
        TS_ASSERT_EQUALS(out[0], "r1");
        TS_ASSERT_EQUALS(out[1], "r2");
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

        TS_ASSERT_EQUALS(out.size(), 3U);
        TS_ASSERT_EQUALS(out[0], "q1");
        TS_ASSERT_EQUALS(out[1], "q2");
        TS_ASSERT_EQUALS(out[2], "q3");
    }

    mock.checkFinish();
}

