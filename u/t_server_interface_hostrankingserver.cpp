/**
  *  \file u/t_server_interface_hostrankingserver.cpp
  *  \brief Test for server::interface::HostRankingServer
  */

#include <stdexcept>
#include "server/interface/hostrankingserver.hpp"

#include "t_server_interface.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/interface/hostranking.hpp"
#include "server/interface/hostrankingclient.hpp"

using afl::data::Segment;
using afl::string::Format;
using server::Value_t;
using server::interface::HostRanking;

namespace {
    class HostRankingMock : public afl::test::CallReceiver, public HostRanking {
     public:
        HostRankingMock(afl::test::Assert a)
            : CallReceiver(a)
            { }

        virtual Value_t* getUserList(const ListRequest& req)
            {
                String_t cmd = "getUserList()";
                if (const String_t* p = req.sortField.get()) {
                    cmd += Format(" sort=%s", *p);
                }
                if (req.sortReverse) {
                    cmd += " reverse";
                }
                for (size_t i = 0; i < req.fieldsToGet.size(); ++i) {
                    cmd += Format(" get=%s", req.fieldsToGet[i]);
                }
                checkCall(cmd);
                return consumeReturnValue<Value_t*>();
            }
    };
}

void
TestServerInterfaceHostRankingServer::testIt()
{
    HostRankingMock mock("TestServerInterfaceHostRankingServer::testIt");
    server::interface::HostRankingServer testee(mock);

    // Calls
    mock.expectCall("getUserList()");
    mock.provideReturnValue((Value_t*) 0);
    testee.callVoid(Segment().pushBackString("RANKLIST"));

    mock.expectCall("getUserList()");
    mock.provideReturnValue((Value_t*) 0);
    testee.callVoid(Segment().pushBackString("ranklist"));

    mock.expectCall("getUserList() reverse");
    mock.provideReturnValue((Value_t*) 0);
    testee.callVoid(Segment().pushBackString("RANKLIST").pushBackString("REVERSE"));

    mock.expectCall("getUserList() sort=a reverse get=b get=c");
    mock.provideReturnValue((Value_t*) 0);
    testee.callVoid(Segment().pushBackString("RANKLIST").pushBackString("SORT").pushBackString("a").pushBackString("REVERSE").pushBackString("FIELDS").pushBackString("b").pushBackString("c"));

    mock.expectCall("getUserList() sort=a reverse get=b get=c");
    mock.provideReturnValue((Value_t*) 0);
    testee.callVoid(Segment().pushBackString("RANKLIST").pushBackString("sort").pushBackString("a").pushBackString("reverse").pushBackString("fields").pushBackString("b").pushBackString("c"));

    mock.expectCall("getUserList() reverse get=b get=c get=SORT get=a");
    mock.provideReturnValue((Value_t*) 0);
    testee.callVoid(Segment().pushBackString("RANKLIST").pushBackString("REVERSE").pushBackString("FIELDS").pushBackString("b").pushBackString("c").pushBackString("SORT").pushBackString("a"));

    // Return value
    mock.expectCall("getUserList()");
    mock.provideReturnValue(server::makeIntegerValue(42));
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("RANKLIST")), 42);

    mock.checkFinish();
}

void
TestServerInterfaceHostRankingServer::testErrors()
{
    HostRankingMock mock("TestServerInterfaceHostRankingServer::testErrors");
    server::interface::HostRankingServer testee(mock);

    Segment empty;
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("RANKLIST").pushBackString("SORT")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("RANKLIST").pushBackString("SORT")), std::exception);
}

void
TestServerInterfaceHostRankingServer::testRoundtrip()
{
    HostRankingMock mock("TestServerInterfaceHostRankingServer::testRoundtrip");
    server::interface::HostRankingServer level1(mock);
    server::interface::HostRankingClient level2(level1);;
    server::interface::HostRankingServer level3(level2);
    server::interface::HostRankingClient level4(level3);

    HostRanking::ListRequest req;
    req.sortField = "a";
    req.sortReverse = true;
    req.fieldsToGet.push_back("b");
    req.fieldsToGet.push_back("c");

    mock.expectCall("getUserList() sort=a reverse get=b get=c");
    mock.provideReturnValue(server::makeStringValue("the result"));

    std::auto_ptr<Value_t> p(level4.getUserList(req));

    TS_ASSERT_EQUALS(server::toString(p.get()), "the result");
    mock.checkFinish();
}

