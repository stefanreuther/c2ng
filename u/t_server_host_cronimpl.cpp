/**
  *  \file u/t_server_host_cronimpl.cpp
  *  \brief Test for server::host::CronImpl
  */

#include "server/host/cronimpl.hpp"

#include "t_server_host.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/integerlistkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/string/format.hpp"
#include "server/host/game.hpp"
#include "server/host/root.hpp"
#include "server/interface/hostcron.hpp"
#include "server/interface/hostgame.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "util/processrunner.hpp"

using afl::net::redis::HashKey;
using afl::net::redis::IntegerListKey;
using afl::net::redis::Subtree;
using afl::string::Format;
using server::host::Game;
using server::interface::HostCron;
using server::interface::HostGame;

namespace {
    const int MINUTES_PER_DAY = 60*24;

    class TestHarness {
     public:
        TestHarness()
            : m_db(), m_null(), m_mail(m_null), m_runner(), m_fs(), m_root(m_db, m_null, m_null, m_mail, m_runner, m_fs, server::host::Configuration())
            { createPlayers(); }

        server::host::Root& root()
            { return m_root; }

        afl::net::CommandHandler& db()
            { return m_db; }

        void createPlayers();
        void createGame(int gameId, HostGame::State state);
        void addPlayer(int gameId, int slot, int playerId);
        void setTurnState(int gameId, int slot, int turnState);
        void setGameConfig(int gameId, const char* key, int value);

        void setSchedule(int gameId, int scheduleId, const char* key, int value);
        void addSchedule(int gameId, int scheduleId);

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
TestHarness::createPlayers()
{
    for (int p = 601; p <= 650; ++p) {
        Subtree u(db(), "user:");
        u.subtree(p).stringKey("name").set(Format("test_user_%d", p));
        u.subtree(p).hashKey("profile").stringField("realname").set(Format("Test User %d", p));
        u.subtree(p).hashKey("profile").stringField("screenname").set(Format("Test User %d", p));
        u.subtree(p).hashKey("profile").intField("turnreliability").set(90000);
        u.subtree(p).hashKey("profile").intField("turnsplayed").set(100);
        u.subtree(p).hashKey("profile").intField("turnsmissed").set(5);
    }
}

void
TestHarness::createGame(int gameId, HostGame::State state)
{
    const String_t stateName = HostGame::formatState(state);

    // Create game
    Subtree g(db(), "game:");
    Subtree u(db(), "user:");
    g.subtree(gameId).stringKey("name").set("Test Game");
    g.subtree(gameId).stringKey("state").set(stateName);
    g.subtree(gameId).stringKey("type").set("public");

    // Create slots
    for (int s = 1; s <= 11; ++s) {
        g.subtree(gameId).subtree("player").subtree(s).hashKey("status").intField("slot").set(1);
        g.subtree(gameId).subtree("player").subtree(s).hashKey("status").intField("turn").set(0);
    }

    // Indexes
    g.intSetKey("all").add(gameId);
    g.intSetKey("state:" + stateName).add(gameId);
    g.intSetKey("pubstate:" + stateName).add(gameId);
}

void
TestHarness::addPlayer(int gameId, int slot, int playerId)
{
    Game(root(), gameId).pushPlayerSlot(slot, Format("%d", playerId), root());
}

void
TestHarness::setTurnState(int gameId, int slot, int turnState)
{
    Subtree(db(), "game:").subtree(gameId).subtree("player").subtree(slot).hashKey("status").intField("turn").set(turnState);
}

void
TestHarness::setGameConfig(int gameId, const char* key, int value)
{
    HashKey(db(), Format("game:%d:settings", gameId)).intField(key).set(value);
}

void
TestHarness::setSchedule(int gameId, int scheduleId, const char* key, int value)
{
    HashKey(db(), Format("game:%d:schedule:%d", gameId, scheduleId)).intField(key).set(value);
}

void
TestHarness::addSchedule(int gameId, int scheduleId)
{
    IntegerListKey(db(), Format("game:%d:schedule:list", gameId)).pushFront(scheduleId);
}

/********************************* Tests *********************************/

/** Test Master time computation for empty game.
    This must not produce a schedule. */
void
TestServerHostCronImpl::testMaster()
{
    const int GAME_ID = 37;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Joining);
    h.setGameConfig(GAME_ID, "lastPlayerJoined", 100);

