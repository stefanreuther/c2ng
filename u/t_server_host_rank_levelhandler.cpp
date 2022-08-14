/**
  *  \file u/t_server_host_rank_levelhandler.cpp
  *  \brief Test for server::host::rank::LevelHandler
  */

#include "server/host/rank/levelhandler.hpp"

#include "t_server_host_rank.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "util/processrunner.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "server/host/game.hpp"
#include "afl/net/redis/stringfield.hpp"

using afl::net::redis::HashKey;
using afl::net::redis::IntegerSetKey;

namespace {
    class TestHarness {
     public:
        TestHarness()
            : m_db(), m_null(), m_mail(m_null), m_runner(), m_fs(), m_root(m_db, m_null, m_null, m_mail, m_runner, m_fs, server::host::Configuration())
            { }

        server::host::Root& root()
            { return m_root; }

        afl::net::CommandHandler& db()
            { return m_db; }

     private:
        afl::net::redis::InternalDatabase m_db;
        afl::net::NullCommandHandler m_null;
        server::interface::MailQueueClient m_mail;
        util::ProcessRunner m_runner;
        afl::io::NullFileSystem m_fs;
        server::host::Root m_root;
    };
}


/** Test submitting a turn. */
void
TestServerHostRankLevelHandler::testTurnSubmission()
{
    // Turn submission does not care about the replacement level, so just try multiple levels.
    TestHarness h;   // (outside the loop because it creates a ProcessRunner which is expensive)
    for (int i = 0; i < 100; ++i) {
        // Setup
        HashKey(h.db(), "user:1390:profile").intField("turnsplayed").set(9);
        HashKey(h.db(), "user:1390:profile").intField("turnsmissed").set(1);
        HashKey(h.db(), "user:1390:profile").intField("turnreliability").set(90000);

        // Testee
        server::host::rank::LevelHandler testee(h.root());

        // Submit a turn. Reliability should now be 0.97*90 + 0.03*100 = 90.3
        testee.handlePlayerTurn("1390", true, 0);

        // Verify
        TS_ASSERT_EQUALS(HashKey(h.db(), "user:1390:profile").intField("turnsplayed").get(), 10);
        TS_ASSERT_EQUALS(HashKey(h.db(), "user:1390:profile").intField("turnsmissed").get(), 1);
        TS_ASSERT_EQUALS(HashKey(h.db(), "user:1390:profile").intField("turnreliability").get(), 90300);
    }
}

/** Test missing a turn. */
void
TestServerHostRankLevelHandler::testTurnMiss()
{
    TestHarness h;

    // Setup
    HashKey(h.db(), "user:1390:profile").intField("turnsplayed").set(9);
    HashKey(h.db(), "user:1390:profile").intField("turnsmissed").set(1);
    HashKey(h.db(), "user:1390:profile").intField("turnreliability").set(90000);

    // Testee
    server::host::rank::LevelHandler testee(h.root());

    // Miss a turn as primary player. Reliability should now be 0.97*90 + 0 = 87.3
    testee.handlePlayerTurn("1390", false, 0);

    // Verify
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1390:profile").intField("turnsplayed").get(), 9);
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1390:profile").intField("turnsmissed").get(), 2);
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1390:profile").intField("turnreliability").get(), 87300);

    // Miss a turn as replacement player. Reliability should now be 0.97*87.3 + 0.03*50 = 86.181
    testee.handlePlayerTurn("1390", false, 1);
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1390:profile").intField("turnsplayed").get(), 9);
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1390:profile").intField("turnsmissed").get(), 3);
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1390:profile").intField("turnreliability").get(), 86181);

    // Miss a turn as replacement's replacement. Reliability should now be 0.97*86.181 + 0.03*75 = 88.84557
    // Note that we're truncating the reliability points!
    testee.handlePlayerTurn("1390", false, 2);
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1390:profile").intField("turnsplayed").get(), 9);
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1390:profile").intField("turnsmissed").get(), 4);
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1390:profile").intField("turnreliability").get(), 85845);
}

/** Test dropping in turn 0. */
void
TestServerHostRankLevelHandler::testDropTurn0()
{
    // Setup: just define a player and declare the game existing, but empty.
    // This models a freshly-mastered game.
    TestHarness h;
    HashKey(h.db(), "user:1776:profile").intField("turnsplayed").set(9);
    HashKey(h.db(), "user:1776:profile").intField("turnsmissed").set(1);
    HashKey(h.db(), "user:1776:profile").intField("turnreliability").set(90000);
    IntegerSetKey(h.db(), "game:all").add(7);

    // Testee
    server::host::rank::LevelHandler testee(h.root());
    server::host::Game g(h.root(), 7);
    testee.handlePlayerDrop("1776", g, 3);

    // Verify: no change
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1776:profile").intField("turnreliability").get(), 90000);
}

