/**
  *  \file test/server/host/hostcrontest.cpp
  *  \brief Test for server::host::HostCron
  */

#include "server/host/hostcron.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/host/cron.hpp"
#include "server/host/hostgame.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "util/processrunner.hpp"

using afl::net::redis::HashKey;
using afl::net::redis::IntegerSetKey;
using afl::net::redis::StringKey;
using afl::string::Format;
using server::host::HostCron;
using server::host::HostGame;

namespace {
    /* Cron mock. */
    class CronMock : public server::host::Cron, public afl::test::CallReceiver {
     public:
        CronMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual Event_t getGameEvent(int32_t gameId)
            {
                checkCall(Format("getGameEvent(%d)", gameId));
                return consumeReturnValue<Event_t>();
            }
        virtual void listGameEvents(std::vector<Event_t>& result)
            {
                checkCall("listGameEvents()");
                int n = consumeReturnValue<int>();
                while (n-- > 0) {
                    result.push_back(consumeReturnValue<Event_t>());
                }
            }
        virtual void handleGameChange(int32_t gameId)
            { checkCall(Format("handleGameChange(%d)", gameId)); }

        virtual void suspendScheduler(server::Time_t absTime)
            { checkCall(Format("suspendScheduler(%d)", absTime ? 1 : 0)); }

        void provideSampleList();
    };

    /* Test harness. */
    class TestHarness {
     public:
        TestHarness()
            : m_db(), m_null(), m_mail(m_null), m_runner(), m_fs(), m_root(m_db, m_null, m_null, m_mail, m_runner, m_fs, server::host::Configuration())
            { }

        server::host::Root& root()
            { return m_root; }

        afl::net::CommandHandler& db()
            { return m_db; }

        void createGame(int32_t id, HostGame::Type type);

     private:
        afl::net::redis::InternalDatabase m_db;
        afl::net::NullCommandHandler m_null;
        server::interface::MailQueueClient m_mail;
        util::ProcessRunner m_runner;
        afl::io::NullFileSystem m_fs;
        server::host::Root m_root;
    };
}

void
CronMock::provideSampleList()
{
    expectCall("listGameEvents()");
    provideReturnValue(4);
    provideReturnValue(HostCron::Event(1, HostCron::MasterAction, 100));
    provideReturnValue(HostCron::Event(2, HostCron::ScheduleChangeAction, 200));
    provideReturnValue(HostCron::Event(3, HostCron::HostAction, 300));
    provideReturnValue(HostCron::Event(4, HostCron::MasterAction, 400));
}

void
TestHarness::createGame(int32_t id, HostGame::Type type)
{
    // Indexes
    IntegerSetKey(m_db, "game:all").add(id);
    IntegerSetKey(m_db, "game:state:joining").add(id);
    if (type == HostGame::PublicGame) {
        IntegerSetKey(m_db, "game:pubstate:joining").add(id);
    }
    IntegerSetKey(m_db, Format("game:type:%s", HostGame::formatType(type))).add(id);

    // Game
    StringKey(m_db, Format("game:%d:type", id)).set(HostGame::formatType(type));
    StringKey(m_db, Format("game:%d:state", id)).set("joining");
}


/** Test operation without a cron instance ("-notx"). */
AFL_TEST("server.host.HostCron:no-cron", a)
{
    // Setup
    TestHarness h;
    server::host::Session s;
    HostCron testee(s, h.root());

    // - Game 39 is broken (for the kickstart test)
    // - Games 12,39,99 must exist for the commands to go through
    IntegerSetKey(h.db(), "game:broken").add(39);
    IntegerSetKey(h.db(), "game:all").add(12);
    IntegerSetKey(h.db(), "game:all").add(39);
    IntegerSetKey(h.db(), "game:all").add(99);

    // Test
    HostCron::Event e = testee.getGameEvent(99);
    a.checkEqual("01. action", e.action, HostCron::NoAction);
    a.checkEqual("02. gameId", e.gameId, 99);
    a.checkEqual("03. time",   e.time, 0);

    std::vector<HostCron::Event> list;
    testee.listGameEvents(afl::base::Nothing, list);
    a.checkEqual("11. size", list.size(), 0U);

    // Kickstart
    a.checkEqual("21. kickstartGame", testee.kickstartGame(12), false);
    a.checkEqual("22. kickstartGame", testee.kickstartGame(39), true);
    a.checkEqual("23. broken", IntegerSetKey(h.db(), "game:broken").contains(39), false);

    // Suspend
    AFL_CHECK_SUCCEEDS(a("31. suspendScheduler"), testee.suspendScheduler(0));
    AFL_CHECK_SUCCEEDS(a("32. suspendScheduler"), testee.suspendScheduler(1));
}