    // Game has no players yet and thus generates no schedule.
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(105, h.root(), GAME_ID, sch);
    TS_ASSERT(sch.empty());
}

/** Test Master time computation for fully-joined game.
    This must produce the correct Master action. */
void
TestServerHostCronImpl::testMasterJoin()
{
    const int GAME_ID = 37;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Joining);

    // Join some players
    h.setGameConfig(GAME_ID, "lastPlayerJoined", 100);
    for (int i = 1; i <= 11; ++i) {
        h.addPlayer(GAME_ID, i, 600+i);
    }

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(105, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::MasterAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, 115 /* lastPlayerJoined=100 plus MASTER_DELAY=15 */);
}

/** Test Master time computation for fully-joined game without proper timestamp.
    This must produce the correct Master action, runnable "now" due to lack of better information. */
void
TestServerHostCronImpl::testMasterJoinTimeless()
{
    const int GAME_ID = 37;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Joining);

    // Join some players but don't provide a lastPlayerJoined time
    for (int i = 1; i <= 11; ++i) {
        h.addPlayer(GAME_ID, i, 600+i);
    }

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(108, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::MasterAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, 108 /* current time */);
}

/** Test Preparing state.
    Cron must not create a time. */
void
TestServerHostCronImpl::testPreparing()
{
    const int GAME_ID = 200;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Preparing);

    // Join some players and submit their turns
    h.setGameConfig(GAME_ID, "lastPlayerJoined", 100);
    for (int i = 1; i <= 11; ++i) {
        h.addPlayer(GAME_ID, i, 600+i);
        h.setTurnState(GAME_ID, i, Game::TurnGreen);
    }

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(108, h.root(), GAME_ID, sch);
    TS_ASSERT(sch.empty());
}

/** Test Finished state.
    Cron must not create a time. */
void
TestServerHostCronImpl::testFinished()
{
    const int GAME_ID = 500;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Finished);

    // Join some players and submit their turns
    h.setGameConfig(GAME_ID, "lastPlayerJoined", 100);
    for (int i = 1; i <= 11; ++i) {
        h.addPlayer(GAME_ID, i, 600+i);
        h.setTurnState(GAME_ID, i, Game::TurnGreen);
    }

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(108, h.root(), GAME_ID, sch);
    TS_ASSERT(sch.empty());
}

/** Test Running game, initial state.
    Master runs immediately. */
void
TestServerHostCronImpl::testRunningInitial()
{
    const int GAME_ID = 257;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(105, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::MasterAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, 105);
}

/** Test Running game, initial state #2.
    Host runs immediately. */
void
TestServerHostCronImpl::testRunningInitial2()
{
    const int GAME_ID = 257;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(105, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::HostAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, 105);
}

/** Test Running game with no schedule.
    Must not generate a schedule. */
void
TestServerHostCronImpl::testRunningNoSchedule()
{
    const int GAME_ID = 500;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);

    // Join some players and submit their turns
    for (int i = 1; i <= 11; ++i) {
        h.addPlayer(GAME_ID, i, 600+i);
        h.setTurnState(GAME_ID, i, Game::TurnGreen);
    }

    // Must set a turn number and last host time.
    // Otherwise, scheduler assumes that game was never hosted, and runs host immediately
    // (to fulfill the implied invariant that a Running game should have seen at least one host run).
    h.setGameConfig(GAME_ID, "turn", 30);
    h.setGameConfig(GAME_ID, "lastHostTime", 20);

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(108, h.root(), GAME_ID, sch);
    TS_ASSERT(sch.empty());
}

