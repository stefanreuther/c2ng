/**
  *  \file u/t_server_host_game.cpp
  *  \brief Test for server::host::Game
  *
  *  The idea for this one is to test most complex operations,
  *  but not every individual getter.
  *
  *  Conformance to physical storage format is also tested by the system test;
  *  coverage for small getters is achieved by testing "outer" components
  *  such as command handlers.
  */

#include "server/host/game.hpp"

#include "t_server_host.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/string/format.hpp"
#include "afl/test/commandhandler.hpp"
#include "game/test/files.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/host/configuration.hpp"
#include "server/host/root.hpp"
#include "server/host/talkadapter.hpp"
#include "server/interface/hostplayer.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "server/interface/talkforumclient.hpp"
#include "util/processrunner.hpp"

using afl::net::redis::HashKey;
using afl::net::redis::IntegerSetKey;
using afl::net::redis::StringKey;
using afl::net::redis::StringListKey;
using afl::net::redis::StringSetKey;
using afl::net::redis::Subtree;
using afl::string::Format;
using server::interface::HostGame;
using server::interface::HostPlayer;

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

/** Test creating a Game object normally.
    Must succeed and allow querying the Id. */
void
TestServerHostGame::testCreateNormal()
{
    const int32_t GAME_ID = 150;

    TestHarness h;
    IntegerSetKey(h.db(), "game:all").add(GAME_ID);

    TS_ASSERT_THROWS_NOTHING(server::host::Game(h.root(), GAME_ID));
    TS_ASSERT_THROWS_NOTHING(server::host::Game(h.root(), GAME_ID, server::host::Game::NoExistanceCheck));
    TS_ASSERT_EQUALS(server::host::Game(h.root(), GAME_ID).getId(), GAME_ID);
}

/** Test creating a Game object normally for a non-existant game.
    Must throw. */
void
TestServerHostGame::testCreateNonexistant()
{
    TestHarness h;
    TS_ASSERT_THROWS(server::host::Game(h.root(), 150), std::exception);
}

/** Test creating a Game object without database check.
    It must not talk to any microservice. */
void
TestServerHostGame::testCreateUnchecked()
{
    // The CommandHandler will complain bitterly when being talked to.
    afl::test::CommandHandler sensitiveCH("testCreateUnchecked");

    // Remainder of environment
    server::interface::TalkForumClient forum(sensitiveCH);
    server::interface::MailQueueClient mailQueue(sensitiveCH);
    server::host::TalkAdapter forumAdapter(forum);
    util::ProcessRunner runner;
    afl::io::NullFileSystem fs;
    server::host::Root root(sensitiveCH, sensitiveCH, sensitiveCH, mailQueue, runner, fs, server::host::Configuration());
    root.setForum(&forumAdapter);

    // Create the Game object
    TS_ASSERT_THROWS_NOTHING(server::host::Game(root, 49, server::host::Game::NoExistanceCheck));
}

