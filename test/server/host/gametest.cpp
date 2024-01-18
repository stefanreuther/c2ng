/**
  *  \file test/server/host/gametest.cpp
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
#include "afl/test/testrunner.hpp"
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
AFL_TEST("server.host.Game:create:normal", a)
{
    const int32_t GAME_ID = 150;

    TestHarness h;
    IntegerSetKey(h.db(), "game:all").add(GAME_ID);

    AFL_CHECK_SUCCEEDS(a("01. create"), server::host::Game(h.root(), GAME_ID));
    AFL_CHECK_SUCCEEDS(a("02. create"), server::host::Game(h.root(), GAME_ID, server::host::Game::NoExistanceCheck));
    a.checkEqual("03. getId", server::host::Game(h.root(), GAME_ID).getId(), GAME_ID);
}

/** Test creating a Game object normally for a non-existant game.
    Must throw. */
AFL_TEST("server.host.Game:create:nonexistant", a)
{
    TestHarness h;
    AFL_CHECK_THROWS(a, server::host::Game(h.root(), 150), std::exception);
}

/** Test creating a Game object without database check.
    It must not talk to any microservice. */
AFL_TEST("server.host.Game:create:unchecked", a)
{
    // The CommandHandler will complain bitterly when being talked to.
    afl::test::CommandHandler sensitiveCH(a);

    // Remainder of environment
    server::interface::TalkForumClient forum(sensitiveCH);
    server::interface::MailQueueClient mailQueue(sensitiveCH);
    server::host::TalkAdapter forumAdapter(forum);
    util::ProcessRunner runner;
    afl::io::NullFileSystem fs;
    server::host::Root root(sensitiveCH, sensitiveCH, sensitiveCH, mailQueue, runner, fs, server::host::Configuration());
    root.setForum(&forumAdapter);

    // Create the Game object
    AFL_CHECK_SUCCEEDS(a, server::host::Game(root, 49, server::host::Game::NoExistanceCheck));
}

