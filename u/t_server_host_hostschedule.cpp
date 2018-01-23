/**
  *  \file u/t_server_host_hostschedule.cpp
  *  \brief Test for server::host::HostSchedule
  */

#include "server/host/hostschedule.hpp"

#include "t_server_host.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/string/format.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/host/hostgame.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "util/processrunner.hpp"

using server::interface::HostGame;
using server::interface::HostSchedule;
using afl::string::Format;
using afl::net::redis::StringKey;
using afl::net::redis::StringSetKey;

namespace {
    class TestHarness {
     public:
        TestHarness()
            : m_db(), m_hostFile(), m_userFile(), m_null(), m_mail(m_null), m_runner(), m_fs(),
              m_root(m_db, m_hostFile, m_userFile, m_mail, m_runner, m_fs, server::host::Configuration())
            { }

        server::host::Root& root()
            { return m_root; }

        afl::net::CommandHandler& db()
            { return m_db; }

        int32_t createNewGame(HostGame::Type type, HostGame::State state);

     private:
        afl::net::redis::InternalDatabase m_db;
        server::file::InternalFileServer m_hostFile;
        server::file::InternalFileServer m_userFile;
        afl::net::NullCommandHandler m_null;
        server::interface::MailQueueClient m_mail;
        util::ProcessRunner m_runner;
        afl::io::NullFileSystem m_fs;
        server::host::Root m_root;
    };
}

int32_t
TestHarness::createNewGame(HostGame::Type type, HostGame::State state)
{
    server::host::Session session;
    server::host::HostGame hg(session, root());
    int32_t gid = hg.createNewGame();
    hg.setType(gid, type);
    hg.setState(gid, state);
    return gid;
}


/** Test adding and querying schedules. */
void
TestServerHostHostSchedule::testAddQuery()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostSchedule testee(session, h.root());

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Preparing);

    // Replace-to-create:
    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Daily;
        sch.interval = 3;
        testee.replace(gid, sch);
    }

    // Add
    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Weekly;
        sch.weekdays = 1;
        testee.add(gid, sch);
    }

    // Modify
    {
        HostSchedule::Schedule sch;
        sch.condition = HostSchedule::Turn;
        sch.conditionTurn = 10;
        testee.modify(gid, sch);
    }

    // Verify result
    {
        std::vector<HostSchedule::Schedule> result;
        testee.getAll(gid, result);

        TS_ASSERT_EQUALS(result.size(), 2U);

        // Added/modified schedule is first
        TS_ASSERT(result[0].type.isSame(HostSchedule::Weekly));
        TS_ASSERT(result[0].weekdays.isSame(1));
        TS_ASSERT(result[0].condition.isSame(HostSchedule::Turn));
        TS_ASSERT(result[0].conditionTurn.isSame(10));
        TS_ASSERT(result[0].hostEarly.isSame(true));                // default
        TS_ASSERT(result[0].hostDelay.isSame(30));                  // default

        // Original schedule is second
        TS_ASSERT(result[1].type.isSame(HostSchedule::Daily));
        TS_ASSERT(result[1].interval.isSame(3));
        TS_ASSERT(result[1].condition.isSame(HostSchedule::None));  // default
        TS_ASSERT(result[1].hostEarly.isSame(true));                // default
        TS_ASSERT(result[1].hostDelay.isSame(30));                  // default

        // Same daytime
        TS_ASSERT(result[0].daytime.isValid());
        TS_ASSERT(result[0].daytime.isSame(result[1].daytime));
    }
}

/** Test adding schedules with all properties. */
void
TestServerHostHostSchedule::testAddAll()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostSchedule testee(session, h.root());

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Preparing);

    // Add
    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Daily;
        sch.interval = 3;
        sch.hostEarly = false;
        sch.hostDelay = 15;
        sch.daytime = 400;
        sch.hostLimit = 50;
        testee.add(gid, sch);
    }

    // Verify result
    {
        std::vector<HostSchedule::Schedule> result;
        testee.getAll(gid, result);

        TS_ASSERT_EQUALS(result.size(), 1U);
        TS_ASSERT(result[0].type.isSame(HostSchedule::Daily));
        TS_ASSERT(result[0].interval.isSame(3));
        TS_ASSERT(result[0].hostEarly.isSame(false));
        TS_ASSERT(result[0].hostDelay.isSame(15));
        TS_ASSERT(result[0].daytime.isSame(400));
        TS_ASSERT(result[0].hostLimit.isSame(50));
    }
}

/** Test initial schedule state.
    A newly-created game must report an empty schedule. */
void
TestServerHostHostSchedule::testInit()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostSchedule testee(session, h.root());

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Preparing);

    // Verify result
    {
        std::vector<HostSchedule::Schedule> result;
        testee.getAll(gid, result);

        TS_ASSERT_EQUALS(result.size(), 0U);
    }
}

