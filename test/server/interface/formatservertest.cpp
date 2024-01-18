/**
  *  \file test/server/interface/formatservertest.cpp
  *  \brief Test for server::interface::FormatServer
  */

#include "server/interface/formatserver.hpp"

#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
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
AFL_TEST("server.interface.FormatServer:commands", a)
{
    FormatMock mock(a);
    server::interface::FormatServer testee(mock);

    // Extra commands
    a.check("01. help", testee.callString(Segment().pushBackString("HELP")).size() > 10);
    a.checkEqual("02. ping", testee.callString(Segment().pushBackString("PING")), "PONG");

    // Variations
    mock.expectCall("pack(infmt,data,outfmt,charset)");
    mock.provideReturnValue(server::makeIntegerValue(76));
    a.checkEqual("11. pack", testee.callInt(Segment().pushBackString("PACK").pushBackString("infmt").pushBackString("data").pushBackString("FORMAT").pushBackString("outfmt").pushBackString("CHARSET").pushBackString("charset")), 76);

    mock.expectCall("pack(infmt,data,no-format,no-charset)");
    mock.provideReturnValue(server::makeIntegerValue(75));
    a.checkEqual("21. pack", testee.callInt(Segment().pushBackString("PACK").pushBackString("infmt").pushBackString("data")), 75);

    mock.expectCall("unpack(infmt2,data2,outfmt2,charset2)");
    mock.provideReturnValue(server::makeIntegerValue(74));
    a.checkEqual("31. unpack", testee.callInt(Segment().pushBackString("UNPACK").pushBackString("infmt2").pushBackString("data2").pushBackString("CHARSET").pushBackString("charset2").pushBackString("FORMAT").pushBackString("outfmt2")), 74);

    mock.expectCall("unpack(infmt2,data2,no-format,charset2)");
    mock.provideReturnValue(server::makeIntegerValue(73));
    a.checkEqual("41. unpack", testee.callInt(Segment().pushBackString("unpack").pushBackString("infmt2").pushBackString("data2").pushBackString("charset").pushBackString("charset2")), 73);

    mock.checkFinish();
}

/** Test syntax errors. */
AFL_TEST("server.interface.FormatServer:errors", a)
{
    FormatMock mock(a);
    server::interface::FormatServer testee(mock);

    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    AFL_CHECK_THROWS(a("01. empty"),          testee.callVoid(empty), std::exception);
    AFL_CHECK_THROWS(a("02. bad verb"),       testee.callVoid(Segment().pushBackString("egal")), std::exception);
    AFL_CHECK_THROWS(a("03. missing arg"),    testee.callVoid(Segment().pushBackString("PACK")), std::exception);
    AFL_CHECK_THROWS(a("04. missing option"), testee.callVoid(Segment().pushBackString("PACK").pushBackString("a").pushBackString("b").pushBackString("FORMAT")), std::exception);
    AFL_CHECK_THROWS(a("05. bad option"),     testee.callVoid(Segment().pushBackString("PACK").pushBackString("a").pushBackString("b").pushBackString("what")), std::exception);
}