/** Test describe(). */
AFL_TEST("server.host.Game:describe", a)
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
    t.hashKey("settings").intField("minRankLevelToJoin").set(4);

    // Player 3 has a yellow turn
    t.hashKey("player:3:status").intField("slot").set(1);
    t.hashKey("player:3:status").intField("turn").set(2);
    t.stringListKey("player:3:users").pushFront("user-a");

    // Player 9 has no turn
    t.hashKey("player:9:status").intField("slot").set(1);
    t.stringListKey("player:9:users").pushFront("user-b");

    // Player 11 is open
    t.hashKey("player:11:status").intField("slot").set(1);

    // Reference counters
    t.hashKey("users").intField("user-a").set(1);
    t.hashKey("users").intField("user-b").set(1);

    t.hashKey("turn:12:scores").stringField("timscore").set(String_t("\1\0\0\0\2\0\0\0\3\0\0\0\4\0\0\0\5\0\0\0\6\0\0\0\7\0\0\0\10\0\0\0\11\0\0\0\12\0\0\0\13\0\0\0", 44));

    // Environment
    HashKey(h.db(), "prog:host:prog:P").stringField("description").set("a host");
    HashKey(h.db(), "prog:host:prog:P").stringField("kind").set("host kind");
    HashKey(h.db(), "prog:master:prog:M").stringField("description").set("a master");
    HashKey(h.db(), "prog:master:prog:M").stringField("kind").set("master kind");
    HashKey(h.db(), "prog:sl:prog:S").stringField("description").set("a shiplist");
    HashKey(h.db(), "prog:sl:prog:S").stringField("kind").set("shiplist kind");
    HashKey(h.db(), "user:user-c:profile").intField("rank").set(4);
    HashKey(h.db(), "user:user-d:profile").intField("rank").set(3);

    // Query
    {
        // Not verbose
        HostGame::Info i = server::host::Game(h.root(), GAME_ID).describe(false, "user-a", "", h.root());
        a.checkEqual("01. gameId",              i.gameId, GAME_ID);
        a.checkEqual("02. state",               i.state, HostGame::Running);
        a.checkEqual("03. type",                i.type, HostGame::UnlistedGame);
        a.checkEqual("04. name",                i.name, "the name");
        a.checkEqual("05. difficulty",          i.difficulty, 100); // because there are no files that change it
        a.check     ("06. currentSchedule",    !i.currentSchedule.isValid());
        a.checkEqual("07. hostName",            i.hostName, "P");
        a.checkEqual("08. hostDescription",     i.hostDescription, "a host");
        a.checkEqual("09. hostKind",            i.hostKind, "host kind");
        a.checkEqual("10. shipListName",        i.shipListName, "S");
        a.checkEqual("11. shipListDescription", i.shipListDescription, "a shiplist");
        a.checkEqual("12. shipListKind",        i.shipListKind, "shiplist kind");
        a.checkEqual("13. turnNumber",          i.turnNumber, 12);
        a.check     ("14. userPlays",           i.userPlays.isSame(true));
    }
    {
        // Verbose
        HostGame::Info i = server::host::Game(h.root(), GAME_ID).describe(true, "user-a", "", h.root());
        a.checkEqual("15. gameId",      i.gameId, GAME_ID);
        a.checkEqual("16. state",       i.state, HostGame::Running);
        a.checkEqual("17. type",        i.type, HostGame::UnlistedGame);
        a.checkEqual("18. name",        i.name, "the name");
        a.check     ("19. description", i.description.isSame(String_t("the description")));

        a.check     ("21. slotStates",  i.slotStates.isValid());
        a.checkEqual("22. slotStates",  i.slotStates.get()->size(), 11U);
        a.checkEqual("23. slotStates",  i.slotStates.get()->at(0), HostGame::DeadSlot);
        a.checkEqual("24. slotStates",  i.slotStates.get()->at(2), HostGame::SelfSlot); // player 3
        a.checkEqual("25. slotStates",  i.slotStates.get()->at(8), HostGame::OccupiedSlot); // player 9
        a.checkEqual("26. slotStates",  i.slotStates.get()->at(10), HostGame::OpenSlot); // player 11

        a.check     ("31. turnStates",  i.turnStates.isValid());
        a.checkEqual("32. turnStates",  i.turnStates.get()->size(), 11U);
        a.checkEqual("33. turnStates",  i.turnStates.get()->at(0), 0);
        a.checkEqual("34. turnStates",  i.turnStates.get()->at(2), 2); // player 3

        a.check     ("41. joinable",    i.joinable.isSame(false));
        a.check     ("42. userPlays",   i.userPlays.isSame(true));

        a.checkEqual("51. minRankLevelToJoin",  i.minRankLevelToJoin.orElse(-1), 4);
        a.checkEqual("52. maxRankLevelToJoin",  i.maxRankLevelToJoin.isValid(), false);
        a.checkEqual("53. minRankPointsToJoin", i.minRankPointsToJoin.isValid(), false);
        a.checkEqual("54. maxRankLevelToJoin",  i.maxRankPointsToJoin.isValid(), false);

        a.check     ("61. scores", i.scores.isValid());
        a.checkEqual("62. scores", i.scores.get()->at(2), 3);

        a.check("71. scoreName",         i.scoreName.isSame(String_t("timscore")));
        a.check("72. scoreDescription",  i.scoreDescription.isSame(String_t("Classic Score")));
        a.check("73. masterName",        i.masterName.isSame(String_t("M")));
        a.check("74. masterDescription", i.masterDescription.isSame(String_t("a master")));
        a.check("75. masterKind",        i.masterKind.isSame(String_t("master kind")));
        a.check("76. forumId",           i.forumId.isSame(46));
    }
    {
        // Verbose, as user C
        HostGame::Info i = server::host::Game(h.root(), GAME_ID).describe(true, "user-c", "", h.root());

        a.check     ("81. slotStates", i.slotStates.isValid());
        a.checkEqual("82. slotStates", i.slotStates.get()->size(), 11U);
        a.checkEqual("83. slotStates", i.slotStates.get()->at(2), HostGame::OccupiedSlot); // player 3
        a.checkEqual("84. slotStates", i.slotStates.get()->at(8), HostGame::OccupiedSlot); // player 9

        a.check     ("91. turnStates", i.turnStates.isValid());
        a.checkEqual("92. turnStates", i.turnStates.get()->size(), 11U);
        a.checkEqual("93. turnStates", i.turnStates.get()->at(2), 1); // player 3 - difference, Yellow is mapped to Green

        a.check("101. joinable", i.joinable.isSame(true));
        a.check("102. userPlays", i.userPlays.isSame(false));
    }
    {
        // Verbose, as user D - not joinable
        HostGame::Info i = server::host::Game(h.root(), GAME_ID).describe(true, "user-d", "", h.root());
        a.check("103. joinable", i.joinable.isSame(false));
    }
}