/** Test dropping without score. */
void
TestServerHostRankLevelHandler::testDropScoreless()
{
    // Setup: just define a player and a game with a nonzero turn.
    // This models a freshly-imported game (or a game with broken scoring).
    TestHarness h;
    HashKey(h.db(), "user:1776:profile").intField("turnsplayed").set(9);
    HashKey(h.db(), "user:1776:profile").intField("turnsmissed").set(1);
    HashKey(h.db(), "user:1776:profile").intField("turnreliability").set(90000);
    IntegerSetKey(h.db(), "game:all").add(7);
    HashKey(h.db(), "game:7:settings").intField("turn").set(5);

    // Testee
    server::host::rank::LevelHandler testee(h.root());
    server::host::Game g(h.root(), 7);
    testee.handlePlayerDrop("1776", g, 3);

    // Verify: no change
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1776:profile").intField("turnreliability").get(), 90000);
}

/** Test dropping with score zero. */
void
TestServerHostRankLevelHandler::testDropZeroScore()
{
    // Setup: a game with turn and score, but all scores are 0
    TestHarness h;
    HashKey(h.db(), "user:1776:profile").intField("turnsplayed").set(9);
    HashKey(h.db(), "user:1776:profile").intField("turnsmissed").set(1);
    HashKey(h.db(), "user:1776:profile").intField("turnreliability").set(90000);
    IntegerSetKey(h.db(), "game:all").add(7);
    HashKey(h.db(), "game:7:settings").intField("turn").set(5);
    HashKey(h.db(), "game:7:settings").stringField("endScoreName").set("w");
    HashKey(h.db(), "game:7:turn:5:scores").stringField("w").set(String_t(44, '\0'));

    // Testee
    server::host::rank::LevelHandler testee(h.root());
    server::host::Game g(h.root(), 7);
    testee.handlePlayerDrop("1776", g, 3);

    // Verify: no change
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1776:profile").intField("turnreliability").get(), 90000);
}

/** Test dropping with medium score. */
void
TestServerHostRankLevelHandler::testDropMidScore()
{
    // Setup: a game with turn and score, dropping player has a score but not the highest
    TestHarness h;
    HashKey(h.db(), "user:1984:profile").intField("turnsplayed").set(9);
    HashKey(h.db(), "user:1984:profile").intField("turnsmissed").set(1);
    HashKey(h.db(), "user:1984:profile").intField("turnreliability").set(90000);
    IntegerSetKey(h.db(), "game:all").add(7);
    HashKey(h.db(), "game:7:settings").intField("turn").set(5);
    HashKey(h.db(), "game:7:settings").stringField("endScoreName").set("w");
    HashKey(h.db(), "game:7:turn:5:scores").stringField("w").set(
        String_t("\0\0\0\0"     // 1
                 "\100\0\0\0"   // 2: score 64
                 "\200\0\0\0"   // 3: score 128
                 "\0\0\0\0"     // 4
                 "\200\0\0\0"   // 5: score 128
                 "\0\0\0\0"     // 6
                 "\300\0\0\0"   // 7: score 192, highest
                 "\0\0\0\0"     // 8
                 "\0\0\0\0"     // 9
                 "\0\0\0\0"     // 10
                 "\0\0\0\0", 44));

    // Testee
    server::host::rank::LevelHandler testee(h.root());
    server::host::Game g(h.root(), 7);
    testee.handlePlayerDrop("1984", g, 3);

    // Verify: new score is 90 * (1 - (0.66 * 128/192)) = 50.4
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1984:profile").intField("turnreliability").get(), 50400);
}

/** Test dropping with high score. */
void
TestServerHostRankLevelHandler::testDropHighScore()
{
    // Setup: a game with turn and score, dropping player has highest score
    TestHarness h;
    HashKey(h.db(), "user:1984:profile").intField("turnsplayed").set(9);
    HashKey(h.db(), "user:1984:profile").intField("turnsmissed").set(1);
    HashKey(h.db(), "user:1984:profile").intField("turnreliability").set(90000);
    IntegerSetKey(h.db(), "game:all").add(7);
    HashKey(h.db(), "game:7:settings").intField("turn").set(5);
    HashKey(h.db(), "game:7:settings").stringField("endScoreName").set("w");
    HashKey(h.db(), "game:7:turn:5:scores").stringField("w").set(
        String_t("\0\0\0\0"     // 1
                 "\100\0\0\0"   // 2: score 64
                 "\377\0\0\0"   // 3: score 255, highest
                 "\0\0\0\0"     // 4
                 "\200\0\0\0"   // 5: score 128
                 "\0\0\0\0"     // 6
                 "\300\0\0\0"   // 7: score 192
                 "\0\0\0\0"     // 8
                 "\0\0\0\0"     // 9
                 "\0\0\0\0"     // 10
                 "\0\0\0\0", 44));

    // Testee
    server::host::rank::LevelHandler testee(h.root());
    server::host::Game g(h.root(), 7);
    testee.handlePlayerDrop("1984", g, 3);

    // Verify: new score is 90 * (1 - (0.66 * 255/255)) = 30.6
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1984:profile").intField("turnreliability").get(), 30600);
}

