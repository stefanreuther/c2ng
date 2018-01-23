/**
  *  \file u/t_server_host_rank_victory.cpp
  *  \brief Test for server::host::rank::Victory
  */

#include "server/host/rank/victory.hpp"

#include "t_server_host_rank.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/integerlistkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/string/format.hpp"
#include "server/host/root.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "util/processrunner.hpp"

using afl::net::redis::Subtree;
using afl::net::redis::HashKey;
using afl::string::Format;

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

        void createPlayers();
        void createGame(int gameId);
        void setScore(int gameId);

        int getRankPoints(int userId);

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
    for (int p = 601; p <= 612; ++p) {
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
TestHarness::createGame(int gameId)
{
    // Create game
    Subtree g(db(), "game:");
    Subtree u(db(), "user:");
    g.subtree(gameId).stringKey("name").set("Test Game");
    g.subtree(gameId).stringKey("state").set("running");
    g.subtree(gameId).stringKey("type").set("public");

    // Join players
    for (int s = 1; s <= 11; ++s) {
        int p = 600 + s;
        g.subtree(gameId).subtree("player").subtree(s).intListKey("users").pushFront(p);
        g.subtree(gameId).subtree("player").subtree(s).hashKey("status").intField("slot").set(1);
        g.subtree(gameId).subtree("player").subtree(s).hashKey("status").intField("turn").set(1);
        g.subtree(gameId).hashKey("users").intField(Format("%d", p)).set(1);
        u.subtree(p).hashKey("games").intField(Format("%d", gameId)).set(1);
        g.subtree(gameId).stringKey("dir").set("/tmp/zzz");
    }

    // Game config
    g.subtree(gameId).hashKey("settings").intField("lastHostTime").set(999999999);
    g.subtree(gameId).hashKey("settings").stringField("host").set("phost-current");
    g.subtree(gameId).hashKey("settings").intField("turn").set(60);
    g.subtree(gameId).hashKey("cache").intField("difficulty").set(100);

    // Turn and score history
    for (int t = 1; t <= 60; ++t) {
        g.subtree(gameId).subtree("turn").subtree(t).hashKey("info").stringField("turnstatus").set(String_t("\1\0\1\0\1\0\1\0\1\0\1\0\1\0\1\0\1\0\1\0\1\0", 22));
        for (int s = 1; s <= 11; ++s) {
            g.subtree(gameId).subtree("turn").subtree(t).hashKey("player").intField(Format("%d", s)).set(s + 600);
        }
    }

    // Indexes
    g.intSetKey("all").add(gameId);
    g.intSetKey("state:running").add(gameId);
    g.intSetKey("pubstate:running").add(gameId);
}

void
TestHarness::setScore(int gameId)
{
    HashKey(db(), Format("game:%d:turn:60:scores", gameId)).stringField("score").set(String_t("\1\0\0\0\2\0\0\0\3\0\0\0\4\0\0\0\5\0\0\0\6\0\0\0\7\0\0\0\10\0\0\0\11\0\0\0\12\0\0\0\13\0\0\0", 44));
    HashKey(db(), Format("game:%d:settings", gameId)).stringField("endScoreName").set("score");
}

int
TestHarness::getRankPoints(int userId)
{
    return HashKey(db(), Format("user:%d:profile", userId)).intField("rankpoints").get();
}


/********************************* Tests *********************************/


/** Test basic ranking: default 60 turn game.
    No ranks declared: everyone gets first place
    -> everyone gets 2000 points
    Bug #345: it's only 2000*(59/60) = 1967 points

    This is the same as c2systest host/02_rank/basic. */
void
TestServerHostRankVictory::testRankingBasic()
{
    const int GAME_ID = 20000;

    // Create game
    TestHarness h;
    h.createPlayers();
    h.createGame(GAME_ID);

    // Rank it
    server::host::Game g(h.root(), GAME_ID);
    server::host::rank::checkForcedGameEnd(g);
    server::host::rank::computeGameRankings(h.root(), g);

    // Verify
    for (int s = 1; s <= 11; ++s) {
        TS_ASSERT_EQUALS(h.getRankPoints(s+600), 1967);
    }
}

/** Test basic ranking: short game.
    Default game, shortened to 40 turns
    No ranks declared: everyone gets first place
    -> everyone gets 1600 points (=2000 * 40/50 Turn_Factor)
    Bug #345: only 1560

    This is the same as c2systest host/02_rank/short. */
void
TestServerHostRankVictory::testRankingShort()
{
    const int GAME_ID = 7654;

    // Create game
    TestHarness h;
    h.createPlayers();
    h.createGame(GAME_ID);
    HashKey(h.db(), Format("game:%d:settings", GAME_ID)).intField("turn").set(40);

    // Rank it
    server::host::Game g(h.root(), GAME_ID);
    server::host::rank::checkForcedGameEnd(g);
    server::host::rank::computeGameRankings(h.root(), g);

    // Verify
    for (int s = 1; s <= 11; ++s) {
        TS_ASSERT_EQUALS(h.getRankPoints(s+600), 1560);
    }
}

/** Test basic ranking: scores.
    Default 60 turn game
    Players have scores
    -> point distribution according to table (2000, 1400, ..., 100)

    This is the same as c2systest host/02_rank/order */
void
TestServerHostRankVictory::testRankingOrder()
{
    const int GAME_ID = 3000;

    // Create game
    TestHarness h;
    h.createPlayers();
    h.createGame(GAME_ID);
    h.setScore(GAME_ID);

    // Rank it
    server::host::Game g(h.root(), GAME_ID);
    server::host::rank::checkForcedGameEnd(g);
    server::host::rank::computeGameRankings(h.root(), g);

    // Verify
    TS_ASSERT_EQUALS(h.getRankPoints(601), 98);
    TS_ASSERT_EQUALS(h.getRankPoints(602), 98);
    TS_ASSERT_EQUALS(h.getRankPoints(603), 98);
    TS_ASSERT_EQUALS(h.getRankPoints(604), 197);
    TS_ASSERT_EQUALS(h.getRankPoints(605), 295);
    TS_ASSERT_EQUALS(h.getRankPoints(606), 393);
    TS_ASSERT_EQUALS(h.getRankPoints(607), 589);
    TS_ASSERT_EQUALS(h.getRankPoints(608), 786);
    TS_ASSERT_EQUALS(h.getRankPoints(609), 982);
    TS_ASSERT_EQUALS(h.getRankPoints(610), 1375);
    TS_ASSERT_EQUALS(h.getRankPoints(611), 1964);
}

/** Test basic ranking: replacement.
    Default 60 turn game.
    Player 3 starts as 612, then replaced by 603
    Players have scores
    -> point distribution according to table. Everyone gets usual points, 603 and 612 share.

    This is the same as c2systest host/02_rank/repl */
void
TestServerHostRankVictory::testRankingReplacement()
{
    const int GAME_ID = 3000;

    // Create game
    TestHarness h;
    h.createPlayers();
    h.createGame(GAME_ID);
    h.setScore(GAME_ID);
    for (int t = 1; t <= 20; ++t) {
        HashKey(h.db(), Format("game:%d:turn:%d:player", GAME_ID, t)).stringField("3").set("612");
    }

    // Rank it
    server::host::Game g(h.root(), GAME_ID);
    server::host::rank::checkForcedGameEnd(g);
    server::host::rank::computeGameRankings(h.root(), g);

    // Verify
    TS_ASSERT_EQUALS(h.getRankPoints(601), 98);
    TS_ASSERT_EQUALS(h.getRankPoints(602), 98);
    TS_ASSERT_EQUALS(h.getRankPoints(603), 67);
    TS_ASSERT_EQUALS(h.getRankPoints(604), 197);
    TS_ASSERT_EQUALS(h.getRankPoints(605), 295);
    TS_ASSERT_EQUALS(h.getRankPoints(606), 393);
    TS_ASSERT_EQUALS(h.getRankPoints(607), 589);
    TS_ASSERT_EQUALS(h.getRankPoints(608), 786);
    TS_ASSERT_EQUALS(h.getRankPoints(609), 982);
    TS_ASSERT_EQUALS(h.getRankPoints(610), 1375);
    TS_ASSERT_EQUALS(h.getRankPoints(611), 1964);
    TS_ASSERT_EQUALS(h.getRankPoints(612), 32);
}

/** Test basic ranking: different original ranks.
    Default 60 turn game.
    Players have scores.
    Player 5 already has rank 10.
    -> point distribution according to table; ranks above get more points, 5 gets fewer points, below get regular points

    This is the same as c2systest host/02_rank/diff. */
void
TestServerHostRankVictory::testRankingDifferent()
{
    const int GAME_ID = 1701;

    // Create game
    TestHarness h;
    h.createPlayers();
    h.createGame(GAME_ID);
    h.setScore(GAME_ID);
    HashKey(h.db(), "user:605:profile").intField("rank").set(9);
    HashKey(h.db(), "user:605:profile").intField("rankpoints").set(6666);
    HashKey(h.db(), "user:605:profile").intField("turnreliability").set(90000);
    HashKey(h.db(), "user:605:profile").intField("turnsplayed").set(222);
    HashKey(h.db(), "user:605:profile").intField("turnsmissed").set(2);

    // Rank it
    server::host::Game g(h.root(), GAME_ID);
    server::host::rank::checkForcedGameEnd(g);
    server::host::rank::computeGameRankings(h.root(), g);

    // Verify
    TS_ASSERT_EQUALS(h.getRankPoints(601), 98);
    TS_ASSERT_EQUALS(h.getRankPoints(602), 98);
    TS_ASSERT_EQUALS(h.getRankPoints(603), 98);
    TS_ASSERT_EQUALS(h.getRankPoints(604), 197);
    TS_ASSERT_EQUALS(h.getRankPoints(605), 6890);  // +224, not +295
    TS_ASSERT_EQUALS(h.getRankPoints(606), 423);
    TS_ASSERT_EQUALS(h.getRankPoints(607), 635);
    TS_ASSERT_EQUALS(h.getRankPoints(608), 845);
    TS_ASSERT_EQUALS(h.getRankPoints(609), 1056);
    TS_ASSERT_EQUALS(h.getRankPoints(610), 1477);
    TS_ASSERT_EQUALS(h.getRankPoints(611), 2109);
}

/** Test basic ranking: late join.
    Default 60 turn game.
    Player 3 joins late (turn 21).
    Players have scores.
    -> point distribution according to table. High ranks get less points (it was easier when player 3 was not playing).

    This is the same as c2systest host/02_rank/late */
void
TestServerHostRankVictory::testRankingLate()
{
    const int GAME_ID = 32168;

    // Create game
    TestHarness h;
    h.createPlayers();
    h.createGame(GAME_ID);
    h.setScore(GAME_ID);
    for (int t = 1; t <= 20; ++t) {
        HashKey(h.db(), Format("game:%d:turn:%d:info", GAME_ID, t)).stringField("turnstatus").set(String_t("\1\0\1\0\xFF\xFF\1\0\1\0\1\0\1\0\1\0\1\0\1\0\1\0", 22));
        HashKey(h.db(), Format("game:%d:turn:%d:player", GAME_ID, t)).field("3").remove();
    }

    // Rank it
    server::host::Game g(h.root(), GAME_ID);
    server::host::rank::checkForcedGameEnd(g);
    server::host::rank::computeGameRankings(h.root(), g);

    // Verify
    TS_ASSERT_EQUALS(h.getRankPoints(601), 98);
    TS_ASSERT_EQUALS(h.getRankPoints(602), 98);
    TS_ASSERT_EQUALS(h.getRankPoints(603), 67);
    TS_ASSERT_EQUALS(h.getRankPoints(604), 196);
    TS_ASSERT_EQUALS(h.getRankPoints(605), 294);
    TS_ASSERT_EQUALS(h.getRankPoints(606), 392);
    TS_ASSERT_EQUALS(h.getRankPoints(607), 588);
    TS_ASSERT_EQUALS(h.getRankPoints(608), 784);
    TS_ASSERT_EQUALS(h.getRankPoints(609), 980);
    TS_ASSERT_EQUALS(h.getRankPoints(610), 1371);
    TS_ASSERT_EQUALS(h.getRankPoints(611), 1959);
}

/** Test basic ranking: undo.
    This is the same as testRankingOrder, but we claim in the database to have already given 1000 points to everyone.
    The net result should be the same as for testRankingOrder.

    This is the same as c2systest host/02_rank/undo. */
void
TestServerHostRankVictory::testRankingUndo()
{
    const int GAME_ID = 11111;

    // Create game
    TestHarness h;
    h.createPlayers();
    h.createGame(GAME_ID);
    h.setScore(GAME_ID);
    HashKey(h.db(), Format("game:%d:settings", GAME_ID)).intField("rankTurn").set(20);
    for (int s = 1; s <= 11; ++s) {
        int p = 600 + s;
        HashKey(h.db(), Format("game:%d:rankpoints", GAME_ID)).intField(Format("%d", p)).set(1000);
        HashKey(h.db(), Format("user:%d:profile", p)).intField("rankpoints").set(1000);
    }

    // Rank it
    server::host::Game g(h.root(), GAME_ID);
    server::host::rank::checkForcedGameEnd(g);
    server::host::rank::computeGameRankings(h.root(), g);

    // Verify
    TS_ASSERT_EQUALS(h.getRankPoints(601), 98);
    TS_ASSERT_EQUALS(h.getRankPoints(602), 98);
    TS_ASSERT_EQUALS(h.getRankPoints(603), 98);
    TS_ASSERT_EQUALS(h.getRankPoints(604), 197);
    TS_ASSERT_EQUALS(h.getRankPoints(605), 295);
    TS_ASSERT_EQUALS(h.getRankPoints(606), 393);
    TS_ASSERT_EQUALS(h.getRankPoints(607), 589);
    TS_ASSERT_EQUALS(h.getRankPoints(608), 786);
    TS_ASSERT_EQUALS(h.getRankPoints(609), 982);
    TS_ASSERT_EQUALS(h.getRankPoints(610), 1375);
    TS_ASSERT_EQUALS(h.getRankPoints(611), 1964);
}

/** Test ranking with predefined ranks.
    This does not compute the ranks based on a score, but preconfigures the ranks. */
void
TestServerHostRankVictory::testRankingPredef()
{
    const int GAME_ID = 7;

    // Create game
    TestHarness h;
    h.createPlayers();
    h.createGame(GAME_ID);
    for (int s = 1; s <= 11; ++s) {
        // Ranks need not be contiguous
        HashKey(h.db(), Format("game:%d:player:%d:status", GAME_ID, s)).intField("rank").set(s*s);
    }

    // Rank it
    server::host::Game g(h.root(), GAME_ID);
    server::host::rank::computeGameRankings(h.root(), g);

    // Verify
    TS_ASSERT_EQUALS(h.getRankPoints(601), 1964);
    TS_ASSERT_EQUALS(h.getRankPoints(602), 1375);
    TS_ASSERT_EQUALS(h.getRankPoints(603), 982);
    TS_ASSERT_EQUALS(h.getRankPoints(604), 786);
    TS_ASSERT_EQUALS(h.getRankPoints(605), 589);
    TS_ASSERT_EQUALS(h.getRankPoints(606), 393);
    TS_ASSERT_EQUALS(h.getRankPoints(607), 295);
    TS_ASSERT_EQUALS(h.getRankPoints(608), 197);
    TS_ASSERT_EQUALS(h.getRankPoints(609), 98);
    TS_ASSERT_EQUALS(h.getRankPoints(610), 98);
    TS_ASSERT_EQUALS(h.getRankPoints(611), 98);
}

/** Test score condition. */
void
TestServerHostRankVictory::testScoreCondition()
{
    const int GAME_ID = 7;

    // Create game
    TestHarness h;
    h.createPlayers();
    h.createGame(GAME_ID);

    // Configure "end if someone has 100 planets for 4 turns" score
    HashKey(h.db(), Format("game:%d:settings", GAME_ID)).stringField("endCondition").set("score");
    HashKey(h.db(), Format("game:%d:settings", GAME_ID)).stringField("endScoreName").set("planets");
    HashKey(h.db(), Format("game:%d:settings", GAME_ID)).intField("endTurn").set(4);
    HashKey(h.db(), Format("game:%d:settings", GAME_ID)).intField("endScore").set(100);

    // Create turn scores for 60 turns
    // Scores are turn x slot / 2.
    // That is, in turn 19, player 11 has 11*19/2 = 104 planets and fulfils the criterion for the first time.
    for (int t = 1; t <= 60; ++t) {
        String_t scores;
        for (int s = 1; s <= 11; ++s) {
            int32_t score = t*s/2;
            scores += char(score & 255);
            scores += char((score>>8) & 255);
            scores += char((score>>16) & 255);
            scores += char((score>>24) & 255);
        }
        HashKey(h.db(), Format("game:%d:turn:%d:scores", GAME_ID, t)).stringField("planets").set(scores);
    }

    // Rate it in turn 1. Cannot exit.
    server::host::Game g(h.root(), GAME_ID);
    HashKey(h.db(), Format("game:%d:settings", GAME_ID)).intField("turn").set(1);
    TS_ASSERT(!server::host::rank::checkVictory(h.root(), "egal", g));

    // Rate it in turn 5. No winner yet.
    HashKey(h.db(), Format("game:%d:settings", GAME_ID)).intField("turn").set(5);
    TS_ASSERT(!server::host::rank::checkVictory(h.root(), "egal", g));

    // Rate it in turn 19. First over limit, no winner yet.
    HashKey(h.db(), Format("game:%d:settings", GAME_ID)).intField("turn").set(19);
    TS_ASSERT(!server::host::rank::checkVictory(h.root(), "egal", g));

    // Rate it in turn 21. Not yet a winner.
    HashKey(h.db(), Format("game:%d:settings", GAME_ID)).intField("turn").set(21);
    TS_ASSERT(!server::host::rank::checkVictory(h.root(), "egal", g));

    // Rate it in turn 22. Got a winner.
    HashKey(h.db(), Format("game:%d:settings", GAME_ID)).intField("turn").set(22);
    TS_ASSERT(server::host::rank::checkVictory(h.root(), "egal", g));

    // Check rankings
    for (int s = 1; s <= 11; ++s) {
        TS_ASSERT_EQUALS(HashKey(h.db(), Format("game:%d:player:%d:status", GAME_ID, s)).intField("rank").get(), 12-s);
    }
}

