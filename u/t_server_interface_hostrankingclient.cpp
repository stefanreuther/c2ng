/**
  *  \file u/t_server_interface_hostrankingclient.cpp
  *  \brief Test for server::interface::HostRankingClient
  */

#include "server/interface/hostrankingclient.hpp"
#include "afl/test/commandhandler.hpp"

#include "t_server_interface.hpp"
#include "afl/data/access.hpp"

using server::interface::HostRanking;
using afl::data::Access;

void
TestServerInterfaceHostRankingClient::testIt()
{
    afl::test::CommandHandler mock("testIt");
    server::interface::HostRankingClient testee(mock);
    std::auto_ptr<server::Value_t> p;

    // getUserList
    // Because getUserList returns a raw value, we don't need to mock it particularily careful.
    // - simple
    mock.expectCall("RANKLIST");
    mock.provideNewResult(0);
    TS_ASSERT_THROWS_NOTHING(p.reset(testee.getUserList(HostRanking::ListRequest())));
    TS_ASSERT(p.get() == 0);

    // - partial
    {
        HostRanking::ListRequest req;
        req.fieldsToGet.push_back("a");
        req.fieldsToGet.push_back("x");

        mock.expectCall("RANKLIST, FIELDS, a, x");
        mock.provideNewResult(server::makeIntegerValue(77));

        TS_ASSERT_THROWS_NOTHING(p.reset(testee.getUserList(req)));
        TS_ASSERT_EQUALS(Access(p).toInteger(), 77);
    }

    // - full
    {
        HostRanking::ListRequest req;
        req.sortField = "a";
        req.fieldsToGet.push_back("e");
        req.fieldsToGet.push_back("i");
        req.sortReverse = true;

        mock.expectCall("RANKLIST, SORT, a, REVERSE, FIELDS, e, i");
        mock.provideNewResult(server::makeIntegerValue(42));

        TS_ASSERT_THROWS_NOTHING(p.reset(testee.getUserList(req)));
        TS_ASSERT_EQUALS(Access(p).toInteger(), 42);
    }

    mock.checkFinish();
}