/** Test weekly schedule, normal case.
    Configures a "every 4 days" schedule, which must produce a host after 4 days. */
void
TestServerHostCronImpl::testRunningWeeklyNormal()
{
    const int GAME_ID = 7;
    const int DAYTIME = 400;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);
    h.setGameConfig(GAME_ID, "lastHostTime", 500*MINUTES_PER_DAY + DAYTIME + 3 /* random jitter */);
    h.setSchedule(GAME_ID, 3, "type", 2 /* weekly */);
    h.setSchedule(GAME_ID, 3, "interval", 4 /* days */);
    h.setSchedule(GAME_ID, 3, "daytime", DAYTIME);
    h.setSchedule(GAME_ID, 3, "hostLimit", 300);
    h.addSchedule(GAME_ID, 3);

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(200, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::HostAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, (500 + 4)*MINUTES_PER_DAY + DAYTIME);
}

/** Test weekly schedule, delayed host.
    Configures a "every 4 days" schedule but delays host more than allowed.
    Scheduler must produce a host after 5 days. */
void
TestServerHostCronImpl::testRunningWeeklyDelayed()
{
    const int GAME_ID = 7;
    const int DAYTIME = 150;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);
    h.setGameConfig(GAME_ID, "lastHostTime", 500*MINUTES_PER_DAY + DAYTIME + 50 /* delay */);
    h.setSchedule(GAME_ID, 3, "type", 2 /* weekly */);
    h.setSchedule(GAME_ID, 3, "interval", 4 /* days */);
    h.setSchedule(GAME_ID, 3, "daytime", DAYTIME);
    h.setSchedule(GAME_ID, 3, "hostLimit", 45 /* permitted delay */);
    h.addSchedule(GAME_ID, 3);

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(200, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::HostAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, (500 + 5 /* one more than configured */)*MINUTES_PER_DAY + DAYTIME);
}

/** Test weekly schedule, delayed host, edge case.
    Host is delayed exactly the permitted amount. */
void
TestServerHostCronImpl::testRunningWeeklyDelayedEdge()
{
    const int GAME_ID = 9876;
    const int DAYTIME = 150;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);
    h.setGameConfig(GAME_ID, "lastHostTime", 500*MINUTES_PER_DAY + DAYTIME + 45 /* delay */);
    h.setSchedule(GAME_ID, 4, "type", 2 /* weekly */);
    h.setSchedule(GAME_ID, 4, "interval", 4 /* days */);
    h.setSchedule(GAME_ID, 4, "daytime", DAYTIME);
    h.setSchedule(GAME_ID, 4, "hostLimit", 45 /* permitted delay */);
    h.addSchedule(GAME_ID, 4);

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(200, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::HostAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, (500 + 4 /* exact interval */)*MINUTES_PER_DAY + DAYTIME);
}

/** Test daily schedule, normal case.
    Last host on Monday, must run on Wednesday. */
void
TestServerHostCronImpl::testRunningDailyNormal()
{
    const int GAME_ID = 77;
    const int DAYTIME = 360;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);
    h.setGameConfig(GAME_ID, "lastHostTime", (700 /* 100 weeks */ - 4 /* day 0 is thursday */ + 1 /* monday */)*MINUTES_PER_DAY + DAYTIME);
    h.setSchedule(GAME_ID, 8, "type", 1 /* daily */);
    h.setSchedule(GAME_ID, 8, "weekdays", 2+8+32 /* Mo,We,Fr */);
    h.setSchedule(GAME_ID, 8, "daytime", DAYTIME);
    h.setSchedule(GAME_ID, 8, "hostLimit", 45 /* permitted delay */);
    h.addSchedule(GAME_ID, 8);

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(200, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::HostAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, (700-4+3 /* Wednesday */)*MINUTES_PER_DAY + DAYTIME);
}

/** Test daily schedule, normal case.
    Last host on Friday, must run on next Monday. */