/** Test addPlayerRankPoints(). */
void
TestServerHostRankLevelHandler::testRankPoints()
{
    // Setup: empty database, corresponds to freshly-made player
    TestHarness h;

    // Testee
    server::host::rank::LevelHandler testee(h.root());

    // Test
    testee.addPlayerRankPoints("1206", 30);
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1206:profile").intField("rankpoints").get(), 30);

    testee.addPlayerRankPoints("1206", -5);
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1206:profile").intField("rankpoints").get(), 25);
}

/** Test promotion. */
void
TestServerHostRankLevelHandler::testPromote()
{
    // Setup
    TestHarness h;

    // - 1390: 55% reliability, 40 turns, enough for rank #2 "Spaceman"
    HashKey(h.db(), "user:1390:profile").intField("turnsplayed").set(40);
    HashKey(h.db(), "user:1390:profile").intField("turnreliability").set(55000);
    HashKey(h.db(), "user:1390:profile").intField("rankpoints").set(250);

    // - 1394: 54% reliability is too little for Spaceman, stays at 0
    HashKey(h.db(), "user:1394:profile").intField("turnsplayed").set(40);
    HashKey(h.db(), "user:1394:profile").intField("turnreliability").set(54000);
    HashKey(h.db(), "user:1394:profile").intField("rankpoints").set(250);

    // - 1397: all the way up to vice admiral
    HashKey(h.db(), "user:1397:profile").intField("turnsplayed").set(10000);
    HashKey(h.db(), "user:1397:profile").intField("turnreliability").set(96000);
    HashKey(h.db(), "user:1397:profile").intField("rankpoints").set(49000);

    // Testee
    server::host::rank::LevelHandler testee(h.root());
    testee.handlePlayerRankChanges("1390");
    testee.handlePlayerRankChanges("1394");
    testee.handlePlayerRankChanges("1397");

    // Verify
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1390:profile").intField("rank").get(), 2);
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1394:profile").intField("rank").get(), 0);
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1397:profile").intField("rank").get(), 13);
}

/** Test demotion. */
void
TestServerHostRankLevelHandler::testDemote()
{
    // Setup
    TestHarness h;

    // - 1390: 55% reliability, 40 turns, enough for rank #2 "Spaceman", but not #3 "Petty Officer"
    HashKey(h.db(), "user:1390:profile").intField("turnsplayed").set(40);
    HashKey(h.db(), "user:1390:profile").intField("turnreliability").set(55000);
    HashKey(h.db(), "user:1390:profile").intField("rankpoints").set(250);
    HashKey(h.db(), "user:1390:profile").intField("rank").set(3);

    // - 1394: 54% reliability is enough to keep Spaceman
    HashKey(h.db(), "user:1394:profile").intField("turnsplayed").set(40);
    HashKey(h.db(), "user:1394:profile").intField("turnreliability").set(54000);
    HashKey(h.db(), "user:1394:profile").intField("rankpoints").set(250);
    HashKey(h.db(), "user:1394:profile").intField("rank").set(2);

    // - 1397: reliability loss from Vice Admiral; Captain #11 only needs 85%.
    HashKey(h.db(), "user:1397:profile").intField("turnsplayed").set(10000);
    HashKey(h.db(), "user:1397:profile").intField("turnreliability").set(89499);
    HashKey(h.db(), "user:1397:profile").intField("rankpoints").set(49000);
    HashKey(h.db(), "user:1397:profile").intField("rank").set(13);

    // Testee
    server::host::rank::LevelHandler testee(h.root());
    testee.handlePlayerRankChanges("1390");
    testee.handlePlayerRankChanges("1394");
    testee.handlePlayerRankChanges("1397");

    // Verify
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1390:profile").intField("rank").get(), 2);
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1394:profile").intField("rank").get(), 2);
    TS_ASSERT_EQUALS(HashKey(h.db(), "user:1397:profile").intField("rank").get(), 11);
}