/** Test describe(). */
void
TestServerHostGame::testDescribe()
{
    using server::interface::HostGame;
    const int32_t GAME_ID = 42;
    TestHarness h;

    // Create game
    IntegerSetKey(h.db(), "game:all").add(GAME_ID);
    Subtree t(Subtree(h.db(), "game:").subtree(GAME_ID));
    t.stringKey("name").set("the name");
    t.stringKey("state").set("running");
    t.stringKey("type").set("unlisted");
    t.hashKey("settings").intField("turn").set(12);
    t.hashKey("settings").stringField("description").set("the description");
    t.hashKey("settings").stringField("host").set("P");
    t.hashKey("settings").stringField("master").set("M");
    t.hashKey("settings").stringField("shiplist").set("S");
    t.hashKey("settings").intField("forum").set(46);

    // Player 3 has a yellow turn
    t.hashKey("player:3:status").intField("slot").set(1);
    t.hashKey("player:3:status").intField("turn").set(2);
    t.stringListKey("player:3:users").pushFront("user-a");

    // Player 9 has no turn
    t.hashKey("player:9:status").intField("slot").set(1);
    t.stringListKey("player:9:users").pushFront("user-b");

    // Player 11 is open
    t.hashKey("player:11:status").intField("slot").set(1);

    t.hashKey("turn:12:scores").stringField("timscore").set(String_t("\1\0\0\0\2\0\0\0\3\0\0\0\4\0\0\0\5\0\0\0\6\0\0\0\7\0\0\0\10\0\0\0\11\0\0\0\12\0\0\0\13\0\0\0", 44));

    // Environment
    HashKey(h.db(), "prog:host:prog:P").stringField("description").set("a host");
    HashKey(h.db(), "prog:master:prog:M").stringField("description").set("a master");
    HashKey(h.db(), "prog:sl:prog:S").stringField("description").set("a shiplist");

    // Query
    {
        // Not verbose
        HostGame::Info i = server::host::Game(h.root(), GAME_ID).describe(false, "user-a", h.root());
        TS_ASSERT_EQUALS(i.gameId, GAME_ID);
        TS_ASSERT_EQUALS(i.state, HostGame::Running);
        TS_ASSERT_EQUALS(i.type, HostGame::UnlistedGame);
        TS_ASSERT_EQUALS(i.name, "the name");
        TS_ASSERT_EQUALS(i.difficulty, 100); // because there are no files that change it
        TS_ASSERT(!i.currentSchedule.isValid());
        TS_ASSERT_EQUALS(i.hostName, "P");
        TS_ASSERT_EQUALS(i.hostDescription, "a host");
        TS_ASSERT_EQUALS(i.shipListName, "S");
        TS_ASSERT_EQUALS(i.shipListDescription, "a shiplist");
        TS_ASSERT_EQUALS(i.turnNumber, 12);
    }
    {
        // Verbose
        HostGame::Info i = server::host::Game(h.root(), GAME_ID).describe(true, "user-a", h.root());
        TS_ASSERT_EQUALS(i.gameId, GAME_ID);
        TS_ASSERT_EQUALS(i.state, HostGame::Running);
        TS_ASSERT_EQUALS(i.type, HostGame::UnlistedGame);
        TS_ASSERT_EQUALS(i.name, "the name");
        TS_ASSERT(i.description.isSame(String_t("the description")));

        TS_ASSERT(i.slotStates.isValid());
        TS_ASSERT_EQUALS(i.slotStates.get()->size(), 11U);
        TS_ASSERT_EQUALS(i.slotStates.get()->at(0), HostGame::DeadSlot);
        TS_ASSERT_EQUALS(i.slotStates.get()->at(2), HostGame::SelfSlot); // player 3
        TS_ASSERT_EQUALS(i.slotStates.get()->at(8), HostGame::OccupiedSlot); // player 9
        TS_ASSERT_EQUALS(i.slotStates.get()->at(10), HostGame::OpenSlot); // player 11

        TS_ASSERT(i.turnStates.isValid());
        TS_ASSERT_EQUALS(i.turnStates.get()->size(), 11U);
        TS_ASSERT_EQUALS(i.turnStates.get()->at(0), 0);
        TS_ASSERT_EQUALS(i.turnStates.get()->at(2), 2); // player 3

        TS_ASSERT(i.joinable.isSame(false));

        TS_ASSERT(i.scores.isValid());
        TS_ASSERT_EQUALS(i.scores.get()->at(2), 3);

        TS_ASSERT(i.scoreName.isSame(String_t("timscore")));
        TS_ASSERT(i.scoreDescription.isSame(String_t("Classic Score")));
        TS_ASSERT(i.masterName.isSame(String_t("M")));
        TS_ASSERT(i.masterDescription.isSame(String_t("a master")));
        TS_ASSERT(i.forumId.isSame(46));
    }
    {
        // Verbose, as user C
        HostGame::Info i = server::host::Game(h.root(), GAME_ID).describe(true, "user-c", h.root());

        TS_ASSERT(i.slotStates.isValid());
        TS_ASSERT_EQUALS(i.slotStates.get()->size(), 11U);
        TS_ASSERT_EQUALS(i.slotStates.get()->at(2), HostGame::OccupiedSlot); // player 3
        TS_ASSERT_EQUALS(i.slotStates.get()->at(8), HostGame::OccupiedSlot); // player 9

        TS_ASSERT(i.turnStates.isValid());
        TS_ASSERT_EQUALS(i.turnStates.get()->size(), 11U);
        TS_ASSERT_EQUALS(i.turnStates.get()->at(2), 1); // player 3 - difference, Yellow is mapped to Green

        TS_ASSERT(i.joinable.isSame(true));
    }
}