void
TestServerHostCronImpl::testRunningDailyNormal2()
{
    const int GAME_ID = 77;
    const int DAYTIME = 360;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);
    h.setGameConfig(GAME_ID, "lastHostTime", (700 /* 100 weeks */ - 4 /* day 0 is thursday */ + 5 /* Friday */)*MINUTES_PER_DAY + DAYTIME);
    h.setSchedule(GAME_ID, 8, "type", 1 /* daily */);
    h.setSchedule(GAME_ID, 8, "weekdays", 2+8+32 /* Mo,We,Fr */);
    h.setSchedule(GAME_ID, 8, "daytime", DAYTIME);
    h.setSchedule(GAME_ID, 8, "hostLimit", 45 /* permitted delay */);
    h.addSchedule(GAME_ID, 8);

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(200, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::HostAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, (700-4+8 /* Next Monday */)*MINUTES_PER_DAY + DAYTIME);
}

/** Test daily schedule, host runs between days.
    Last host on Tuesday, schedule host for Friday. */
void
TestServerHostCronImpl::testRunningDailyMid()
{
    const int GAME_ID = 77;
    const int DAYTIME = 360;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);
    h.setGameConfig(GAME_ID, "lastHostTime", (700 /* 100 weeks */ - 4 /* day 0 is thursday */ + 2 /* Tuesday */)*MINUTES_PER_DAY + DAYTIME);
    h.setSchedule(GAME_ID, 8, "type", 1 /* daily */);
    h.setSchedule(GAME_ID, 8, "weekdays", 2+8+32 /* Mo,We,Fr */);
    h.setSchedule(GAME_ID, 8, "daytime", DAYTIME);
    h.setSchedule(GAME_ID, 8, "hostLimit", 45 /* permitted delay */);
    h.addSchedule(GAME_ID, 8);

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(200, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::HostAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, (700-4+5 /* Friday */)*MINUTES_PER_DAY + DAYTIME);
}

/** Test daily schedule, turns all in with early hosting enabled.
    Host must run after last turn. */
void
TestServerHostCronImpl::testRunningDailyEarly()
{
    const int GAME_ID = 77;
    const int DAYTIME = 360;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);
    h.setGameConfig(GAME_ID, "lastHostTime", (700 /* 100 weeks */ - 4 /* day 0 is thursday */ + 2 /* Tuesday */)*MINUTES_PER_DAY + DAYTIME);
    h.setSchedule(GAME_ID, 8, "type", 1 /* daily */);
    h.setSchedule(GAME_ID, 8, "weekdays", 2+8+32 /* Mo,We,Fr */);
    h.setSchedule(GAME_ID, 8, "daytime", DAYTIME);
    h.setSchedule(GAME_ID, 8, "hostLimit", 45 /* permitted delay */);
    h.setSchedule(GAME_ID, 8, "hostDelay", 22 /* after last turn */);
    h.setSchedule(GAME_ID, 8, "hostEarly", 1);
    h.addSchedule(GAME_ID, 8);
    for (int s = 1; s <= 11; ++s) {
        h.addPlayer(GAME_ID, s, 600+s);
        h.setTurnState(GAME_ID, s, Game::TurnGreen);
    }
    h.setGameConfig(GAME_ID, "lastTurnSubmitted", (700 /* 100 weeks */ - 4 /* day 0 is thursday */ + 2 /* Tuesday */)*MINUTES_PER_DAY + DAYTIME + 100);

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(200, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::HostAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, (700-4+2 /* still Tuesday */)*MINUTES_PER_DAY + DAYTIME + 122);
}

/** Test Manual schedule.
    If trigger is missing, host does not run. */
void
TestServerHostCronImpl::testRunningManual()
{
    const int GAME_ID = 13579;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);
    h.setGameConfig(GAME_ID, "lastHostTime", 4000);
    h.setGameConfig(GAME_ID, "lastTurnSubmitted", 4100);
    h.setSchedule(GAME_ID, 57, "type", 4);
    h.setSchedule(GAME_ID, 57, "hostDelay", 50);
    h.setSchedule(GAME_ID, 57, "hostEarly", 1);
    h.addSchedule(GAME_ID, 57);

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(4110, h.root(), GAME_ID, sch);
    TS_ASSERT(sch.empty());
}

