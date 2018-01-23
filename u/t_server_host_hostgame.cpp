/**
  *  \file u/t_server_host_hostgame.cpp
  *  \brief Test for server::host::HostGame
  */

#include "server/host/hostgame.hpp"

#include "t_server_host.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/host/gamearbiter.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "util/processrunner.hpp"
#include "server/host/game.hpp"

using afl::net::redis::HashKey;
using afl::net::redis::StringKey;
using afl::net::redis::StringSetKey;
using server::interface::HostGame;
using server::interface::HostTool;

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

        void addDefaultTools();
        int32_t addGame(HostGame& testee);

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

void
TestHarness::addDefaultTools()
{
    HashKey(m_db, "prog:host:prog:H").stringField("kind").set("host");
    HashKey(m_db, "prog:host:prog:P").stringField("kind").set("host");
    HashKey(m_db, "prog:master:prog:M").stringField("kind").set("master");
    HashKey(m_db, "prog:sl:prog:S").stringField("kind").set("shiplist");
    StringKey(m_db, "prog:host:default").set("H");
    StringKey(m_db, "prog:master:default").set("M");
    StringKey(m_db, "prog:sl:default").set("S");
    StringSetKey(m_db, "prog:host:list").add("H");
    StringSetKey(m_db, "prog:host:list").add("P");
    StringSetKey(m_db, "prog:master:list").add("M");
    StringSetKey(m_db, "prog:dl:list").add("S");
}

int32_t
TestHarness::addGame(HostGame& testee)
{
    int32_t gid = testee.createNewGame();
    testee.setState(gid, HostGame::Joining);
    testee.setType(gid, HostGame::PublicGame);
    testee.setOwner(gid, "z");

    // Join some users
    server::host::Game g(root(), gid);
    g.pushPlayerSlot(1, "a", root());       // primary, active
    g.pushPlayerSlot(2, "b", root());       // primary
    g.pushPlayerSlot(2, "c", root());       // active
    g.pushPlayerSlot(3, "d", root());       // primary
    g.pushPlayerSlot(3, "e", root());       // inactive
    g.pushPlayerSlot(3, "f", root());       // active

    return gid;
}


/** Test createNewGame().
    Tests just basic operation.
    Actual game creation is tested separately.
    A complete test is in c2systest/host/02_newgame. */
void
TestServerHostHostGame::testNewGame()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Creating two games must create distinct Ids
    int32_t a = testee.createNewGame();
    int32_t b = testee.createNewGame();
    TS_ASSERT_EQUALS(a, 1);
    TS_ASSERT_EQUALS(b, 2);

    // Name and type
    TS_ASSERT_EQUALS(testee.getName(a), "New Game");
    TS_ASSERT_EQUALS(testee.getState(a), HostGame::Preparing);
    TS_ASSERT_EQUALS(testee.getType(a), HostGame::PrivateGame);
    TS_ASSERT_EQUALS(testee.getDirectory(a), "games/0001");

    // Stats
    HostGame::Totals t = testee.getTotals();
    TS_ASSERT_EQUALS(t.numJoiningGames, 0);
    TS_ASSERT_EQUALS(t.numRunningGames, 0);
    TS_ASSERT_EQUALS(t.numFinishedGames, 0);
}

/** Test cloneGame(), standard case.
    Tests just basic operation.
    Actual game creation is tested separately.
    A complete test is in c2systest/host/02_newgame. */
void
TestServerHostHostGame::testCloneGame()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create a game and clone it
    int32_t a = testee.createNewGame();
    int32_t b = testee.cloneGame(a, afl::base::Nothing);
    TS_ASSERT_EQUALS(a, 1);
    TS_ASSERT_EQUALS(b, 2);

    // Verify
    TS_ASSERT_EQUALS(testee.getName(b), "New Game 1");
    TS_ASSERT_EQUALS(testee.getState(b), HostGame::Joining);
    TS_ASSERT_EQUALS(testee.getType(b), HostGame::PrivateGame);
}