/** Test getState. */
void
TestServerHostGame::testGetState()
{
    TestHarness h;

    // Normal case
    {
        IntegerSetKey(h.db(), "game:all").add(3);
        StringKey(h.db(), "game:3:state").set("running");
        server::host::Game g(h.root(), 3);
        TS_ASSERT_EQUALS(g.getState(), HostGame::Running);
    }

    // Error case
    {
        IntegerSetKey(h.db(), "game:all").add(7);
        StringKey(h.db(), "game:7:state").set("joking");
        server::host::Game g(h.root(), 7);
        TS_ASSERT_THROWS(g.getState(), std::exception);
    }
}

/** Test setState(), normal case. */
void
TestServerHostGame::testSetStateNormal()
{
    // Prepare
    TestHarness h;
    IntegerSetKey(h.db(), "game:all").add(98);
    IntegerSetKey(h.db(), "game:state:preparing").add(98);
    IntegerSetKey(h.db(), "game:pubstate:preparing").add(98);
    StringKey(h.db(), "game:98:state").set("preparing");
    StringKey(h.db(), "game:98:type").set("public");

    // Set
    server::host::Game(h.root(), 98).setState(HostGame::Joining, h.root().getForum(), h.root());

    // Verify state
    TS_ASSERT_EQUALS(StringKey(h.db(), "game:98:state").get(), "joining");
    TS_ASSERT_EQUALS(IntegerSetKey(h.db(), "game:state:preparing").size(), 0);
    TS_ASSERT_EQUALS(IntegerSetKey(h.db(), "game:pubstate:preparing").size(), 0);
    TS_ASSERT(IntegerSetKey(h.db(), "game:state:joining").contains(98));
    TS_ASSERT(IntegerSetKey(h.db(), "game:pubstate:joining").contains(98));

    // Verify history
    TS_ASSERT_EQUALS(StringListKey(h.db(), "global:history").size(), 1);
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:98:history").size(), 1);

    String_t a = StringListKey(h.db(), "global:history")[0];
    String_t::size_type n = a.find(':');
    TS_ASSERT(n != String_t::npos);
    TS_ASSERT_EQUALS(a.substr(n), ":game-state:98:joining");
    TS_ASSERT_EQUALS(a, StringListKey(h.db(), "game:98:history")[0]);
}

/** Test setState(), private game. */
void
TestServerHostGame::testSetStatePrivate()
{
    // Prepare
    TestHarness h;
    IntegerSetKey(h.db(), "game:all").add(150);
    IntegerSetKey(h.db(), "game:state:preparing").add(150);
    StringKey(h.db(), "game:150:state").set("preparing");
    StringKey(h.db(), "game:150:type").set("private");

    // Set
    server::host::Game(h.root(), 150).setState(HostGame::Joining, h.root().getForum(), h.root());

    // Verify state
    TS_ASSERT_EQUALS(StringKey(h.db(), "game:150:state").get(), "joining");
    TS_ASSERT_EQUALS(IntegerSetKey(h.db(), "game:state:preparing").size(), 0);
    TS_ASSERT(IntegerSetKey(h.db(), "game:state:joining").contains(150));
    TS_ASSERT(!IntegerSetKey(h.db(), "game:pubstate:joining").contains(150));

    // Verify history
    TS_ASSERT_EQUALS(StringListKey(h.db(), "global:history").size(), 0);
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:150:history").size(), 1);

    String_t a = StringListKey(h.db(), "game:150:history")[0];
    String_t::size_type n = a.find(':');
    TS_ASSERT(n != String_t::npos);
    TS_ASSERT_EQUALS(a.substr(n), ":game-state:150:joining");
}