/** Test Manual schedule with trigger.
    Host must run immediately. */
void
TestServerHostCronImpl::testRunningManualTrigger()
{
    const int GAME_ID = 800;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);
    h.setGameConfig(GAME_ID, "lastHostTime", 4000);
    h.setGameConfig(GAME_ID, "lastTurnSubmitted", 4100);
    h.setGameConfig(GAME_ID, "hostRunNow", 1);
    h.setSchedule(GAME_ID, 5, "type", 4);
    h.setSchedule(GAME_ID, 5, "hostDelay", 50);
    h.addSchedule(GAME_ID, 5);

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(4200, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::HostAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, 4200 /* same as time of query */);
}

/** Test Manual schedule with all turns in.
    Host must run after turn submission. */
void
TestServerHostCronImpl::testRunningManualEarly()
{
    const int GAME_ID = 666;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);
    h.setGameConfig(GAME_ID, "lastHostTime", 4000);
    h.setGameConfig(GAME_ID, "lastTurnSubmitted", 4100);
    h.setSchedule(GAME_ID, 5, "type", 4);
    h.setSchedule(GAME_ID, 5, "hostDelay", 50);
    h.setSchedule(GAME_ID, 5, "hostEarly", 1);
    h.addSchedule(GAME_ID, 5);
    for (int s = 1; s <= 11; ++s) {
        h.setTurnState(GAME_ID, s, (s & 1) ? Game::TurnGreen : Game::TurnYellow /* why not? */);
        h.addPlayer(GAME_ID, s, 600+s);
    }

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(4110, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::HostAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, 4150 /* lastTurnSubmitted + hostDelay */);
}

/** Test Manual schedule with not all turns in.
    Host must not run. */
void
TestServerHostCronImpl::testRunningManualEarlyMiss()
{
    const int GAME_ID = 13579;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);
    h.setGameConfig(GAME_ID, "lastHostTime", 4000);
    h.setGameConfig(GAME_ID, "lastTurnSubmitted", 4100);
    h.setSchedule(GAME_ID, 5, "type", 4);
    h.setSchedule(GAME_ID, 5, "hostDelay", 50);
    h.setSchedule(GAME_ID, 5, "hostEarly", 1);
    h.addSchedule(GAME_ID, 5);
    for (int s = 1; s <= 11; ++s) {
        h.addPlayer(GAME_ID, s, 600+s);
    }
    for (int s = 1; s <= 8 /* Not 11! */; ++s) {
        h.setTurnState(GAME_ID, s, Game::TurnGreen);
    }

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(4110, h.root(), GAME_ID, sch);
    TS_ASSERT(sch.empty());
}

/** Test Quick schedule, all turns in.
    Host must run. */
void
TestServerHostCronImpl::testRunningQuick()
{
    const int GAME_ID = 32168;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);
    h.setGameConfig(GAME_ID, "lastHostTime", 4000);
    h.setGameConfig(GAME_ID, "lastTurnSubmitted", 4100);
    h.setSchedule(GAME_ID, 77, "type", 3);
    h.setSchedule(GAME_ID, 77, "hostDelay", 50);
    h.setSchedule(GAME_ID, 77, "hostEarly", 1);
    h.addSchedule(GAME_ID, 77);
    for (int s = 1; s <= 11; ++s) {
        h.setTurnState(GAME_ID, s, Game::TurnGreen);
        h.addPlayer(GAME_ID, s, 600+s);
    }

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(4110, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::HostAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, 4150 /* lastTurnSubmitted + hostDelay */);
}

/** Test Quick schedule, not all turns in.
    Host must not run. */