/*
 *  Test getState.
 */

// Normal case
AFL_TEST("server.host.Game:getState:normal", a)
{
    TestHarness h;
    IntegerSetKey(h.db(), "game:all").add(3);
    StringKey(h.db(), "game:3:state").set("running");
    server::host::Game g(h.root(), 3);
    a.checkEqual("getState", g.getState(), HostGame::Running);
}

// Error case
AFL_TEST("server.host.Game:getState:error", a)
{
    TestHarness h;
    IntegerSetKey(h.db(), "game:all").add(7);
    StringKey(h.db(), "game:7:state").set("joking");
    server::host::Game g(h.root(), 7);
    AFL_CHECK_THROWS(a, g.getState(), std::exception);
}

/** Test setState(), normal case. */
AFL_TEST("server.host.Game:setState:normal", a)
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
    a.checkEqual("01. state",        StringKey(h.db(), "game:98:state").get(), "joining");
    a.checkEqual("02. state set",    IntegerSetKey(h.db(), "game:state:preparing").size(), 0);
    a.checkEqual("03. pubstate set", IntegerSetKey(h.db(), "game:pubstate:preparing").size(), 0);
    a.check     ("04. state set",    IntegerSetKey(h.db(), "game:state:joining").contains(98));
    a.check     ("05. pubstate set", IntegerSetKey(h.db(), "game:pubstate:joining").contains(98));

    // Verify history
    a.checkEqual("11. global history", StringListKey(h.db(), "global:history").size(), 1);
    a.checkEqual("12. game history",   StringListKey(h.db(), "game:98:history").size(), 1);

    String_t s = StringListKey(h.db(), "global:history")[0];
    String_t::size_type n = s.find(':');
    a.check("21. history", n != String_t::npos);
    a.checkEqual("22. history", s.substr(n), ":game-state:98:joining");
    a.checkEqual("23. history", s, StringListKey(h.db(), "game:98:history")[0]);
}

/** Test setState(), private game. */
AFL_TEST("server.host.Game:setState:private", a)
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
    a.checkEqual("01. state",         StringKey(h.db(), "game:150:state").get(), "joining");
    a.checkEqual("02. state set",     IntegerSetKey(h.db(), "game:state:preparing").size(), 0);
    a.check     ("03. state set",     IntegerSetKey(h.db(), "game:state:joining").contains(150));
    a.check     ("04. pubstate set", !IntegerSetKey(h.db(), "game:pubstate:joining").contains(150));

    // Verify history
    a.checkEqual("11. global history", StringListKey(h.db(), "global:history").size(), 0);
    a.checkEqual("12. game history",   StringListKey(h.db(), "game:150:history").size(), 1);

    String_t s = StringListKey(h.db(), "game:150:history")[0];
    String_t::size_type n = s.find(':');
    a.check("21. history", n != String_t::npos);
    a.checkEqual("22. history", s.substr(n), ":game-state:150:joining");
}

