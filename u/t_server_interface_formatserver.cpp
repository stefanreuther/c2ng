/**
  *  \file u/t_server_interface_formatserver.cpp
  *  \brief Test for server::interface::FormatServer
  */

#include "server/interface/formatserver.hpp"

#include "t_server_interface.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/interface/format.hpp"
#include "server/types.hpp"

using afl::data::Segment;

namespace {
    class FormatMock : public server::interface::Format, public afl::test::CallReceiver {
     public:
        FormatMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual afl::data::Value* pack(String_t formatName, afl::data::Value* data, afl::base::Optional<String_t> format, afl::base::Optional<String_t> charset)
            {
                checkCall(afl::string::Format("pack(%s,%s,%s,%s)", formatName, server::toString(data), format.orElse("no-format"), charset.orElse("no-charset")));
                return consumeReturnValue<afl::data::Value*>();
            }
        virtual afl::data::Value* unpack(String_t formatName, afl::data::Value* data, afl::base::Optional<String_t> format, afl::base::Optional<String_t> charset)
            {
                checkCall(afl::string::Format("unpack(%s,%s,%s,%s)", formatName, server::toString(data), format.orElse("no-format"), charset.orElse("no-charset")));
                return consumeReturnValue<afl::data::Value*>();
            }
        void provideReturnValue(afl::data::Value* p)
            { CallReceiver::provideReturnValue(p); }
    };
}

/** Test it. */
void
TestServerInterfaceFormatServer::testIt()
{
    FormatMock mock("testIt");
    server::interface::FormatServer testee(mock);

    // Extra commands
    TS_ASSERT(testee.callString(Segment().pushBackString("HELP")).size() > 10);
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("PING")), "PONG");

    // Variations
    mock.expectCall("pack(infmt,data,outfmt,charset)");
    mock.provideReturnValue(server::makeIntegerValue(76));
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("PACK").pushBackString("infmt").pushBackString("data").pushBackString("FORMAT").pushBackString("outfmt").pushBackString("CHARSET").pushBackString("charset")), 76);

    mock.expectCall("pack(infmt,data,no-format,no-charset)");
    mock.provideReturnValue(server::makeIntegerValue(75));
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("PACK").pushBackString("infmt").pushBackString("data")), 75);

    mock.expectCall("unpack(infmt2,data2,outfmt2,charset2)");
    mock.provideReturnValue(server::makeIntegerValue(74));
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("UNPACK").pushBackString("infmt2").pushBackString("data2").pushBackString("CHARSET").pushBackString("charset2").pushBackString("FORMAT").pushBackString("outfmt2")), 74);

    mock.expectCall("unpack(infmt2,data2,no-format,charset2)");
    mock.provideReturnValue(server::makeIntegerValue(73));
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("unpack").pushBackString("infmt2").pushBackString("data2").pushBackString("charset").pushBackString("charset2")), 73);

    mock.checkFinish();
}

/** Test syntax errors. */
void
TestServerInterfaceFormatServer::testErrors()
{
    FormatMock mock("testErrors");
    server::interface::FormatServer testee(mock);

    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("egal")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("PACK")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("PACK").pushBackString("a").pushBackString("b").pushBackString("FORMAT")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("PACK").pushBackString("a").pushBackString("b").pushBackString("what")), std::exception);
}