void
TestServerHostCronImpl::testRunningQuickMiss()
{
    const int GAME_ID = 25392;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);
    h.setGameConfig(GAME_ID, "lastHostTime", 4000);
    h.setGameConfig(GAME_ID, "lastTurnSubmitted", 4100);
    h.setSchedule(GAME_ID, 88, "type", 3);
    h.setSchedule(GAME_ID, 88, "hostDelay", 50);
    h.setSchedule(GAME_ID, 88, "hostEarly", 1);
    h.addSchedule(GAME_ID, 88);

    // For a change, we're submitting 11 turn files but mark half of them temporary
    for (int s = 1; s <= 11; ++s) {
        h.setTurnState(GAME_ID, s, (s & 1) ? Game::TurnGreen : Game::TurnGreen+Game::TurnIsTemporary /* why not? */);
        h.addPlayer(GAME_ID, s, 600+s);
    }

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(4110, h.root(), GAME_ID, sch);
    TS_ASSERT(sch.empty());
}

/** Test Quick schedule, all turns in (but not all slots populated).
    Host must run. */
void
TestServerHostCronImpl::testRunningQuickPartial()
{
    const int GAME_ID = 6722;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);
    h.setGameConfig(GAME_ID, "lastHostTime", 600);
    h.setGameConfig(GAME_ID, "lastTurnSubmitted", 620);
    h.setSchedule(GAME_ID, 6, "type", 3);
    h.setSchedule(GAME_ID, 6, "hostDelay", 50);
    h.setSchedule(GAME_ID, 6, "hostEarly", 1);
    h.addSchedule(GAME_ID, 6);
    for (int s = 1; s <= 8 /* not 11! */; ++s) {
        h.setTurnState(GAME_ID, s, Game::TurnGreen);
        h.addPlayer(GAME_ID, s, 600+s);
    }

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(630, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::HostAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, 670 /* lastTurnSubmitted + hostDelay */);
}

/** Test expiring Weekly schedule.
    We're falling from a once-in-7-days schedule to a once-in-2-days schedule.
    Must show ones-in-2-days. */
void
TestServerHostCronImpl::testRunningExpireWeekly()
{
    const int GAME_ID = 99;
    const int DAYTIME = 200;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);
    h.setGameConfig(GAME_ID, "lastHostTime", (100*MINUTES_PER_DAY) + DAYTIME);

    h.setSchedule(GAME_ID, 4, "type", 2 /* weekly */);
    h.setSchedule(GAME_ID, 4, "interval", 2 /* days */);
    h.setSchedule(GAME_ID, 4, "daytime", DAYTIME);
    h.addSchedule(GAME_ID, 4);

    h.setSchedule(GAME_ID, 3, "type", 2 /* weekly */);
    h.setSchedule(GAME_ID, 3, "interval", 7 /* days */);
    h.setSchedule(GAME_ID, 3, "daytime", DAYTIME);
    h.setSchedule(GAME_ID, 3, "condition", 1);
    h.setSchedule(GAME_ID, 3, "condTurn", 2);
    h.addSchedule(GAME_ID, 3);

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(200, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::HostAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, (102*MINUTES_PER_DAY) + DAYTIME);
}

/** Test expiring Weekly schedule.
    We're falling from a once-in-2-days schedule to a once-in-7-days schedule.
    Must show ones-in-7-days. */
void
TestServerHostCronImpl::testRunningExpireWeekly2()
{
    const int GAME_ID = 99;
    const int DAYTIME = 200;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);
    h.setGameConfig(GAME_ID, "lastHostTime", (100*MINUTES_PER_DAY) + DAYTIME);

    h.setSchedule(GAME_ID, 4, "type", 2 /* weekly */);
    h.setSchedule(GAME_ID, 4, "interval", 7 /* days */);
    h.setSchedule(GAME_ID, 4, "daytime", DAYTIME);
    h.addSchedule(GAME_ID, 4);

    h.setSchedule(GAME_ID, 3, "type", 2 /* weekly */);
    h.setSchedule(GAME_ID, 3, "interval", 2 /* days */);
    h.setSchedule(GAME_ID, 3, "daytime", DAYTIME);
    h.setSchedule(GAME_ID, 3, "condition", 1);
    h.setSchedule(GAME_ID, 3, "condTurn", 2);
    h.addSchedule(GAME_ID, 3);

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(200, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::HostAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, (107*MINUTES_PER_DAY) + DAYTIME);
}