/** Test setState() to finish a game. */
void
TestServerHostGame::testSetStateFinish()
{
    // Prepare
    TestHarness h;
    IntegerSetKey(h.db(), "game:all").add(150);
    IntegerSetKey(h.db(), "game:state:running").add(150);
    IntegerSetKey(h.db(), "game:pubstate:running").add(150);
    StringKey(h.db(), "game:150:state").set("running");
    StringKey(h.db(), "game:150:type").set("public");

    // Add slots
    for (int i = 1; i <= server::host::Game::NUM_PLAYERS; ++i) {
        Subtree t(h.db(), Format("game:150:player:%d:", i));
        t.hashKey("status").intField("slot").set(1);
        t.hashKey("status").intField("turn").set(1);
        // This formula assigns ranks [6,7,8,9,10,11,1,2,3,4,5]
        t.hashKey("status").intField("rank").set(1 + (4+i) % server::host::Game::NUM_PLAYERS);
        t.stringListKey("users").pushBack(Format("u%d", i));
    }

    // Set
    server::host::Game(h.root(), 150).setState(HostGame::Finished, h.root().getForum(), h.root());

    // Verify state
    TS_ASSERT_EQUALS(StringKey(h.db(), "game:150:state").get(), "finished");
    TS_ASSERT_EQUALS(IntegerSetKey(h.db(), "game:state:running").size(), 0);
    TS_ASSERT(IntegerSetKey(h.db(), "game:state:finished").contains(150));
    TS_ASSERT(IntegerSetKey(h.db(), "game:pubstate:finished").contains(150));

    // Verify history
    TS_ASSERT_EQUALS(StringListKey(h.db(), "global:history").size(), 1);
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:150:history").size(), 1);

    String_t a = StringListKey(h.db(), "game:150:history")[0];
    String_t::size_type n = a.find(':');
    TS_ASSERT(n != String_t::npos);
    TS_ASSERT_EQUALS(a.substr(n), ":game-state:150:finished:u7");
    TS_ASSERT_EQUALS(a, StringListKey(h.db(), "global:history")[0]);
}

/** Test setState() to finish a game, no clear winner. */
void
TestServerHostGame::testSetStateFinishAmbiguous()
{
    // Prepare
    TestHarness h;
    IntegerSetKey(h.db(), "game:all").add(150);
    IntegerSetKey(h.db(), "game:state:running").add(150);
    IntegerSetKey(h.db(), "game:pubstate:running").add(150);
    StringKey(h.db(), "game:150:state").set("running");
    StringKey(h.db(), "game:150:type").set("public");

    // Add slots
    for (int i = 1; i <= server::host::Game::NUM_PLAYERS; ++i) {
        Subtree t(h.db(), Format("game:150:player:%d:", i));
        t.hashKey("status").intField("slot").set(1);
        t.hashKey("status").intField("turn").set(1);
        // This formula assigns ranks [2,3,4,5,1,2,3,4,5,1,2]
        t.hashKey("status").intField("rank").set(1 + i % 5);
        t.stringListKey("users").pushBack(Format("u%d", i));
    }

    // Set
    server::host::Game(h.root(), 150).setState(HostGame::Finished, h.root().getForum(), h.root());

    // Verify state
    TS_ASSERT_EQUALS(StringKey(h.db(), "game:150:state").get(), "finished");
    TS_ASSERT_EQUALS(IntegerSetKey(h.db(), "game:state:running").size(), 0);
    TS_ASSERT(IntegerSetKey(h.db(), "game:state:finished").contains(150));
    TS_ASSERT(IntegerSetKey(h.db(), "game:pubstate:finished").contains(150));

    // Verify history
    TS_ASSERT_EQUALS(StringListKey(h.db(), "global:history").size(), 1);
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:150:history").size(), 1);

    String_t a = StringListKey(h.db(), "game:150:history")[0];
    String_t::size_type n = a.find(':');
    TS_ASSERT(n != String_t::npos);
    TS_ASSERT_EQUALS(a.substr(n), ":game-state:150:finished");          // note no user listed!
    TS_ASSERT_EQUALS(a, StringListKey(h.db(), "global:history")[0]);
}

/** Test getType(). */
void
TestServerHostGame::testGetType()
{
    TestHarness h;

    // Normal case
    {
        IntegerSetKey(h.db(), "game:all").add(86);
        IntegerSetKey(h.db(), "game:state:preparing").add(86);
        StringKey(h.db(), "game:86:state").set("preparing");
        StringKey(h.db(), "game:86:type").set("private");

        server::host::Game g(h.root(), 86);
        TS_ASSERT_EQUALS(g.getType(), HostGame::PrivateGame);
    }

    // Error case
    {
        IntegerSetKey(h.db(), "game:all").add(72);
        IntegerSetKey(h.db(), "game:state:preparing").add(72);
        StringKey(h.db(), "game:72:state").set("preparing");
        StringKey(h.db(), "game:72:type").set("fun");

        server::host::Game g(h.root(), 72);
        TS_ASSERT_THROWS(g.getType(), std::exception);
    }
}

