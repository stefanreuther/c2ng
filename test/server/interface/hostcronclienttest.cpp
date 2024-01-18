/**
  *  \file test/server/interface/hostcronclienttest.cpp
  *  \brief Test for server::interface::HostCronClient
  */

#include "server/interface/hostcronclient.hpp"

#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"

using afl::data::Vector;
using afl::data::VectorValue;
using afl::data::Hash;
using afl::data::HashValue;
using server::interface::HostCron;

/** Simple test. */
AFL_TEST("server.interface.HostCronClient", a)
{
    afl::test::CommandHandler mock(a);
    server::interface::HostCronClient testee(mock);

    // getGameEvent - null (default) return
    {
        mock.expectCall("CRONGET, 39");
        mock.provideNewResult(0);

        HostCron::Event e = testee.getGameEvent(39);
        a.checkEqual("01. action", e.action, HostCron::UnknownAction);
        a.checkEqual("02. time",   e.time, 0);
        a.checkEqual("03. gameId", e.gameId, 0);
    }

    // getGameEvent - no event
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("action", server::makeStringValue("none"));
        mock.expectCall("CRONGET, 1");
        mock.provideNewResult(new HashValue(h));

        HostCron::Event e = testee.getGameEvent(1);
        a.checkEqual("11. action", e.action, HostCron::NoAction);
        a.checkEqual("12. time",   e.time, 0);
        a.checkEqual("13. gameId", e.gameId, 0);
    }

    // getGameEvent - schedule change
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("action", server::makeStringValue("schedulechange"));
        h->setNew("game", server::makeIntegerValue(2));
        h->setNew("time", server::makeIntegerValue(11223322));
        mock.expectCall("CRONGET, 2");
        mock.provideNewResult(new HashValue(h));

        HostCron::Event e = testee.getGameEvent(2);
        a.checkEqual("21. action", e.action, HostCron::ScheduleChangeAction);
        a.checkEqual("22. time",   e.time, 11223322);
        a.checkEqual("23. gameId", e.gameId, 2);
    }

    // getGameEvent - host
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("action", server::makeStringValue("host"));
        h->setNew("game", server::makeIntegerValue(3));
        h->setNew("time", server::makeIntegerValue(11223355));
        mock.expectCall("CRONGET, 3");
        mock.provideNewResult(new HashValue(h));

        HostCron::Event e = testee.getGameEvent(3);
        a.checkEqual("31. action", e.action, HostCron::HostAction);
        a.checkEqual("32. time",   e.time, 11223355);
        a.checkEqual("33. gameId", e.gameId, 3);
    }

    // getGameEvent - master
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("action", server::makeStringValue("master"));
        h->setNew("game", server::makeIntegerValue(4));
        h->setNew("time", server::makeIntegerValue(11223344));
        mock.expectCall("CRONGET, 4");
        mock.provideNewResult(new HashValue(h));

        HostCron::Event e = testee.getGameEvent(4);
        a.checkEqual("41. action", e.action, HostCron::MasterAction);
        a.checkEqual("42. time",   e.time, 11223344);
        a.checkEqual("43. gameId", e.gameId, 4);
    }

    // listGameEvents - empty
    {
        mock.expectCall("CRONLIST");
        mock.provideNewResult(0);

        std::vector<HostCron::Event> es;
        testee.listGameEvents(afl::base::Nothing, es);
        a.check("51. empty", es.empty());
    }

    // listGameEvents - empty, with limit
    {
        mock.expectCall("CRONLIST, LIMIT, 9");
        mock.provideNewResult(0);

        std::vector<HostCron::Event> es;
        testee.listGameEvents(9, es);
        a.check("61. empty", es.empty());
    }

    // listGameEvents - with result
    {
        Hash::Ref_t h1 = Hash::create();
        h1->setNew("action", server::makeStringValue("master"));
        h1->setNew("game", server::makeIntegerValue(4));
        h1->setNew("time", server::makeIntegerValue(11223344));

        Hash::Ref_t h2 = Hash::create();
        h2->setNew("action", server::makeStringValue("host"));
        h2->setNew("game", server::makeIntegerValue(9));
        h2->setNew("time", server::makeIntegerValue(11223355));

        Vector::Ref_t vec = Vector::create();
        vec->pushBackNew(new HashValue(h1));
        vec->pushBackNew(new HashValue(h2));

        mock.expectCall("CRONLIST, LIMIT, 7");
        mock.provideNewResult(new VectorValue(vec));

        std::vector<HostCron::Event> es;
        testee.listGameEvents(7, es);

        a.checkEqual("71. size",   es.size(), 2U);
        a.checkEqual("72. action", es[0].action, HostCron::MasterAction);
        a.checkEqual("73. time",   es[0].time, 11223344);
        a.checkEqual("74. gameId", es[0].gameId, 4);
        a.checkEqual("75. action", es[1].action, HostCron::HostAction);
        a.checkEqual("76. time",   es[1].time, 11223355);
        a.checkEqual("77. gameId", es[1].gameId, 9);
    }

    // kickstartGame
    {
        mock.expectCall("CRONKICK, 92");
        mock.provideNewResult(server::makeIntegerValue(1));
        bool ok = testee.kickstartGame(92);
        a.check("81. kickstartGame", ok);
    }

    // suspendScheduler
    {
        mock.expectCall("CRONSUSPEND, 15");
        mock.provideNewResult(server::makeStringValue("x"));
        testee.suspendScheduler(15);
    }

    // getBrokenGames
    {
        Vector::Ref_t vec = Vector::create();
        vec->pushBackInteger(10);
        vec->pushBackString("x");
        vec->pushBackInteger(15);
        vec->pushBackString("y");
        vec->pushBackInteger(77);
        vec->pushBackString("z");
        mock.expectCall("CRONLSBROKEN");
        mock.provideNewResult(new VectorValue(vec));

        server::interface::HostCron::BrokenMap_t m;
        testee.getBrokenGames(m);

        a.checkEqual("91. size", m.size(), 3U);
        a.checkEqual("92. result", m[10], "x");
        a.checkEqual("93. result", m[15], "y");
        a.checkEqual("94. result", m[77], "z");
    }

    mock.checkFinish();
}