/** Test setState() to finish a game. */
AFL_TEST("server.host.Game:setState:finished", a)
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
    a.checkEqual("01. state",        StringKey(h.db(), "game:150:state").get(), "finished");
    a.checkEqual("02. state set",    IntegerSetKey(h.db(), "game:state:running").size(), 0);
    a.check     ("03. state set",    IntegerSetKey(h.db(), "game:state:finished").contains(150));
    a.check     ("04. pubstate set", IntegerSetKey(h.db(), "game:pubstate:finished").contains(150));

    // Verify history
    a.checkEqual("11. global history", StringListKey(h.db(), "global:history").size(), 1);
    a.checkEqual("12. game history",   StringListKey(h.db(), "game:150:history").size(), 1);

    String_t s = StringListKey(h.db(), "game:150:history")[0];
    String_t::size_type n = s.find(':');
    a.check("21. history", n != String_t::npos);
    a.checkEqual("22. history", s.substr(n), ":game-state:150:finished:u7");
    a.checkEqual("23. history", s, StringListKey(h.db(), "global:history")[0]);

    // Verify statistic
    server::interface::HostGame::Info info = server::host::Game(h.root(), 150).describe(true, "u1", "u2", h.root());
    a.checkEqual("31. userRank", info.userRank.orElse(-1), 6);
    a.checkEqual("32. otherRank", info.otherRank.orElse(-1), 7);
}

/** Test setState() to finish a game, no clear winner. */
AFL_TEST("server.host.Game:setState:finished:ambiguous-rank", a)
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
    a.checkEqual("01. state",        StringKey(h.db(), "game:150:state").get(), "finished");
    a.checkEqual("02. state set",    IntegerSetKey(h.db(), "game:state:running").size(), 0);
    a.check     ("03. state set",    IntegerSetKey(h.db(), "game:state:finished").contains(150));
    a.check     ("04. pubstate set", IntegerSetKey(h.db(), "game:pubstate:finished").contains(150));

    // Verify history
    a.checkEqual("11. global history", StringListKey(h.db(), "global:history").size(), 1);
    a.checkEqual("12. game history",   StringListKey(h.db(), "game:150:history").size(), 1);

    String_t s = StringListKey(h.db(), "game:150:history")[0];
    String_t::size_type n = s.find(':');
    a.check("21. history", n != String_t::npos);
    a.checkEqual("22. history", s.substr(n), ":game-state:150:finished");          // note no user listed!
    a.checkEqual("23. history", s, StringListKey(h.db(), "global:history")[0]);
}

/*
 *  Test getType().
 */

// Normal case
AFL_TEST("server.host.Game:getType:normal", a)
{
    TestHarness h;
    IntegerSetKey(h.db(), "game:all").add(86);
    IntegerSetKey(h.db(), "game:state:preparing").add(86);
    StringKey(h.db(), "game:86:state").set("preparing");
    StringKey(h.db(), "game:86:type").set("private");

    server::host::Game g(h.root(), 86);
    a.checkEqual("getType", g.getType(), HostGame::PrivateGame);
}

// Error case
AFL_TEST("server.host.Game:getType:error", a)
{
    TestHarness h;
    IntegerSetKey(h.db(), "game:all").add(72);
    IntegerSetKey(h.db(), "game:state:preparing").add(72);
    StringKey(h.db(), "game:72:state").set("preparing");
    StringKey(h.db(), "game:72:type").set("fun");

    server::host::Game g(h.root(), 72);
    AFL_CHECK_THROWS(a, g.getType(), std::exception);
}

/** Test setType(). */
AFL_TEST("server.host.Game:setType", a)
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
    a.checkEqual("01. type",    StringKey(h.db(), "game:94:type").get(), "public");
    a.check("02. state set",    IntegerSetKey(h.db(), "game:state:preparing").contains(94));
    a.check("03. pubstate set", IntegerSetKey(h.db(), "game:pubstate:preparing").contains(94));

    // Make it unlisted
    g.setType(HostGame::UnlistedGame, h.root().getForum(), h.root());

    // Verify
    a.checkEqual("11. type",     StringKey(h.db(), "game:94:type").get(), "unlisted");
    a.check("12. state set",     IntegerSetKey(h.db(), "game:state:preparing").contains(94));
    a.check("13. pubstate set", !IntegerSetKey(h.db(), "game:pubstate:preparing").contains(94));
}