/** Test cloneGame(), operation with target state. */
void
TestServerHostHostGame::testCloneGameStatus()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create a game and clone it
    int32_t a = testee.createNewGame();
    int32_t b = testee.cloneGame(a, HostGame::Preparing);
    TS_ASSERT_EQUALS(a, 1);
    TS_ASSERT_EQUALS(b, 2);

    // Verify
    TS_ASSERT_EQUALS(testee.getName(b), "New Game 1");
    TS_ASSERT_EQUALS(testee.getState(b), HostGame::Preparing);
    TS_ASSERT_EQUALS(testee.getType(b), HostGame::PrivateGame);
}

/** Test cloneGame(), error case.
    Users cannot clone games. */
void
TestServerHostHostGame::testCloneGameErrorUser()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create a game
    int32_t a = testee.createNewGame();

    // Set user context
    session.setUser("u");

    // Clone game. Must fail (admin-only operation).
    TS_ASSERT_THROWS(testee.cloneGame(a, afl::base::Nothing), std::exception);
}

/** Test cloneGame(), error case.
    Cloning fails if the game is locked. */
void
TestServerHostHostGame::testCloneGameErrorLocked()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create a game
    int32_t a = testee.createNewGame();

    // Lock it for hosting
    server::host::GameArbiter::Guard guard(h.root().arbiter(), a, server::host::GameArbiter::Host);

    // Clone game. Must fail.
    TS_ASSERT_THROWS(testee.cloneGame(a, afl::base::Nothing), std::exception);
}

/** Test cloneGame(), error case.
    Cloning fails if the source game does not exist. */
void
TestServerHostHostGame::testCloneGameId()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // The first game will receive the Id 1. This clone must fail (and not create game 1 and copy it onto itself).
    TS_ASSERT_THROWS(testee.cloneGame(1, afl::base::Nothing), std::exception);

    // Clone game with invented Id. Must fail.
    TS_ASSERT_THROWS(testee.cloneGame(72, afl::base::Nothing), std::exception);
}

/** Test listGame() and related functions.
    This test is similar to c2systest/host/04_listgame. */