/** Test setType(). */
void
TestServerHostGame::testSetType()
{
    // Setup
    TestHarness h;
    IntegerSetKey(h.db(), "game:all").add(94);
    IntegerSetKey(h.db(), "game:state:preparing").add(94);
    StringKey(h.db(), "game:94:state").set("preparing");
    StringKey(h.db(), "game:94:type").set("private");

    // Make it public
    server::host::Game g(h.root(), 94);
    g.setType(HostGame::PublicGame, h.root().getForum(), h.root());

    // Verify
    TS_ASSERT_EQUALS(StringKey(h.db(), "game:94:type").get(), "public");
    TS_ASSERT(IntegerSetKey(h.db(), "game:state:preparing").contains(94));
    TS_ASSERT(IntegerSetKey(h.db(), "game:pubstate:preparing").contains(94));

    // Make it unlisted
    g.setType(HostGame::UnlistedGame, h.root().getForum(), h.root());

    // Verify
    TS_ASSERT_EQUALS(StringKey(h.db(), "game:94:type").get(), "unlisted");
    TS_ASSERT(IntegerSetKey(h.db(), "game:state:preparing").contains(94));
    TS_ASSERT(!IntegerSetKey(h.db(), "game:pubstate:preparing").contains(94));
}

/** Test setOwner(). */
void
TestServerHostGame::testSetOwner()
{
    // Setup
    TestHarness h;
    IntegerSetKey(h.db(), "game:all").add(74);
    IntegerSetKey(h.db(), "game:state:preparing").add(74);
    StringKey(h.db(), "game:74:state").set("preparing");
    StringKey(h.db(), "game:74:type").set("private");

    // Give it to user 'x'
    server::host::Game g(h.root(), 74);
    g.setOwner("x", h.root());

    // Verify
    TS_ASSERT_EQUALS(StringKey(h.db(), "game:74:owner").get(), "x");
    TS_ASSERT(IntegerSetKey(h.db(), "user:x:ownedGames").contains(74));

    // Give it to user 'y'
    g.setOwner("y", h.root());

    // Verify
    TS_ASSERT_EQUALS(StringKey(h.db(), "game:74:owner").get(), "y");
    TS_ASSERT(!IntegerSetKey(h.db(), "user:x:ownedGames").contains(74));
    TS_ASSERT(IntegerSetKey(h.db(), "user:y:ownedGames").contains(74));

    // Null assignment (no change)
    g.setOwner("y", h.root());

    // Verify
    TS_ASSERT_EQUALS(StringKey(h.db(), "game:74:owner").get(), "y");
    TS_ASSERT(!IntegerSetKey(h.db(), "user:x:ownedGames").contains(74));
    TS_ASSERT(IntegerSetKey(h.db(), "user:y:ownedGames").contains(74));

    // Make it unowned
    g.setOwner("", h.root());

    // Verify
    TS_ASSERT_EQUALS(StringKey(h.db(), "game:74:owner").get(), "");
    TS_ASSERT(!IntegerSetKey(h.db(), "user:x:ownedGames").contains(74));
    TS_ASSERT(!IntegerSetKey(h.db(), "user:y:ownedGames").contains(74));
    TS_ASSERT(!IntegerSetKey(h.db(), "user::ownedGames").contains(74));
}