/** Test operation with a cron instance (standard). */
AFL_TEST("server.host.HostCron:normal", a)
{
    CronMock m(a);
    TestHarness h;
    server::host::Session s;
    h.root().setCron(&m);
    server::host::HostCron testee(s, h.root());

    // - Game 39 is broken (for the kickstart test)
    // - Games 12,39,99 must exist for the commands to go through
    IntegerSetKey(h.db(), "game:broken").add(39);
    IntegerSetKey(h.db(), "game:all").add(12);
    IntegerSetKey(h.db(), "game:all").add(39);
    IntegerSetKey(h.db(), "game:all").add(99);

    // Test
    m.expectCall("getGameEvent(99)");
    m.provideReturnValue(HostCron::Event(99, HostCron::ScheduleChangeAction, 1234567));
    HostCron::Event e = testee.getGameEvent(99);
    a.checkEqual("01. action", e.action, HostCron::ScheduleChangeAction);
    a.checkEqual("02. gameId", e.gameId, 99);
    a.checkEqual("03. time",   e.time, 1234567);

    // List
    // - return entire list
    {
        m.expectCall("listGameEvents()");
        m.provideReturnValue(2);
        m.provideReturnValue(HostCron::Event(99, HostCron::ScheduleChangeAction, 1234567));
        m.provideReturnValue(HostCron::Event(12, HostCron::MasterAction,         2345678));
        std::vector<HostCron::Event> list;
        testee.listGameEvents(afl::base::Nothing, list);
        a.checkEqual("11. size", list.size(), 2U);
        a.checkEqual("12. list", list[0].gameId, 99);
        a.checkEqual("13. list", list[1].gameId, 12);
    }
    // - return trimmed list
    {
        m.expectCall("listGameEvents()");
        m.provideReturnValue(2);
        m.provideReturnValue(HostCron::Event(99, HostCron::ScheduleChangeAction, 1234567));
        m.provideReturnValue(HostCron::Event(12, HostCron::MasterAction,         2345678));
        std::vector<HostCron::Event> list;
        testee.listGameEvents(1, list);
        a.checkEqual("14. size", list.size(), 1U);
        a.checkEqual("15. list", list[0].gameId, 99);
    }

    // Kickstart
    // - does not go through
    a.checkEqual("21. kickstartGame", testee.kickstartGame(12), false);

    // - goes through
    m.expectCall("handleGameChange(39)");
    a.checkEqual("31. kickstartGame", testee.kickstartGame(39), true);
    a.checkEqual("32. broken", IntegerSetKey(h.db(), "game:broken").contains(39), false);

    // Suspend
    m.expectCall("suspendScheduler(0)");
    AFL_CHECK_SUCCEEDS(a("41. suspendScheduler"), testee.suspendScheduler(0));
    m.expectCall("suspendScheduler(1)");
    AFL_CHECK_SUCCEEDS(a("42. suspendScheduler"), testee.suspendScheduler(77));

    m.checkFinish();
}

/** Test listGameEvents() operation with permissions. */
AFL_TEST("server.host.HostCron:listGameEvents:permissions", a)
{
    CronMock m(a);
    TestHarness h;
    h.root().setCron(&m);

    // Create games
    h.createGame(1, HostGame::PublicGame);
    h.createGame(2, HostGame::PrivateGame);
    h.createGame(3, HostGame::PublicGame);
    h.createGame(4, HostGame::PublicGame);

    // Game 2 is owned by user "u", and played by user "p".
    StringKey(h.db(), "game:2:owner").set("u");
    HashKey(h.db(), "game:2:users").intField("p").set(0);

    // Test as admin
    {
        server::host::Session s;
        server::host::HostCron testee(s, h.root());
        m.provideSampleList();

        std::vector<HostCron::Event> list;
        testee.listGameEvents(afl::base::Nothing, list);

        a.checkEqual("01. size", list.size(), 4U);
        a.checkEqual("02. list", list[0].gameId, 1);
        a.checkEqual("03. list", list[1].gameId, 2);
        a.checkEqual("04. list", list[2].gameId, 3);
        a.checkEqual("05. list", list[3].gameId, 4);
    }

    // Test as user "u": gets 4 results
    {
        server::host::Session s;
        server::host::HostCron testee(s, h.root());
        m.provideSampleList();
        s.setUser("u");

        std::vector<HostCron::Event> list;
        testee.listGameEvents(afl::base::Nothing, list);

        a.checkEqual("11. size", list.size(), 4U);
    }

    // Test as user "p": gets 4 results
    {
        server::host::Session s;
        server::host::HostCron testee(s, h.root());
        m.provideSampleList();
        s.setUser("p");

        std::vector<HostCron::Event> list;
        testee.listGameEvents(afl::base::Nothing, list);

        a.checkEqual("21. size", list.size(), 4U);
    }

    // Test as user "o": gets 3 results
    {
        server::host::Session s;
        server::host::HostCron testee(s, h.root());
        m.provideSampleList();
        s.setUser("o");

        std::vector<HostCron::Event> list;
        testee.listGameEvents(afl::base::Nothing, list);

        a.checkEqual("31. size", list.size(), 3U);
        a.checkEqual("32. list", list[0].gameId, 1);
        a.checkEqual("33. list", list[1].gameId, 3);
        a.checkEqual("34. list", list[2].gameId, 4);
    }

    // Test as user "o" with limit
    {
        server::host::Session s;
        server::host::HostCron testee(s, h.root());
        m.provideSampleList();
        s.setUser("o");

        std::vector<HostCron::Event> list;
        testee.listGameEvents(2, list);

        a.checkEqual("41. size", list.size(), 2U);
        a.checkEqual("42. list", list[0].gameId, 1);
        a.checkEqual("43. list", list[1].gameId, 3);
    }
}