void
TestServerHostHostGame::testListGame()
{
    TestHarness h;
    h.addDefaultTools();
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Prepare: create a bunch of games in different states
    // - 1: public/joining
    TS_ASSERT_EQUALS(testee.createNewGame(), 1);
    TS_ASSERT_THROWS_NOTHING(testee.setType(1, HostGame::PublicGame));
    TS_ASSERT_THROWS_NOTHING(testee.setState(1, HostGame::Joining));

    // - 2: unlisted/joining
    TS_ASSERT_EQUALS(testee.createNewGame(), 2);
    TS_ASSERT_THROWS_NOTHING(testee.setType(2, HostGame::UnlistedGame));
    TS_ASSERT_THROWS_NOTHING(testee.setState(2, HostGame::Joining));

    // - 3: public/preparing
    TS_ASSERT_EQUALS(testee.createNewGame(), 3);
    TS_ASSERT_THROWS_NOTHING(testee.setType(3, HostGame::PublicGame));
    TS_ASSERT_THROWS_NOTHING(testee.setState(3, HostGame::Preparing));

    //  4: private/preparing
    TS_ASSERT_EQUALS(testee.createNewGame(), 4);
    TS_ASSERT_THROWS_NOTHING(testee.setType(4, HostGame::PrivateGame));
    TS_ASSERT_THROWS_NOTHING(testee.setState(4, HostGame::Preparing));
    TS_ASSERT_THROWS_NOTHING(testee.setOwner(4, "u"));

    // Test
    // - admin
    {
        afl::data::IntegerList_t result;
        TS_ASSERT_THROWS_NOTHING(testee.getGames(afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, result));
        TS_ASSERT_EQUALS(result.size(), 4U);
        TS_ASSERT_EQUALS(result[0], 1);
        TS_ASSERT_EQUALS(result[1], 2);
        TS_ASSERT_EQUALS(result[2], 3);
        TS_ASSERT_EQUALS(result[3], 4);
    }
    {
        afl::data::IntegerList_t result;
        TS_ASSERT_THROWS_NOTHING(testee.getGames(afl::base::Nothing, HostGame::PublicGame, afl::base::Nothing, result));
        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT_EQUALS(result[0], 1);
        TS_ASSERT_EQUALS(result[1], 3);
    }
    {
        afl::data::IntegerList_t result;
        TS_ASSERT_THROWS_NOTHING(testee.getGames(HostGame::Joining, afl::base::Nothing, afl::base::Nothing, result));
        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT_EQUALS(result[0], 1);
        TS_ASSERT_EQUALS(result[1], 2);
    }
    {
        afl::data::IntegerList_t result;
        TS_ASSERT_THROWS_NOTHING(testee.getGames(HostGame::Joining, HostGame::PublicGame, afl::base::Nothing, result));
        TS_ASSERT_EQUALS(result.size(), 1U);
        TS_ASSERT_EQUALS(result[0], 1);
    }
    {
        afl::data::IntegerList_t result;
        TS_ASSERT_THROWS_NOTHING(testee.getGames(HostGame::Running, HostGame::PublicGame, afl::base::Nothing, result));
        TS_ASSERT_EQUALS(result.size(), 0U);
    }
    {
        afl::data::IntegerList_t result;
        TS_ASSERT_THROWS_NOTHING(testee.getGames(HostGame::Preparing, afl::base::Nothing, afl::base::Nothing, result));
        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT_EQUALS(result[0], 3);
        TS_ASSERT_EQUALS(result[1], 4);
    }

    // - user "u"
    {
        session.setUser("u");
        afl::data::IntegerList_t result;
        TS_ASSERT_THROWS_NOTHING(testee.getGames(HostGame::Preparing, afl::base::Nothing, afl::base::Nothing, result));
        TS_ASSERT_EQUALS(result.size(), 1U);
        TS_ASSERT_EQUALS(result[0], 4);
    }

    // - user "z"
    {
        session.setUser("z");
        afl::data::IntegerList_t result;
        TS_ASSERT_THROWS_NOTHING(testee.getGames(HostGame::Preparing, afl::base::Nothing, afl::base::Nothing, result));
        TS_ASSERT_EQUALS(result.size(), 0U);
    }

    // While we are at it, test getTotals
    HostGame::Totals t = testee.getTotals();
    TS_ASSERT_EQUALS(t.numJoiningGames, 1);         // only public!
    TS_ASSERT_EQUALS(t.numRunningGames, 0);
    TS_ASSERT_EQUALS(t.numFinishedGames, 0);

    // Likewise, test getOwner
    {
        session.setUser("z");
        TS_ASSERT_EQUALS(testee.getOwner(1), "");
        TS_ASSERT_THROWS(testee.getOwner(4), std::exception);   // not accessible to 'z', it's private!
    }
    {
        session.setUser("");
        TS_ASSERT_EQUALS(testee.getOwner(4), "u");
    }
    {
        session.setUser("u");
        TS_ASSERT_EQUALS(testee.getOwner(4), "u");
    }
}