/** Test automatic daytime assignment.
    Setting the same (initial) schedule to multiple games must produce different daytimes. */
void
TestServerHostHostSchedule::testDaytime()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostSchedule testee(session, h.root());

    // Create a game
    int32_t gid1 = h.createNewGame(HostGame::PublicGame, HostGame::Preparing);
    int32_t gid2 = h.createNewGame(HostGame::PublicGame, HostGame::Preparing);
    int32_t gid3 = h.createNewGame(HostGame::PublicGame, HostGame::Preparing);

    // Set the same schedule to all
    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Daily;
        sch.interval = 3;
        testee.replace(gid1, sch);
        testee.replace(gid2, sch);
        testee.replace(gid3, sch);
    }

    // Verify all 3 schedules
    {
        std::vector<HostSchedule::Schedule> result1, result2, result3;
        testee.getAll(gid1, result1);
        testee.getAll(gid2, result2);
        testee.getAll(gid3, result3);

        TS_ASSERT_EQUALS(result1.size(), 1U);
        TS_ASSERT_EQUALS(result2.size(), 1U);
        TS_ASSERT_EQUALS(result3.size(), 1U);

        TS_ASSERT(result1[0].daytime.isValid());
        TS_ASSERT(result2[0].daytime.isValid());
        TS_ASSERT(result3[0].daytime.isValid());

        TS_ASSERT(!result1[0].daytime.isSame(result2[0].daytime));
        TS_ASSERT(!result1[0].daytime.isSame(result3[0].daytime));
        TS_ASSERT(!result2[0].daytime.isSame(result3[0].daytime));
    }
}

/** Test drop().
    Just a simple functionality test. */
void
TestServerHostHostSchedule::testDrop()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostSchedule testee(session, h.root());

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Preparing);

    // Add
    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Daily;
        sch.interval = 3;
        testee.replace(gid, sch);
    }

    // Add
    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Weekly;
        sch.weekdays = 1;
        testee.add(gid, sch);
    }

    // Remove
    testee.drop(gid);

    // Verify result
    {
        std::vector<HostSchedule::Schedule> result;
        testee.getAll(gid, result);

        TS_ASSERT_EQUALS(result.size(), 1U);

        // Original schedule remains
        TS_ASSERT(result[0].type.isSame(HostSchedule::Daily));
    }

    // Remove another
    testee.drop(gid);

    // Verify
    {
        std::vector<HostSchedule::Schedule> result;
        testee.getAll(gid, result);

        TS_ASSERT_EQUALS(result.size(), 0U);
    }

    // Remove another: this is harmless / no-op
    TS_ASSERT_THROWS_NOTHING(testee.drop(gid));
    TS_ASSERT_THROWS_NOTHING(testee.drop(gid));
}


/** Test preview().
    Just a simple functionality test. */
void
TestServerHostHostSchedule::testPreview()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostSchedule testee(session, h.root());

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Preparing);

    // Add
    {
        HostSchedule::Schedule sch;
        sch.type = HostSchedule::Daily;
        sch.interval = 3;
        sch.condition = HostSchedule::Turn;
        sch.conditionTurn = 10;
        testee.add(gid, sch);
    }

    // Preview "up to 100"
    {
        afl::data::IntegerList_t result;
        testee.preview(gid, afl::base::Nothing, 100, result);

        // 11 results: master + turn 1..10
        TS_ASSERT_EQUALS(result.size(), 11U);

        // Differences between turns must be 3 days
        for (int i = 1; i < 10; ++i) {
            TS_ASSERT_EQUALS(result[i] + 3*60*24, result[i+1]);
        }
    }

    // Preview "up to 5"
    {
        afl::data::IntegerList_t result;
        testee.preview(gid, afl::base::Nothing, 5, result);

        TS_ASSERT_EQUALS(result.size(), 5U);

        // Differences between turns must be 3 days
        for (int i = 1; i < 3; ++i) {
            TS_ASSERT_EQUALS(result[i] + 3*60*24, result[i+1]);
        }
    }

    // Preview "up to 7 days"
    {
        afl::data::IntegerList_t result;
        testee.preview(gid, 7*60*24, 100, result);

        // Must return master + 2 turns (+ 1 turn: it stops AFTER exceeding the limit).
        // It still needs a turn limit (same as -classic), although this might be debatable.
        TS_ASSERT_EQUALS(result.size(), 4U);
        TS_ASSERT_EQUALS(result[1] + 3*60*24, result[2]);
    }

    // Unlimited preview is not permitted
    {
        afl::data::IntegerList_t result;
        testee.preview(gid, afl::base::Nothing, afl::base::Nothing, result);
        TS_ASSERT_EQUALS(result.size(), 0U);
    }
}