/** Test describeSlot(). */
void
TestServerHostGame::testDescribeSlot()
{
    // Setup
    TestHarness h;
    IntegerSetKey(h.db(), "game:all").add(61);
    IntegerSetKey(h.db(), "game:state:joining").add(61);
    StringKey(h.db(), "game:61:state").set("joining");
    StringKey(h.db(), "game:61:type").set("unlisted");

    HashKey(h.db(), "game:61:player:1:status").intField("slot").set(1);
    HashKey(h.db(), "game:61:player:2:status").intField("slot").set(1);
    StringListKey(h.db(), "game:61:player:1:users").pushBack("a");
    StringListKey(h.db(), "game:61:player:1:users").pushBack("b");
    StringListKey(h.db(), "game:61:player:1:users").pushBack("c");
    HashKey(h.db(), "game:61:users").intField("a").set(1);
    HashKey(h.db(), "game:61:users").intField("b").set(1);
    HashKey(h.db(), "game:61:users").intField("c").set(1);

    // Race names
    server::common::RaceNames raceNames;
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    raceNames.load(game::test::getDefaultRaceNames(), cs);

    // Test
    server::host::Game g(h.root(), 61);
    TS_ASSERT(!g.isMultiJoinAllowed());

    HostPlayer::Info a = g.describeSlot(1, "a", raceNames);
    HostPlayer::Info b = g.describeSlot(1, "b", raceNames);
    HostPlayer::Info c = g.describeSlot(1, "c", raceNames);
    HostPlayer::Info d = g.describeSlot(1, "d", raceNames);

    // Verify
    // - a
    TS_ASSERT_EQUALS(a.longName, "The Solar Federation");
    TS_ASSERT_EQUALS(a.shortName, "The Feds");
    TS_ASSERT_EQUALS(a.adjectiveName, "Fed");
    TS_ASSERT_EQUALS(a.userIds.size(), 3U);
    TS_ASSERT_EQUALS(a.userIds[0], "a");
    TS_ASSERT_EQUALS(a.userIds[1], "b");
    TS_ASSERT_EQUALS(a.userIds[2], "c");
    TS_ASSERT_EQUALS(a.numEditable, 3);
    TS_ASSERT_EQUALS(a.joinable, false);

    // - b
    TS_ASSERT_EQUALS(b.longName, a.longName);
    TS_ASSERT_EQUALS(b.shortName, a.shortName);
    TS_ASSERT_EQUALS(b.adjectiveName, a.adjectiveName);
    TS_ASSERT_EQUALS(b.userIds, a.userIds);
    TS_ASSERT_EQUALS(b.numEditable, 2);
    TS_ASSERT_EQUALS(b.joinable, false);

    // - c
    TS_ASSERT_EQUALS(c.numEditable, 1);
    TS_ASSERT_EQUALS(c.joinable, false);

    // - b
    TS_ASSERT_EQUALS(d.numEditable, 0);
    TS_ASSERT_EQUALS(d.joinable, false);

    // Test slot 2
    HostPlayer::Info a2 = g.describeSlot(2, "a", raceNames);
    HostPlayer::Info b2 = g.describeSlot(2, "b", raceNames);
    HostPlayer::Info d2 = g.describeSlot(2, "d", raceNames);

    // - a
    TS_ASSERT_EQUALS(a2.longName, "The Lizard Alliance");
    TS_ASSERT_EQUALS(a2.shortName, "The Lizards");
    TS_ASSERT_EQUALS(a2.adjectiveName, "Lizard");
    TS_ASSERT_EQUALS(a2.userIds.size(), 0U);
    TS_ASSERT_EQUALS(a2.numEditable, 0);
    TS_ASSERT_EQUALS(a2.joinable, false);

    // - b, c
    TS_ASSERT_EQUALS(b2.joinable, true);
    TS_ASSERT_EQUALS(d2.joinable, true);
}

/** Test describeVictoryCondition(), no condition set. */
void
TestServerHostGame::testDescribeVictoryNone()
{
    // Setup
    TestHarness h;
    IntegerSetKey(h.db(), "game:all").add(61);
    IntegerSetKey(h.db(), "game:state:joining").add(61);
    StringKey(h.db(), "game:61:state").set("joining");
    StringKey(h.db(), "game:61:type").set("unlisted");

    // Test
    server::host::Game g(h.root(), 61);
    HostGame::VictoryCondition vc = g.describeVictoryCondition(h.root());

    // Verify
    TS_ASSERT_EQUALS(vc.endCondition, "");
    TS_ASSERT(!vc.endTurn.isValid());
    TS_ASSERT(!vc.endProbability.isValid());
    TS_ASSERT(!vc.endScore.isValid());
    TS_ASSERT(!vc.endScoreName.isValid());
    TS_ASSERT(!vc.endScoreDescription.isValid());
    TS_ASSERT(!vc.referee.isValid());
    TS_ASSERT(!vc.refereeDescription.isValid());
}