/** Test getGameInfo() and related. */
void
TestServerHostHostGame::testGameInfo()
{
    TestHarness h;
    h.addDefaultTools();
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Prepare: create two games
    TS_ASSERT_EQUALS(testee.createNewGame(), 1);
    TS_ASSERT_THROWS_NOTHING(testee.setType(1, HostGame::PublicGame));
    TS_ASSERT_THROWS_NOTHING(testee.setState(1, HostGame::Joining));
    TS_ASSERT_THROWS_NOTHING(testee.setName(1, "One"));

    TS_ASSERT_EQUALS(testee.createNewGame(), 2);
    TS_ASSERT_THROWS_NOTHING(testee.setType(2, HostGame::PublicGame));
    TS_ASSERT_THROWS_NOTHING(testee.setState(2, HostGame::Joining));
    TS_ASSERT_THROWS_NOTHING(testee.setName(2, "Two"));

    // Query single game
    {
        HostGame::Info i = testee.getInfo(2);
        TS_ASSERT_EQUALS(i.gameId, 2);
        TS_ASSERT_EQUALS(i.state, HostGame::Joining);
        TS_ASSERT_EQUALS(i.type, HostGame::PublicGame);
        TS_ASSERT_EQUALS(i.name, "Two");
    }

    // Query list
    {
        std::vector<HostGame::Info> result;
        TS_ASSERT_THROWS_NOTHING(testee.getInfos(HostGame::Joining, afl::base::Nothing, afl::base::Nothing, false, result));
        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT_EQUALS(result[0].gameId, 1);
        TS_ASSERT_EQUALS(result[0].name, "One");
        TS_ASSERT_EQUALS(result[1].gameId, 2);
        TS_ASSERT_EQUALS(result[1].name, "Two");
    }

    // Query list, no match
    {
        std::vector<HostGame::Info> result;
        TS_ASSERT_THROWS_NOTHING(testee.getInfos(HostGame::Running, afl::base::Nothing, afl::base::Nothing, false, result));
        TS_ASSERT_EQUALS(result.size(), 0U);
    }

    // Query single, error case
    {
        TS_ASSERT_THROWS(testee.getInfo(3), std::exception);
    }
}

/** Test setConfig, simple. */
void
TestServerHostHostGame::testSetConfigSimple()
{
    TestHarness h;
    h.addDefaultTools();
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create game
    TS_ASSERT_EQUALS(testee.createNewGame(), 1);

    // Set config
    afl::data::StringList_t s;
    s.push_back("description");
    s.push_back("The Game");
    s.push_back("rankDisable");
    s.push_back("1");
    TS_ASSERT_THROWS_NOTHING(testee.setConfig(1, s));

    // Verify
    TS_ASSERT_EQUALS(HashKey(h.db(), "game:1:settings").stringField("description").get(), "The Game");
    TS_ASSERT_EQUALS(HashKey(h.db(), "game:1:settings").intField("rankDisable").get(), 1);

    // Read back
    TS_ASSERT_EQUALS(testee.getConfig(1, "description"), "The Game");
    TS_ASSERT_EQUALS(testee.getConfig(1, "rankDisable"), "1");

    // Read back, complex
    {
        afl::data::StringList_t in;
        in.push_back("rankDisable");
        in.push_back("endChanged");
        in.push_back("description");

        afl::data::StringList_t out;
        TS_ASSERT_THROWS_NOTHING(testee.getConfig(1, in, out));

        TS_ASSERT_EQUALS(out.size(), 3U);
        TS_ASSERT_EQUALS(out[0], "1");
        TS_ASSERT_EQUALS(out[1], "");
        TS_ASSERT_EQUALS(out[2], "The Game");
    }
}

/** Test setConfig() for tool config.
    Must implicitly set the configChanged flag. */
void
TestServerHostHostGame::testSetConfigTool()
{
    TestHarness h;
    h.addDefaultTools();
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create game
    TS_ASSERT_EQUALS(testee.createNewGame(), 1);
    TS_ASSERT_EQUALS(testee.getConfig(1, "host"), "H");

    // Set config
    afl::data::StringList_t s;
    s.push_back("host");
    s.push_back("P");
    TS_ASSERT_THROWS_NOTHING(testee.setConfig(1, s));

    // Read back
    TS_ASSERT_EQUALS(testee.getConfig(1, "host"), "P");
    TS_ASSERT_EQUALS(testee.getConfig(1, "configChanged"), "1");
}

/** Test setConfig() with bad tool config.
    Must fail the setting completely. */