/** Test expiring Daily schedule.
    Changing from a Su/Th schedule to a Mo/We/Fr schedule. */
void
TestServerHostCronImpl::testRunningExpireDaily()
{
    const int GAME_ID = 77;
    const int DAYTIME = 360;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);
    h.setGameConfig(GAME_ID, "lastHostTime", (700 /* 100 weeks */ - 4 /* day 0 is thursday */ + 0 /* Sunday */)*MINUTES_PER_DAY + DAYTIME);

    h.setSchedule(GAME_ID, 8, "type", 1 /* daily */);
    h.setSchedule(GAME_ID, 8, "weekdays", 2+8+32 /* Mo,We,Fr */);
    h.setSchedule(GAME_ID, 8, "daytime", DAYTIME);
    h.setSchedule(GAME_ID, 8, "hostLimit", 45 /* permitted delay */);
    h.addSchedule(GAME_ID, 8);

    h.setSchedule(GAME_ID, 3, "type", 1 /* daily */);
    h.setSchedule(GAME_ID, 3, "weekdays", 1+16 /* Su,Th */);
    h.setSchedule(GAME_ID, 3, "daytime", DAYTIME);
    h.setSchedule(GAME_ID, 3, "hostLimit", 45 /* permitted delay */);
    h.setSchedule(GAME_ID, 3, "condition", 1);
    h.setSchedule(GAME_ID, 3, "condTurn", 2);
    h.addSchedule(GAME_ID, 3);

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(200, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::HostAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, (700-4+3 /* Wednesday */)*MINUTES_PER_DAY + DAYTIME);
}

/** Test expiring Daily schedule.
    Changing from a Mo/We/Fr schedule to a Su/Th schedule. */
void
TestServerHostCronImpl::testRunningExpireDaily2()
{
    const int GAME_ID = 77;
    const int DAYTIME = 360;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);
    h.setGameConfig(GAME_ID, "lastHostTime", (700 /* 100 weeks */ - 4 /* day 0 is thursday */ + 3 /* Wednesday */)*MINUTES_PER_DAY + DAYTIME);

    h.setSchedule(GAME_ID, 8, "type", 1 /* daily */);
    h.setSchedule(GAME_ID, 8, "weekdays", 1+16 /* Su,Th */);
    h.setSchedule(GAME_ID, 8, "daytime", DAYTIME);
    h.setSchedule(GAME_ID, 8, "hostLimit", 45 /* permitted delay */);
    h.addSchedule(GAME_ID, 8);

    h.setSchedule(GAME_ID, 3, "type", 1 /* daily */);
    h.setSchedule(GAME_ID, 3, "weekdays", 2+8+32 /* Mo,We,Fr */);
    h.setSchedule(GAME_ID, 3, "daytime", DAYTIME);
    h.setSchedule(GAME_ID, 3, "hostLimit", 45 /* permitted delay */);
    h.setSchedule(GAME_ID, 3, "condition", 1);
    h.setSchedule(GAME_ID, 3, "condTurn", 2);
    h.addSchedule(GAME_ID, 3);

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(200, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::HostAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, (700-4+7 /* Sunday */)*MINUTES_PER_DAY + DAYTIME);
}