/** Test describeVictoryCondition(), turn condition. */
void
TestServerHostGame::testDescribeVictoryTurn()
{
    // Setup
    TestHarness h;
    IntegerSetKey(h.db(), "game:all").add(61);
    IntegerSetKey(h.db(), "game:state:joining").add(61);
    StringKey(h.db(), "game:61:state").set("joining");
    StringKey(h.db(), "game:61:type").set("unlisted");

    HashKey(h.db(), "game:61:settings").stringField("endCondition").set("turn");
    HashKey(h.db(), "game:61:settings").intField("endTurn").set(100);
    HashKey(h.db(), "game:61:settings").intField("endProbability").set(35);

    // Test
    server::host::Game g(h.root(), 61);
    HostGame::VictoryCondition vc = g.describeVictoryCondition(h.root());

    // Verify
    TS_ASSERT_EQUALS(vc.endCondition, "turn");
    TS_ASSERT(vc.endTurn.isSame(100));
    TS_ASSERT(vc.endProbability.isSame(35));
    TS_ASSERT(!vc.endScore.isValid());
    TS_ASSERT(!vc.endScoreName.isValid());
    TS_ASSERT(!vc.endScoreDescription.isValid());
    TS_ASSERT(!vc.referee.isValid());
    TS_ASSERT(!vc.refereeDescription.isValid());
}

/** Test describeVictoryCondition(), score condition. */
void
TestServerHostGame::testDescribeVictoryScore()
{
    // Setup
    TestHarness h;
    IntegerSetKey(h.db(), "game:all").add(61);
    IntegerSetKey(h.db(), "game:state:joining").add(61);
    StringKey(h.db(), "game:61:state").set("joining");
    StringKey(h.db(), "game:61:type").set("unlisted");

    HashKey(h.db(), "game:61:settings").stringField("endCondition").set("score");
    HashKey(h.db(), "game:61:settings").intField("endScore").set(15000);
    HashKey(h.db(), "game:61:settings").stringField("endScoreName").set("xscore");
    HashKey(h.db(), "game:61:scores").stringField("xscore").set("X!");

    // Test
    server::host::Game g(h.root(), 61);
    HostGame::VictoryCondition vc = g.describeVictoryCondition(h.root());

    // Verify
    TS_ASSERT_EQUALS(vc.endCondition, "score");
    TS_ASSERT(vc.endTurn.isSame(1));                // implied "must hold score for 1 turn"
    TS_ASSERT(!vc.endProbability.isValid());
    TS_ASSERT(vc.endScore.isSame(15000));
    TS_ASSERT(vc.endScoreName.isSame(String_t("xscore")));
    TS_ASSERT(vc.endScoreDescription.isSame(String_t("X!")));
    TS_ASSERT(!vc.referee.isValid());
    TS_ASSERT(!vc.refereeDescription.isValid());
}

/** Test describeVictoryCondition(), referee tool. */
void
TestServerHostGame::testDescribeVictoryReferee()
{
    // Setup
    TestHarness h;
    IntegerSetKey(h.db(), "game:all").add(61);
    IntegerSetKey(h.db(), "game:state:joining").add(61);
    StringKey(h.db(), "game:61:state").set("joining");
    StringKey(h.db(), "game:61:type").set("unlisted");

    HashKey(h.db(), "prog:tool:prog:judge").stringField("description").set("Dredd");
    HashKey(h.db(), "prog:tool:prog:judge").stringField("type").set("referee");
    StringSetKey(h.db(), "prog:tool:list").add("judge");

    StringSetKey(h.db(), "game:61:tools").add("judge");
    HashKey(h.db(), "game:61:toolkind").stringField("referee").set("judge");

    // Test
    server::host::Game g(h.root(), 61);
    HostGame::VictoryCondition vc = g.describeVictoryCondition(h.root());

    // Verify
    TS_ASSERT_EQUALS(vc.endCondition, "");
    TS_ASSERT(!vc.endTurn.isValid());
    TS_ASSERT(!vc.endProbability.isValid());
    TS_ASSERT(!vc.endScore.isValid());
    TS_ASSERT(!vc.endScoreName.isValid());
    TS_ASSERT(!vc.endScoreDescription.isValid());
    TS_ASSERT(vc.referee.isSame(String_t("judge")));
    TS_ASSERT(vc.refereeDescription.isSame(String_t("Dredd")));
}