void
TestServerHostHostGame::testSetConfigToolError()
{
    TestHarness h;
    h.addDefaultTools();
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create game
    TS_ASSERT_EQUALS(testee.createNewGame(), 1);

    // Set config
    afl::data::StringList_t s;
    s.push_back("rankDisable");
    s.push_back("1");
    s.push_back("host");
    s.push_back("zzz");
    TS_ASSERT_THROWS(testee.setConfig(1, s), std::exception);

    // Read back
    TS_ASSERT_EQUALS(testee.getConfig(1, "host"), "H");
    TS_ASSERT_EQUALS(testee.getConfig(1, "rankDisable"), "");
}

/** Test setConfig() with end config.
    Must set the endChanged flag. */
void
TestServerHostHostGame::testSetConfigEnd()
{
    TestHarness h;
    h.addDefaultTools();
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create game
    TS_ASSERT_EQUALS(testee.createNewGame(), 1);

    // Set config
    afl::data::StringList_t s;
    s.push_back("endCondition");
    s.push_back("turn");
    s.push_back("endTurn");
    s.push_back("80");
    TS_ASSERT_THROWS_NOTHING(testee.setConfig(1, s));

    // Read back
    TS_ASSERT_EQUALS(testee.getConfig(1, "endCondition"), "turn");
    TS_ASSERT_EQUALS(testee.getConfig(1, "endTurn"), "80");
    TS_ASSERT_EQUALS(testee.getConfig(1, "endChanged"), "1");
}

/** Test setConfig() with end config and endChanged flag.
    Must NOT set the endChanged flag because it was specified in the transaction. */
void
TestServerHostHostGame::testSetConfigEndHide()
{
    TestHarness h;
    h.addDefaultTools();
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create game
    TS_ASSERT_EQUALS(testee.createNewGame(), 1);

    // Set config
    afl::data::StringList_t s;
    s.push_back("endCondition");
    s.push_back("turn");
    s.push_back("endChanged");
    s.push_back("0");
    s.push_back("endTurn");
    s.push_back("80");
    TS_ASSERT_THROWS_NOTHING(testee.setConfig(1, s));

    // Read back
    TS_ASSERT_EQUALS(testee.getConfig(1, "endCondition"), "turn");
    TS_ASSERT_EQUALS(testee.getConfig(1, "endTurn"), "80");
    TS_ASSERT_EQUALS(testee.getConfig(1, "endChanged"), "0");
}

