/**
  *  \file u/t_server_interface_hostspecificationserver.cpp
  *  \brief Test for server::interface::HostSpecificationServer
  */

#include "server/interface/hostspecificationserver.hpp"

#include "t_server_interface.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
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
void
TestServerInterfaceHostSpecificationServer::testIt()
{
    HostSpecificationMock mock("testIt");
    server::interface::HostSpecificationServer testee(mock);

    // SPECSHIPLIST
    mock.expectCall("getShiplistData(mee,json,[beamspec,engspec])");
    mock.provideReturnValue(server::makeStringValue("{...}"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("SPECSHIPLIST").pushBackString("mee").pushBackString("json").pushBackString("beamspec").pushBackString("engspec")), "{...}");

    // SPECGAME
    mock.expectCall("getGameData(42,direct,[beamspec])");
    mock.provideReturnValue(server::makeStringValue("{x}"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("SPECGAME").pushBackInteger(42).pushBackString("direct").pushBackString("beamspec")), "{x}");

    // Variation: lower case
    mock.expectCall("getGameData(42,direct,[beamspec])");
    mock.provideReturnValue(server::makeStringValue("{x}"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("specgame").pushBackInteger(42).pushBackString("direct").pushBackString("beamspec")), "{x}");

    mock.checkFinish();
}

/** Test errors. */
void
TestServerInterfaceHostSpecificationServer::testError()
{
    HostSpecificationMock mock("testError");
    server::interface::HostSpecificationServer testee(mock);

    // Command verb missing
    Segment empty;
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);

    // Bad verb
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("foo")), std::exception);

    // Too few args
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("SPECSHIPLIST").pushBackString("mee").pushBackString("json")), std::exception);

    // Type error
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("SPECGAME").pushBackString("mee").pushBackString("json").pushBackString("beamspec")), std::exception);

    // Bad format
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("SPECGAME").pushBackInteger(42).pushBackString("XML").pushBackString("beamspec")), std::exception);
}

/** Test roundtrip with HostSpecificationClient. */
void
TestServerInterfaceHostSpecificationServer::testRoundtrip()
{
    HostSpecificationMock mock("testRoundtrip");
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
        TS_ASSERT_EQUALS(server::toString(p.get()), "{...}");
    }

    // SPECGAME
    mock.expectCall("getGameData(23,direct,[beamspec])");
    mock.provideReturnValue(server::makeStringValue("{x}"));
    {
        afl::data::StringList_t list;
        list.push_back("beamspec");
        std::auto_ptr<Value_t> p(level4.getGameData(23, server::interface::HostSpecificationClient::Direct, list));
        TS_ASSERT_EQUALS(server::toString(p.get()), "{x}");
    }
}