/** Test setOwner(). */
AFL_TEST("server.host.Game:setOwner", a)
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
    a.checkEqual("01. owner", StringKey(h.db(), "game:74:owner").get(), "x");
    a.check("02. user set",   IntegerSetKey(h.db(), "user:x:ownedGames").contains(74));

    // Give it to user 'y'
    g.setOwner("y", h.root());

    // Verify
    a.checkEqual("11. owner", StringKey(h.db(), "game:74:owner").get(), "y");
    a.check("12. user set",  !IntegerSetKey(h.db(), "user:x:ownedGames").contains(74));
    a.check("13. user set",   IntegerSetKey(h.db(), "user:y:ownedGames").contains(74));

    // Null assignment (no change)
    g.setOwner("y", h.root());

    // Verify
    a.checkEqual("21. owner", StringKey(h.db(), "game:74:owner").get(), "y");
    a.check("22. user set",  !IntegerSetKey(h.db(), "user:x:ownedGames").contains(74));
    a.check("23. user set",   IntegerSetKey(h.db(), "user:y:ownedGames").contains(74));

    // Make it unowned
    g.setOwner("", h.root());

    // Verify
    a.checkEqual("31. owner", StringKey(h.db(), "game:74:owner").get(), "");
    a.check("32. user set",  !IntegerSetKey(h.db(), "user:x:ownedGames").contains(74));
    a.check("33. user set",  !IntegerSetKey(h.db(), "user:y:ownedGames").contains(74));
    a.check("34. user set",  !IntegerSetKey(h.db(), "user::ownedGames").contains(74));
}

/** Test describeSlot(). */
AFL_TEST("server.host.Game:describeSlot", a)
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
    a.check("01. isMultiJoinAllowed", !g.isMultiJoinAllowed());

    HostPlayer::Info a1 = g.describeSlot(1, "a", h.root(), raceNames);
    HostPlayer::Info b1 = g.describeSlot(1, "b", h.root(), raceNames);
    HostPlayer::Info c1 = g.describeSlot(1, "c", h.root(), raceNames);
    HostPlayer::Info d1 = g.describeSlot(1, "d", h.root(), raceNames);

    // Verify
    // - a
    a.checkEqual("11. longName",      a1.longName, "The Solar Federation");
    a.checkEqual("12. shortName",     a1.shortName, "The Feds");
    a.checkEqual("13. adjectiveName", a1.adjectiveName, "Fed");
    a.checkEqual("14. userIds",       a1.userIds.size(), 3U);
    a.checkEqual("15. userIds",       a1.userIds[0], "a");
    a.checkEqual("16. userIds",       a1.userIds[1], "b");
    a.checkEqual("17. userIds",       a1.userIds[2], "c");
    a.checkEqual("18. numEditable",   a1.numEditable, 3);
    a.checkEqual("19. joinable",      a1.joinable, false);

    // - b
    a.checkEqual("21. longName",      b1.longName, a1.longName);
    a.checkEqual("22. shortName",     b1.shortName, a1.shortName);
    a.checkEqual("23. adjectiveName", b1.adjectiveName, a1.adjectiveName);
    a.check     ("24. userIds",       b1.userIds == a1.userIds);
    a.checkEqual("25. numEditable",   b1.numEditable, 2);
    a.checkEqual("26. joinable",      b1.joinable, false);

    // - c
    a.checkEqual("31. numEditable",   c1.numEditable, 1);
    a.checkEqual("32. joinable",      c1.joinable, false);

    // - b
    a.checkEqual("41. numEditable",   d1.numEditable, 0);
    a.checkEqual("42. joinable",      d1.joinable, false);

    // Test slot 2
    HostPlayer::Info a2 = g.describeSlot(2, "a", h.root(), raceNames);
    HostPlayer::Info b2 = g.describeSlot(2, "b", h.root(), raceNames);
    HostPlayer::Info d2 = g.describeSlot(2, "d", h.root(), raceNames);

    // - a
    a.checkEqual("51. longName",      a2.longName, "The Lizard Alliance");
    a.checkEqual("52. shortName",     a2.shortName, "The Lizards");
    a.checkEqual("53. adjectiveName", a2.adjectiveName, "Lizard");
    a.checkEqual("54",                a2.userIds.size(), 0U);
    a.checkEqual("55. numEditable",   a2.numEditable, 0);
    a.checkEqual("56. joinable",      a2.joinable, false);

    // - b, c
    a.checkEqual("61. joinable", b2.joinable, true);
    a.checkEqual("62. joinable", d2.joinable, true);
}