/** Test addTool/removeTool/getTools. */
void
TestServerHostHostGame::testTools()
{
    TestHarness h;
    h.addDefaultTools();
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Add some tools
    HashKey(h.db(), "prog:tool:prog:x1").stringField("kind").set("xk");
    HashKey(h.db(), "prog:tool:prog:x1").stringField("description").set("text one");
    HashKey(h.db(), "prog:tool:prog:x2").stringField("kind").set("xk");
    HashKey(h.db(), "prog:tool:prog:x2").stringField("description").set("text two");
    HashKey(h.db(), "prog:tool:prog:y").stringField("kind").set("yk");
    HashKey(h.db(), "prog:tool:prog:y").stringField("description").set("text three");
    StringSetKey(h.db(), "prog:tool:list").add("x1");
    StringSetKey(h.db(), "prog:tool:list").add("x2");
    StringSetKey(h.db(), "prog:tool:list").add("y");

    // Create a game
    int32_t gid = testee.createNewGame();
    TS_ASSERT_EQUALS(gid, 1);

    // List tools; must be none
    {
        std::vector<HostTool::Info> result;
        TS_ASSERT_THROWS_NOTHING(testee.getTools(gid, result));
        TS_ASSERT_EQUALS(result.size(), 0U);
    }

    // Add tools
    TS_ASSERT_EQUALS(testee.addTool(gid, "x1"), true);
    TS_ASSERT_EQUALS(testee.addTool(gid, "y"),  true);

    // List tools; must be both
    {
        std::vector<HostTool::Info> result;
        TS_ASSERT_THROWS_NOTHING(testee.getTools(gid, result));
        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT_EQUALS(result[0].id, "x1");
        TS_ASSERT_EQUALS(result[0].description, "text one");
        TS_ASSERT_EQUALS(result[0].kind, "xk");
        TS_ASSERT_EQUALS(result[1].id, "y");
        TS_ASSERT_EQUALS(result[1].description, "text three");
        TS_ASSERT_EQUALS(result[1].kind, "yk");
    }

    // Add tool x2; replaces x1
    TS_ASSERT_EQUALS(testee.addTool(gid, "x2"), true);

    // List tools; must be x2 and y
    {
        std::vector<HostTool::Info> result;
        TS_ASSERT_THROWS_NOTHING(testee.getTools(gid, result));
        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT_EQUALS(result[0].id, "x2");
        TS_ASSERT_EQUALS(result[0].description, "text two");
        TS_ASSERT_EQUALS(result[0].kind, "xk");
        TS_ASSERT_EQUALS(result[1].id, "y");
    }

    // Remove y
    TS_ASSERT_EQUALS(testee.removeTool(gid, "y"), true);
    {
        std::vector<HostTool::Info> result;
        TS_ASSERT_THROWS_NOTHING(testee.getTools(gid, result));
        TS_ASSERT_EQUALS(result.size(), 1U);
        TS_ASSERT_EQUALS(result[0].id, "x2");
    }

    // Remove non-present
    TS_ASSERT_EQUALS(testee.removeTool(gid, "y"),  false);

    // Remove non-existant
    TS_ASSERT_THROWS(testee.removeTool(gid, "qq"), std::exception);

    // Add already present
    TS_ASSERT_EQUALS(testee.addTool(gid, "x2"), false);

    // Add non-existant
    TS_ASSERT_THROWS(testee.addTool(gid, "q"), std::exception);
}

/** Test update(), admin.
    The command doesn't do anything particular interesting, just verify that it's accepted. */
void
TestServerHostHostGame::testUpdateAdmin()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    int32_t gid = testee.createNewGame();

    {
        afl::data::IntegerList_t in;
        in.push_back(gid);
        TS_ASSERT_THROWS_NOTHING(testee.updateGames(in));
    }
    {
        afl::data::IntegerList_t in;
        in.push_back(99999);
        TS_ASSERT_THROWS(testee.updateGames(in), std::exception);
    }
}

/** Test update(), user. */
void
TestServerHostHostGame::testUpdateUser()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());
    session.setUser("x");

    int32_t gid = testee.createNewGame();

    {
        afl::data::IntegerList_t in;
        in.push_back(gid);
        TS_ASSERT_THROWS(testee.updateGames(in), std::exception);
    }
}

/** Test getPermissions(). */
void
TestServerHostHostGame::testGetPermissions()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create a game
    int32_t gid = h.addGame(testee);
    TS_ASSERT_EQUALS(gid, 1);

    // Verify
    TS_ASSERT_EQUALS(testee.getPermissions(gid, "a"), HostGame::Permissions_t() + HostGame::UserIsPrimary + HostGame::UserIsActive);
    TS_ASSERT_EQUALS(testee.getPermissions(gid, "b"), HostGame::Permissions_t() + HostGame::UserIsPrimary);
    TS_ASSERT_EQUALS(testee.getPermissions(gid, "c"), HostGame::Permissions_t() + HostGame::UserIsActive);
    TS_ASSERT_EQUALS(testee.getPermissions(gid, "d"), HostGame::Permissions_t() + HostGame::UserIsPrimary);
    TS_ASSERT_EQUALS(testee.getPermissions(gid, "e"), HostGame::Permissions_t() + HostGame::UserIsInactive);
    TS_ASSERT_EQUALS(testee.getPermissions(gid, "f"), HostGame::Permissions_t() + HostGame::UserIsActive);
    TS_ASSERT_EQUALS(testee.getPermissions(gid, "x"), HostGame::Permissions_t() + HostGame::GameIsPublic);
    TS_ASSERT_EQUALS(testee.getPermissions(gid, "z"), HostGame::Permissions_t() + HostGame::UserIsOwner);

    // Combinations
    server::host::Game(h.root(), gid).pushPlayerSlot(4, "f", h.root());
    server::host::Game(h.root(), gid).pushPlayerSlot(5, "z", h.root());

    TS_ASSERT_EQUALS(testee.getPermissions(gid, "f"), HostGame::Permissions_t() + HostGame::UserIsActive + HostGame::UserIsPrimary);
    TS_ASSERT_EQUALS(testee.getPermissions(gid, "z"), HostGame::Permissions_t() + HostGame::UserIsActive + HostGame::UserIsPrimary + HostGame::UserIsOwner);
}