/** Test expiration with a time condition. */
void
TestServerHostCronImpl::testRunningExpireDate()
{
    const int GAME_ID = 99;
    const int DAYTIME = 200;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);
    h.setGameConfig(GAME_ID, "lastHostTime", (100*MINUTES_PER_DAY) + DAYTIME);

    h.setSchedule(GAME_ID, 4, "type", 2 /* weekly */);
    h.setSchedule(GAME_ID, 4, "interval", 7 /* days */);
    h.setSchedule(GAME_ID, 4, "daytime", DAYTIME);
    h.addSchedule(GAME_ID, 4);

    h.setSchedule(GAME_ID, 3, "type", 2 /* weekly */);
    h.setSchedule(GAME_ID, 3, "interval", 2 /* days */);
    h.setSchedule(GAME_ID, 3, "daytime", DAYTIME);
    h.setSchedule(GAME_ID, 3, "condition", 2);
    h.setSchedule(GAME_ID, 3, "condTime", (101*MINUTES_PER_DAY) + DAYTIME);
    h.addSchedule(GAME_ID, 3);

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(200, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::ScheduleChangeAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, (101*MINUTES_PER_DAY) + DAYTIME);
}

/** Test expiration towards a fixed-weekday schedule.
    Host date must be set on one of the fixed weekdays instead of being run immmediately for being overdue. */
void
TestServerHostCronImpl::testRunningExpireUpdate()
{
    const int GAME_ID = 66;
    const int DAYTIME = 150;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);
    h.setGameConfig(GAME_ID, "lastHostTime", (700 /* 100 weeks */ - 4 /* day 0 is thursday */ + 0 /* Sunday */)*MINUTES_PER_DAY + DAYTIME);

    h.setSchedule(GAME_ID, 8, "type", 1 /* daily */);
    h.setSchedule(GAME_ID, 8, "weekdays", 2+8+32 /* Mo,We,Fr */);
    h.setSchedule(GAME_ID, 8, "daytime", DAYTIME);
    h.setSchedule(GAME_ID, 8, "hostLimit", 45 /* permitted delay */);
    h.addSchedule(GAME_ID, 8);

    h.setSchedule(GAME_ID, 3, "type", 3 /* asap */);
    h.setSchedule(GAME_ID, 3, "condition", 2 /* time */);
    h.setSchedule(GAME_ID, 3, "condTurn", (700 /* 100 weeks */ - 4 /* day 0 is thursday */ + 7 /* Next Sunday */)*MINUTES_PER_DAY + DAYTIME);
    h.addSchedule(GAME_ID, 3);

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes((700-4+7)*MINUTES_PER_DAY + DAYTIME, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::HostAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, (700-4+8 /* Next Monday */)*MINUTES_PER_DAY + DAYTIME);
}

/** Test schedule change fail protection.
    Even if the schedule says to run now, if that's due to it being recently changed, defer a little. */
void
TestServerHostCronImpl::testRunningChangeProtection()
{
    const int GAME_ID = 32168;

    // Set up
    TestHarness h;
    h.createGame(GAME_ID, HostGame::Running);
    h.setGameConfig(GAME_ID, "turn", 3);
    h.setGameConfig(GAME_ID, "lastHostTime", 4000);
    h.setGameConfig(GAME_ID, "lastTurnSubmitted", 4100);
    h.setGameConfig(GAME_ID, "lastScheduleChange", 4149);
    h.setSchedule(GAME_ID, 77, "type", 3);
    h.setSchedule(GAME_ID, 77, "hostDelay", 50);
    h.setSchedule(GAME_ID, 77, "hostEarly", 1);
    h.addSchedule(GAME_ID, 77);
    for (int s = 1; s <= 11; ++s) {
        h.setTurnState(GAME_ID, s, Game::TurnGreen);
        h.addPlayer(GAME_ID, s, 600+s);
    }

    // Verify
    std::list<HostCron::Event> sch;
    server::host::computeGameTimes(4149, h.root(), GAME_ID, sch);
    TS_ASSERT_EQUALS(sch.size(), 1U);
    TS_ASSERT_EQUALS(sch.front().action, HostCron::HostAction);
    TS_ASSERT_EQUALS(sch.front().gameId, GAME_ID);
    TS_ASSERT_EQUALS(sch.front().time, 4159 /* lastScheduleChange + SCHEDULE_CHANGE_GRACE_PERIOD(=10) */);
}

