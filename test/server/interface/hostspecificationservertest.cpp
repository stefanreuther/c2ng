/**
  *  \file test/server/interface/hostspecificationservertest.cpp
  *  \brief Test for server::interface::HostSpecificationServer
  */

#include "server/interface/hostspecificationserver.hpp"

#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/hostspecification.hpp"
#include "server/interface/hostspecificationclient.hpp"

using afl::data::Segment;
using server::interface::HostSpecification;
using server::Value_t;

namespace {
    class HostSpecificationMock : public afl::test::CallReceiver, public HostSpecification {
     public:
        HostSpecificationMock(afl::test::Assert a)
            : CallReceiver(a)
            { }

        virtual Value_t* getShiplistData(String_t shiplistId, Format format, const afl::data::StringList_t& keys)
            {
                checkCall(afl::string::Format("getShiplistData(%s,%s,%s)", shiplistId, formatFormat(format), toString(keys)));
                return consumeReturnValue<Value_t*>();
            }

        virtual Value_t* getGameData(int32_t gameId, Format format, const afl::data::StringList_t& keys)
            {
                checkCall(afl::string::Format("getGameData(%d,%s,%s)", gameId, formatFormat(format), toString(keys)));
                return consumeReturnValue<Value_t*>();
            }

     private:
        static String_t toString(const afl::data::StringList_t& keys)
            {
                String_t result = "[";
                for (size_t i = 0; i < keys.size(); ++i) {
                    if (i != 0) {
                        result += ",";
                    }
                    result += keys[i];
                }
                result += "]";
                return result;
            }
    };
}

/** Test successful calls. */
AFL_TEST("server.interface.HostSpecificationServer:commands", a)
{
    HostSpecificationMock mock(a);
    server::interface::HostSpecificationServer testee(mock);

    // SPECSHIPLIST
    mock.expectCall("getShiplistData(mee,json,[beamspec,engspec])");
    mock.provideReturnValue(server::makeStringValue("{...}"));
    a.checkEqual("01. specshiplist", testee.callString(Segment().pushBackString("SPECSHIPLIST").pushBackString("mee").pushBackString("json").pushBackString("beamspec").pushBackString("engspec")), "{...}");

    // SPECGAME
    mock.expectCall("getGameData(42,direct,[beamspec])");
    mock.provideReturnValue(server::makeStringValue("{x}"));
    a.checkEqual("11. specgame", testee.callString(Segment().pushBackString("SPECGAME").pushBackInteger(42).pushBackString("direct").pushBackString("beamspec")), "{x}");

    // Variation: lower case
    mock.expectCall("getGameData(42,direct,[beamspec])");
    mock.provideReturnValue(server::makeStringValue("{x}"));
    a.checkEqual("21. specgame", testee.callString(Segment().pushBackString("specgame").pushBackInteger(42).pushBackString("direct").pushBackString("beamspec")), "{x}");

    mock.checkFinish();
}

/** Test errors. */
AFL_TEST("server.interface.HostSpecificationServer:error", a)
{
    HostSpecificationMock mock(a);
    server::interface::HostSpecificationServer testee(mock);

    // Command verb missing
    Segment empty;
    AFL_CHECK_THROWS(a("01. empty"), testee.callVoid(empty), std::exception);

    // Bad verb
    AFL_CHECK_THROWS(a("11. bad verb"), testee.callVoid(Segment().pushBackString("foo")), std::exception);

    // Too few args
    AFL_CHECK_THROWS(a("21. missing arg"), testee.callVoid(Segment().pushBackString("SPECSHIPLIST").pushBackString("mee").pushBackString("json")), std::exception);

    // Type error
    AFL_CHECK_THROWS(a("31. type error"), testee.callVoid(Segment().pushBackString("SPECGAME").pushBackString("mee").pushBackString("json").pushBackString("beamspec")), std::exception);

    // Bad format
    AFL_CHECK_THROWS(a("41. bad format"), testee.callVoid(Segment().pushBackString("SPECGAME").pushBackInteger(42).pushBackString("XML").pushBackString("beamspec")), std::exception);
}

/** Test roundtrip with HostSpecificationClient. */
AFL_TEST("server.interface.HostSpecificationServer:roundtrip", a)
{
    HostSpecificationMock mock(a);
    server::interface::HostSpecificationServer level1(mock);
    server::interface::HostSpecificationClient level2(level1);
    server::interface::HostSpecificationServer level3(level2);
    server::interface::HostSpecificationClient level4(level3);

    // SPECSHIPLIST
    mock.expectCall("getShiplistData(mee,json,[beamspec,engspec])");
    mock.provideReturnValue(server::makeStringValue("{...}"));
    {
        afl::data::StringList_t list;
        list.push_back("beamspec");
        list.push_back("engspec");
        std::auto_ptr<Value_t> p(level4.getShiplistData("mee", server::interface::HostSpecificationClient::JsonString, list));
        a.checkEqual("01. getShiplistData", server::toString(p.get()), "{...}");
    }

    // SPECGAME
    mock.expectCall("getGameData(23,direct,[beamspec])");
    mock.provideReturnValue(server::makeStringValue("{x}"));
    {
        afl::data::StringList_t list;
        list.push_back("beamspec");
        std::auto_ptr<Value_t> p(level4.getGameData(23, server::interface::HostSpecificationClient::Direct, list));
        a.checkEqual("11. getGameData", server::toString(p.get()), "{x}");
    }
}