/** Test getVictoryCondition. */
void
TestServerHostHostGame::testVictoryCondition()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create a game
    int32_t gid = testee.createNewGame();
    TS_ASSERT_EQUALS(gid, 1);

    // Set config
    afl::data::StringList_t s;
    s.push_back("endCondition");
    s.push_back("turn");
    s.push_back("endTurn");
    s.push_back("50");
    s.push_back("endProbability");
    s.push_back("3");
    TS_ASSERT_THROWS_NOTHING(testee.setConfig(1, s));

    // Verify
    HostGame::VictoryCondition vc = testee.getVictoryCondition(1);
    TS_ASSERT_EQUALS(vc.endCondition, "turn");
    TS_ASSERT(vc.endTurn.isSame(50));
    TS_ASSERT(vc.endProbability.isSame(3));
}

/** Test getGames() with user filters. */
void
TestServerHostHostGame::testListUserGames()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create a game
    int32_t gid = h.addGame(testee);
    TS_ASSERT_EQUALS(gid, 1);

    // User a: must list game
    {
        afl::data::IntegerList_t result;
        testee.getGames(afl::base::Nothing, afl::base::Nothing, String_t("a"), result);
        TS_ASSERT_EQUALS(result.size(), 1U);
        TS_ASSERT_EQUALS(result[0], gid);
    }

    // User b: must list game
    {
        afl::data::IntegerList_t result;
        testee.getGames(afl::base::Nothing, afl::base::Nothing, String_t("b"), result);
        TS_ASSERT_EQUALS(result.size(), 1U);
        TS_ASSERT_EQUALS(result[0], gid);
    }

    // User c: must list game
    {
        afl::data::IntegerList_t result;
        testee.getGames(afl::base::Nothing, afl::base::Nothing, String_t("c"), result);
        TS_ASSERT_EQUALS(result.size(), 1U);
        TS_ASSERT_EQUALS(result[0], gid);
    }

    // User z: must NOT list game (owner, but not player)
    {
        afl::data::IntegerList_t result;
        testee.getGames(afl::base::Nothing, afl::base::Nothing, String_t("z"), result);
        TS_ASSERT_EQUALS(result.size(), 0U);
    }

    // User a with matching filter: must list game
    {
        afl::data::IntegerList_t result;
        testee.getGames(HostGame::Joining, HostGame::PublicGame, String_t("a"), result);
        TS_ASSERT_EQUALS(result.size(), 1U);
        TS_ASSERT_EQUALS(result[0], gid);
    }

    // User a with mismatching filter: must NOT list game
    {
        afl::data::IntegerList_t result;
        testee.getGames(HostGame::Running, HostGame::PublicGame, String_t("a"), result);
        TS_ASSERT_EQUALS(result.size(), 0U);
    }

    // User a with mismatching filter: must NOT list game
    {
        afl::data::IntegerList_t result;
        testee.getGames(HostGame::Running, afl::base::Nothing, String_t("a"), result);
        TS_ASSERT_EQUALS(result.size(), 0U);
    }
}

