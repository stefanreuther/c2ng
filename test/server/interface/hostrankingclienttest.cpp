/**
  *  \file test/server/interface/hostrankingclienttest.cpp
  *  \brief Test for server::interface::HostRankingClient
  */

#include "server/interface/hostrankingclient.hpp"

#include "afl/data/access.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"

using server::interface::HostRanking;
using afl::data::Access;

AFL_TEST("server.interface.HostRankingClient", a)
{
    afl::test::CommandHandler mock(a);
    server::interface::HostRankingClient testee(mock);
    std::auto_ptr<server::Value_t> p;

    // getUserList
    // Because getUserList returns a raw value, we don't need to mock it particularily careful.
    // - simple
    mock.expectCall("RANKLIST");
    mock.provideNewResult(0);
    AFL_CHECK_SUCCEEDS(a("01. getUserList"), p.reset(testee.getUserList(HostRanking::ListRequest())));
    a.checkNull("02. result", p.get());

    // - partial
    {
        HostRanking::ListRequest req;
        req.fieldsToGet.push_back("a");
        req.fieldsToGet.push_back("x");

        mock.expectCall("RANKLIST, FIELDS, a, x");
        mock.provideNewResult(server::makeIntegerValue(77));

        AFL_CHECK_SUCCEEDS(a("11. getUserList"), p.reset(testee.getUserList(req)));
        a.checkEqual("12. result", Access(p).toInteger(), 77);
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

        AFL_CHECK_SUCCEEDS(a("21. getUserList"), p.reset(testee.getUserList(req)));
        a.checkEqual("22. result", Access(p).toInteger(), 42);
    }

    mock.checkFinish();
}