/** Test describeVictoryCondition(), no condition set. */
AFL_TEST("server.host.Game:describeVictoryCondition:none", a)
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
    a.checkEqual("01. endCondition",    vc.endCondition, "");
    a.check("02. endTurn",             !vc.endTurn.isValid());
    a.check("03. endProbability",      !vc.endProbability.isValid());
    a.check("04. endScore",            !vc.endScore.isValid());
    a.check("05. endScoreName",        !vc.endScoreName.isValid());
    a.check("06. endScoreDescription", !vc.endScoreDescription.isValid());
    a.check("07. referee",             !vc.referee.isValid());
    a.check("08. refereeDescription",  !vc.refereeDescription.isValid());
}

/** Test describeVictoryCondition(), turn condition. */
AFL_TEST("server.host.Game:describeVictoryCondition:turn", a)
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
    a.checkEqual("01. endCondition",    vc.endCondition, "turn");
    a.check("02. endTurn",              vc.endTurn.isSame(100));
    a.check("03. endProbability",       vc.endProbability.isSame(35));
    a.check("04. endScore",            !vc.endScore.isValid());
    a.check("05. endScoreName",        !vc.endScoreName.isValid());
    a.check("06. endScoreDescription", !vc.endScoreDescription.isValid());
    a.check("07. referee",             !vc.referee.isValid());
    a.check("08. refereeDescription",  !vc.refereeDescription.isValid());
}

/** Test describeVictoryCondition(), score condition. */
AFL_TEST("server.host.Game:describeVictoryCondition:score", a)
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
    a.checkEqual("01. endCondition",   vc.endCondition, "score");
    a.check("02. endTurn",             vc.endTurn.isSame(1));                // implied "must hold score for 1 turn"
    a.check("03. endProbability",     !vc.endProbability.isValid());
    a.check("04. endScore",            vc.endScore.isSame(15000));
    a.check("05. endScoreName",        vc.endScoreName.isSame(String_t("xscore")));
    a.check("06. endScoreDescription", vc.endScoreDescription.isSame(String_t("X!")));
    a.check("07. referee",            !vc.referee.isValid());
    a.check("08. refereeDescription", !vc.refereeDescription.isValid());
}

/** Test describeVictoryCondition(), referee tool. */
AFL_TEST("server.host.Game:describeVictoryCondition:referee", a)
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
    a.checkEqual("01. endCondition",    vc.endCondition, "");
    a.check("02. endTurn",             !vc.endTurn.isValid());
    a.check("03. endProbability",      !vc.endProbability.isValid());
    a.check("04. endScore",            !vc.endScore.isValid());
    a.check("05. endScoreName",        !vc.endScoreName.isValid());
    a.check("06. endScoreDescription", !vc.endScoreDescription.isValid());
    a.check("07. referee",              vc.referee.isSame(String_t("judge")));
    a.check("08. refereeDescription",   vc.refereeDescription.isSame(String_t("Dredd")));
}
