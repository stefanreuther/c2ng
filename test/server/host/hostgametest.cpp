/**
  *  \file test/server/host/hostgametest.cpp
  *  \brief Test for server::host::HostGame
  */

#include "server/host/hostgame.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/test/testrunner.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/host/game.hpp"
#include "server/host/gamearbiter.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "util/processrunner.hpp"

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
    StringSetKey(m_db, "prog:sl:list").add("S");
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
AFL_TEST("server.host.HostGame:createNewGame", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Creating two games must create distinct Ids
    int32_t aa = testee.createNewGame();
    int32_t bb = testee.createNewGame();
    a.checkEqual("01. createNewGame", aa, 1);
    a.checkEqual("02. createNewGame", bb, 2);

    // Name and type
    a.checkEqual("11. getName",      testee.getName(aa), "New Game");
    a.checkEqual("12. getState",     testee.getState(aa), HostGame::Preparing);
    a.checkEqual("13. getType",      testee.getType(aa), HostGame::PrivateGame);
    a.checkEqual("14. getDirectory", testee.getDirectory(aa), "games/0001");

    // Stats
    HostGame::Totals t = testee.getTotals();
    a.checkEqual("21. numJoiningGames",  t.numJoiningGames, 0);
    a.checkEqual("22. numRunningGames",  t.numRunningGames, 0);
    a.checkEqual("23. numFinishedGames", t.numFinishedGames, 0);
}

/** Test cloneGame(), standard case.
    Tests just basic operation.
    Actual game creation is tested separately.
    A complete test is in c2systest/host/02_newgame. */
AFL_TEST("server.host.HostGame:cloneGame", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create a game and clone it
    int32_t aa = testee.createNewGame();
    int32_t bb = testee.cloneGame(aa, afl::base::Nothing);
    a.checkEqual("01. createNewGame", aa, 1);
    a.checkEqual("02. cloneGame", bb, 2);

    // Verify
    a.checkEqual("11. getName",  testee.getName(bb), "New Game 1");
    a.checkEqual("12. getState", testee.getState(bb), HostGame::Joining);
    a.checkEqual("13. getType",  testee.getType(bb), HostGame::PrivateGame);

    // Verify listability
    afl::data::IntegerList_t list;
    server::interface::HostGame::Filter filter;
    filter.requiredCopyOf = aa;
    testee.getGames(filter, list);
    a.checkEqual("21. size", list.size(), 1U);
    a.checkEqual("22. list", list[0], bb);
}

/** Test cloneGame(), operation with target state. */
AFL_TEST("server.host.HostGame:cloneGame:status", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create a game and clone it
    int32_t aa = testee.createNewGame();
    int32_t bb = testee.cloneGame(aa, HostGame::Preparing);
    a.checkEqual("01. createNewGame", aa, 1);
    a.checkEqual("02. cloneGame", bb, 2);

    // Verify
    a.checkEqual("11. getName",  testee.getName(bb), "New Game 1");
    a.checkEqual("12. getState", testee.getState(bb), HostGame::Preparing);
    a.checkEqual("13. getType",  testee.getType(bb), HostGame::PrivateGame);
}

/** Test cloneGame(), error case.
    Users cannot clone games. */
AFL_TEST("server.host.HostGame:cloneGame:error:user", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create a game
    int32_t aa = testee.createNewGame();

    // Set user context
    session.setUser("u");

    // Clone game. Must fail (admin-only operation).
    AFL_CHECK_THROWS(a, testee.cloneGame(aa, afl::base::Nothing), std::exception);
}

/** Test cloneGame(), error case.
    Cloning fails if the game is locked. */
AFL_TEST("server.host.HostGame:cloneGame:error:locked", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create a game
    int32_t aa = testee.createNewGame();

    // Lock it for hosting
    server::host::GameArbiter::Guard guard(h.root().arbiter(), aa, server::host::GameArbiter::Host);

    // Clone game. Must fail.
    AFL_CHECK_THROWS(a, testee.cloneGame(aa, afl::base::Nothing), std::exception);
}

/** Test cloneGame(), error case.
    Cloning fails if the source game does not exist. */
AFL_TEST("server.host.HostGame:cloneGame:error:bad-id", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // The first game will receive the Id 1. This clone must fail (and not create game 1 and copy it onto itself).
    AFL_CHECK_THROWS(a("clone 1"), testee.cloneGame(1, afl::base::Nothing), std::exception);

    // Clone game with invented Id. Must fail.
    AFL_CHECK_THROWS(a("clone 72"), testee.cloneGame(72, afl::base::Nothing), std::exception);
}

/** Test listGame() and related functions.
    This test is similar to c2systest/host/04_listgame. */
AFL_TEST("server.host.HostGame:getGames", a)
{
    TestHarness h;
    h.addDefaultTools();
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Prepare: create a bunch of games in different states
    // - 1: public/joining
    a.checkEqual("01. createNewGame", testee.createNewGame(), 1);
    AFL_CHECK_SUCCEEDS(a("02. setType"), testee.setType(1, HostGame::PublicGame));
    AFL_CHECK_SUCCEEDS(a("03. setState"), testee.setState(1, HostGame::Joining));

    // - 2: unlisted/joining
    a.checkEqual("11. createNewGame", testee.createNewGame(), 2);
    AFL_CHECK_SUCCEEDS(a("12. setType"), testee.setType(2, HostGame::UnlistedGame));
    AFL_CHECK_SUCCEEDS(a("13. setState"), testee.setState(2, HostGame::Joining));

    // - 3: public/preparing
    a.checkEqual("21. createNewGame", testee.createNewGame(), 3);
    AFL_CHECK_SUCCEEDS(a("22. setType"), testee.setType(3, HostGame::PublicGame));
    AFL_CHECK_SUCCEEDS(a("23. setState"), testee.setState(3, HostGame::Preparing));

    //  4: private/preparing
    a.checkEqual("31. createNewGame", testee.createNewGame(), 4);
    AFL_CHECK_SUCCEEDS(a("32. setType"), testee.setType(4, HostGame::PrivateGame));
    AFL_CHECK_SUCCEEDS(a("33. setState"), testee.setState(4, HostGame::Preparing));
    AFL_CHECK_SUCCEEDS(a("34. setOwner"), testee.setOwner(4, "u"));

    // Test
    // - admin
    {
        afl::data::IntegerList_t result;
        AFL_CHECK_SUCCEEDS(a("41. getGames"), testee.getGames(HostGame::Filter(), result));
        a.checkEqual("42. size", result.size(), 4U);
        a.checkEqual("43. result", result[0], 1);
        a.checkEqual("44. result", result[1], 2);
        a.checkEqual("45. result", result[2], 3);
        a.checkEqual("46. result", result[3], 4);
    }
    {
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredType = HostGame::PublicGame;
        AFL_CHECK_SUCCEEDS(a("47. getGames"), testee.getGames(filter, result));
        a.checkEqual("48. size", result.size(), 2U);
        a.checkEqual("49. result", result[0], 1);
        a.checkEqual("50. result", result[1], 3);
    }
    {
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredState = HostGame::Joining;
        AFL_CHECK_SUCCEEDS(a("51. getGames"), testee.getGames(filter, result));
        a.checkEqual("52. size", result.size(), 2U);
        a.checkEqual("53. result", result[0], 1);
        a.checkEqual("54. result", result[1], 2);
    }
    {
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredState = HostGame::Joining;
        filter.requiredType = HostGame::PublicGame;
        AFL_CHECK_SUCCEEDS(a("55. getGames"), testee.getGames(filter, result));
        a.checkEqual("56. size", result.size(), 1U);
        a.checkEqual("57. result", result[0], 1);
    }
    {
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredState = HostGame::Running;
        filter.requiredType = HostGame::PublicGame;
        AFL_CHECK_SUCCEEDS(a("58. getGames"), testee.getGames(filter, result));
        a.checkEqual("59. size", result.size(), 0U);
    }
    {
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredState = HostGame::Preparing;
        AFL_CHECK_SUCCEEDS(a("60. getGames"), testee.getGames(filter, result));
        a.checkEqual("61. size", result.size(), 2U);
        a.checkEqual("62. result", result[0], 3);
        a.checkEqual("63. result", result[1], 4);
    }

    // - user "u"
    {
        session.setUser("u");
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredState = HostGame::Preparing;
        AFL_CHECK_SUCCEEDS(a("71. getGames"), testee.getGames(filter, result));
        a.checkEqual("72. size", result.size(), 1U);
        a.checkEqual("73. result", result[0], 4);
    }

    // - user "z"
    {
        session.setUser("z");
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredState = HostGame::Preparing;
        AFL_CHECK_SUCCEEDS(a("81. getGames"), testee.getGames(filter, result));
        a.checkEqual("82. size", result.size(), 0U);
    }

    // While we are at it, test getTotals
    HostGame::Totals t = testee.getTotals();
    a.checkEqual("91. numJoiningGames",  t.numJoiningGames, 1);         // only public!
    a.checkEqual("92. numRunningGames",  t.numRunningGames, 0);
    a.checkEqual("93. numFinishedGames", t.numFinishedGames, 0);

    // Likewise, test getOwner
    {
        session.setUser("z");
        a.checkEqual("101. getOwner", testee.getOwner(1), "");
        AFL_CHECK_THROWS(a("102. getOwner"), testee.getOwner(4), std::exception);   // not accessible to 'z', it's private!
    }
    {
        session.setUser("");
        a.checkEqual("103. getOwner", testee.getOwner(4), "u");
    }
    {
        session.setUser("u");
        a.checkEqual("104. getOwner", testee.getOwner(4), "u");
    }
}

/** Test getGameInfo() and related. */
AFL_TEST("server.host.HostGame:getInfo", a)
{
    TestHarness h;
    h.addDefaultTools();
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Prepare: create two games
    a.checkEqual("01. createNewGame", testee.createNewGame(), 1);
    AFL_CHECK_SUCCEEDS(a("02. setType"), testee.setType(1, HostGame::PublicGame));
    AFL_CHECK_SUCCEEDS(a("03. setState"), testee.setState(1, HostGame::Joining));
    AFL_CHECK_SUCCEEDS(a("04. setName"), testee.setName(1, "One"));

    a.checkEqual("11. createNewGame", testee.createNewGame(), 2);
    AFL_CHECK_SUCCEEDS(a("12. setType"), testee.setType(2, HostGame::PublicGame));
    AFL_CHECK_SUCCEEDS(a("13. setState"), testee.setState(2, HostGame::Joining));
    AFL_CHECK_SUCCEEDS(a("14. setName"), testee.setName(2, "Two"));

    // Query single game
    {
        HostGame::Info i = testee.getInfo(2);
        a.checkEqual("21. gameId", i.gameId, 2);
        a.checkEqual("22. state", i.state, HostGame::Joining);
        a.checkEqual("23. type", i.type, HostGame::PublicGame);
        a.checkEqual("24. name", i.name, "Two");
    }

    // Query list
    {
        std::vector<HostGame::Info> result;
        HostGame::Filter filter;
        filter.requiredState = HostGame::Joining;
        AFL_CHECK_SUCCEEDS(a("31. getInfos"), testee.getInfos(filter, false, result));
        a.checkEqual("32. size", result.size(), 2U);
        a.checkEqual("33. result", result[0].gameId, 1);
        a.checkEqual("34. result", result[0].name, "One");
        a.checkEqual("35. result", result[1].gameId, 2);
        a.checkEqual("36. result", result[1].name, "Two");
    }

    // Query list, no match
    {
        std::vector<HostGame::Info> result;
        HostGame::Filter filter;
        filter.requiredState = HostGame::Running;
        AFL_CHECK_SUCCEEDS(a("41. getInfos"), testee.getInfos(filter, false, result));
        a.checkEqual("42. size", result.size(), 0U);
    }

    // Query single, error case
    {
        AFL_CHECK_THROWS(a("51. getInfo"), testee.getInfo(3), std::exception);
    }
}

/** Test setConfig, simple. */
AFL_TEST("server.host.HostGame:setConfig", a)
{
    TestHarness h;
    h.addDefaultTools();
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create game
    a.checkEqual("01. createNewGame", testee.createNewGame(), 1);

    // Set config
    afl::data::StringList_t s;
    s.push_back("description");
    s.push_back("The Game");
    s.push_back("rankDisable");
    s.push_back("1");
    AFL_CHECK_SUCCEEDS(a("11. setConfig"), testee.setConfig(1, s));

    // Verify
    a.checkEqual("21. db", HashKey(h.db(), "game:1:settings").stringField("description").get(), "The Game");
    a.checkEqual("22. db", HashKey(h.db(), "game:1:settings").intField("rankDisable").get(), 1);

    // Read back
    a.checkEqual("31. getConfig", testee.getConfig(1, "description"), "The Game");
    a.checkEqual("32. getConfig", testee.getConfig(1, "rankDisable"), "1");

    // Read back, complex
    {
        afl::data::StringList_t in;
        in.push_back("rankDisable");
        in.push_back("endChanged");
        in.push_back("description");

        afl::data::StringList_t out;
        AFL_CHECK_SUCCEEDS(a("41. getConfig"), testee.getConfig(1, in, out));

        a.checkEqual("51. size", out.size(), 3U);
        a.checkEqual("52. result", out[0], "1");
        a.checkEqual("53. result", out[1], "");
        a.checkEqual("54. result", out[2], "The Game");
    }
}

/** Test setConfig() for tool config.
    Must implicitly set the configChanged flag. */
AFL_TEST("server.host.HostGame:setConfig:tool", a)
{
    TestHarness h;
    h.addDefaultTools();
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create game
    a.checkEqual("01. createNewGame", testee.createNewGame(), 1);
    a.checkEqual("02. getConfig", testee.getConfig(1, "host"), "H");

    // Set config
    afl::data::StringList_t s;
    s.push_back("host");
    s.push_back("P");
    AFL_CHECK_SUCCEEDS(a("11. setConfig"), testee.setConfig(1, s));

    // Read back
    a.checkEqual("21. getConfig", testee.getConfig(1, "host"), "P");
    a.checkEqual("22. getConfig", testee.getConfig(1, "configChanged"), "1");
}

/** Test setConfig() with bad tool config.
    Must fail the setting completely. */
AFL_TEST("server.host.HostGame:setConfig:tool:error", a)
{
    TestHarness h;
    h.addDefaultTools();
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create game
    a.checkEqual("01. createNewGame", testee.createNewGame(), 1);

    // Set config
    afl::data::StringList_t s;
    s.push_back("rankDisable");
    s.push_back("1");
    s.push_back("host");
    s.push_back("zzz");
    AFL_CHECK_THROWS(a("11. setConfig"), testee.setConfig(1, s), std::exception);

    // Read back
    a.checkEqual("21. getConfig", testee.getConfig(1, "host"), "H");
    a.checkEqual("22. getConfig", testee.getConfig(1, "rankDisable"), "");
}

/** Test setConfig() with end config.
    Must set the endChanged flag. */
AFL_TEST("server.host.HostGame:setConfig:end", a)
{
    TestHarness h;
    h.addDefaultTools();
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create game
    a.checkEqual("01. createNewGame", testee.createNewGame(), 1);

    // Set config
    afl::data::StringList_t s;
    s.push_back("endCondition");
    s.push_back("turn");
    s.push_back("endTurn");
    s.push_back("80");
    AFL_CHECK_SUCCEEDS(a("11. setConfig"), testee.setConfig(1, s));

    // Read back
    a.checkEqual("21. getConfig", testee.getConfig(1, "endCondition"), "turn");
    a.checkEqual("22. getConfig", testee.getConfig(1, "endTurn"), "80");
    a.checkEqual("23. getConfig", testee.getConfig(1, "endChanged"), "1");
}

/** Test setConfig() with end config and endChanged flag.
    Must NOT set the endChanged flag because it was specified in the transaction. */
AFL_TEST("server.host.HostGame:setConfig:end:hidden", a)
{
    TestHarness h;
    h.addDefaultTools();
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create game
    a.checkEqual("01. createNewGame", testee.createNewGame(), 1);

    // Set config
    afl::data::StringList_t s;
    s.push_back("endCondition");
    s.push_back("turn");
    s.push_back("endChanged");
    s.push_back("0");
    s.push_back("endTurn");
    s.push_back("80");
    AFL_CHECK_SUCCEEDS(a("11. setConfig"), testee.setConfig(1, s));

    // Read back
    a.checkEqual("21. getConfig", testee.getConfig(1, "endCondition"), "turn");
    a.checkEqual("22. getConfig", testee.getConfig(1, "endTurn"), "80");
    a.checkEqual("23. getConfig", testee.getConfig(1, "endChanged"), "0");
}

/** Test addTool/removeTool/getTools. */
AFL_TEST("server.host.HostGame:tools", a)
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
    a.checkEqual("01. createNewGame", gid, 1);

    // List tools; must be none
    {
        std::vector<HostTool::Info> result;
        AFL_CHECK_SUCCEEDS(a("11. getTools"), testee.getTools(gid, result));
        a.checkEqual("12. size", result.size(), 0U);
    }

    // Add tools
    a.checkEqual("21. addTool", testee.addTool(gid, "x1"), true);
    a.checkEqual("22. addTool", testee.addTool(gid, "y"),  true);

    // List tools; must be both
    {
        std::vector<HostTool::Info> result;
        AFL_CHECK_SUCCEEDS(a("31. getTools"), testee.getTools(gid, result));
        a.checkEqual("32. size", result.size(), 2U);
        a.checkEqual("33. result", result[0].id, "x1");
        a.checkEqual("34. result", result[0].description, "text one");
        a.checkEqual("35. result", result[0].kind, "xk");
        a.checkEqual("36. result", result[1].id, "y");
        a.checkEqual("37. result", result[1].description, "text three");
        a.checkEqual("38. result", result[1].kind, "yk");
    }

    // Add tool x2; replaces x1
    a.checkEqual("41. addTool", testee.addTool(gid, "x2"), true);

    // List tools; must be x2 and y
    {
        std::vector<HostTool::Info> result;
        AFL_CHECK_SUCCEEDS(a("51. getTools"), testee.getTools(gid, result));
        a.checkEqual("52. size", result.size(), 2U);
        a.checkEqual("53. result", result[0].id, "x2");
        a.checkEqual("54. result", result[0].description, "text two");
        a.checkEqual("55. result", result[0].kind, "xk");
        a.checkEqual("56. result", result[1].id, "y");
    }

    // Remove y
    a.checkEqual("61. removeTool", testee.removeTool(gid, "y"), true);
    {
        std::vector<HostTool::Info> result;
        AFL_CHECK_SUCCEEDS(a("62. getTools"), testee.getTools(gid, result));
        a.checkEqual("63. size", result.size(), 1U);
        a.checkEqual("64. result", result[0].id, "x2");
    }

    // Remove non-present
    a.checkEqual("71. removeTool", testee.removeTool(gid, "y"),  false);

    // Remove non-existant
    AFL_CHECK_THROWS(a("81. removeTool"), testee.removeTool(gid, "qq"), std::exception);

    // Add already present
    a.checkEqual("91. addTool", testee.addTool(gid, "x2"), false);

    // Add non-existant
    AFL_CHECK_THROWS(a("101. addTool"), testee.addTool(gid, "q"), std::exception);
}

/** Test update(), admin.
    The command doesn't do anything particular interesting, just verify that it's accepted. */
AFL_TEST("server.host.HostGame:updateGames", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    int32_t gid = testee.createNewGame();

    {
        afl::data::IntegerList_t in;
        in.push_back(gid);
        AFL_CHECK_SUCCEEDS(a("01. updateGames"), testee.updateGames(in));
    }
    {
        afl::data::IntegerList_t in;
        in.push_back(99999);
        AFL_CHECK_THROWS(a("02. updateGames"), testee.updateGames(in), std::exception);
    }
}

/** Test update(), user. */
AFL_TEST("server.host.HostGame:updateGames:user", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());
    session.setUser("x");

    int32_t gid = testee.createNewGame();

    {
        afl::data::IntegerList_t in;
        in.push_back(gid);
        AFL_CHECK_THROWS(a, testee.updateGames(in), std::exception);
    }
}

/** Test getPermissions(). */
AFL_TEST("server.host.HostGame:getPermissions", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create a game
    int32_t gid = h.addGame(testee);
    a.checkEqual("01. addGame", gid, 1);

    // Verify
    a.checkEqual("11", testee.getPermissions(gid, "a"), HostGame::Permissions_t() + HostGame::UserIsPrimary + HostGame::UserIsActive);
    a.checkEqual("12", testee.getPermissions(gid, "b"), HostGame::Permissions_t() + HostGame::UserIsPrimary);
    a.checkEqual("13", testee.getPermissions(gid, "c"), HostGame::Permissions_t() + HostGame::UserIsActive);
    a.checkEqual("14", testee.getPermissions(gid, "d"), HostGame::Permissions_t() + HostGame::UserIsPrimary);
    a.checkEqual("15", testee.getPermissions(gid, "e"), HostGame::Permissions_t() + HostGame::UserIsInactive);
    a.checkEqual("16", testee.getPermissions(gid, "f"), HostGame::Permissions_t() + HostGame::UserIsActive);
    a.checkEqual("17", testee.getPermissions(gid, "x"), HostGame::Permissions_t() + HostGame::GameIsPublic);
    a.checkEqual("18", testee.getPermissions(gid, "z"), HostGame::Permissions_t() + HostGame::UserIsOwner);

    // Combinations
    server::host::Game(h.root(), gid).pushPlayerSlot(4, "f", h.root());
    server::host::Game(h.root(), gid).pushPlayerSlot(5, "z", h.root());

    a.checkEqual("21", testee.getPermissions(gid, "f"), HostGame::Permissions_t() + HostGame::UserIsActive + HostGame::UserIsPrimary);
    a.checkEqual("22", testee.getPermissions(gid, "z"), HostGame::Permissions_t() + HostGame::UserIsActive + HostGame::UserIsPrimary + HostGame::UserIsOwner);
}

/** Test getVictoryCondition. */
AFL_TEST("server.host.HostGame:getVictoryCondition", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create a game
    int32_t gid = testee.createNewGame();
    a.checkEqual("01. createNewGame", gid, 1);

    // Set config
    afl::data::StringList_t s;
    s.push_back("endCondition");
    s.push_back("turn");
    s.push_back("endTurn");
    s.push_back("50");
    s.push_back("endProbability");
    s.push_back("3");
    AFL_CHECK_SUCCEEDS(a("11. setConfig"), testee.setConfig(1, s));

    // Verify
    HostGame::VictoryCondition vc = testee.getVictoryCondition(1);
    a.checkEqual("21. endCondition", vc.endCondition, "turn");
    a.check("22. endTurn",           vc.endTurn.isSame(50));
    a.check("23. endProbability",    vc.endProbability.isSame(3));
}

/** Test getGames() with user filters. */
AFL_TEST("server.host.HostGame:getGames:user", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create a game
    int32_t gid = h.addGame(testee);
    a.checkEqual("01. addGame", gid, 1);

    // User a: must list game
    {
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredUser = String_t("a");
        AFL_CHECK_SUCCEEDS(a("11. getGames"), testee.getGames(filter, result));
        a.checkEqual("12. size", result.size(), 1U);
        a.checkEqual("13. result", result[0], gid);
    }

    // User b: must list game
    {
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredUser = String_t("b");
        AFL_CHECK_SUCCEEDS(a("21. getGames"), testee.getGames(filter, result));
        a.checkEqual("22. size", result.size(), 1U);
        a.checkEqual("23. result", result[0], gid);
    }

    // User c: must list game
    {
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredUser = String_t("c");
        AFL_CHECK_SUCCEEDS(a("31. getGames"), testee.getGames(filter, result));
        a.checkEqual("32. size", result.size(), 1U);
        a.checkEqual("33. result", result[0], gid);
    }

    // User z: must NOT list game (owner, but not player)
    {
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredUser = String_t("z");
        AFL_CHECK_SUCCEEDS(a("41. getGames"), testee.getGames(filter, result));
        a.checkEqual("42. size", result.size(), 0U);
    }

    // User a with matching filter: must list game
    {
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredState = HostGame::Joining;
        filter.requiredType = HostGame::PublicGame;
        filter.requiredUser = String_t("a");
        AFL_CHECK_SUCCEEDS(a("51. getGames"), testee.getGames(filter, result));
        a.checkEqual("52. size", result.size(), 1U);
        a.checkEqual("53. result", result[0], gid);
    }

    // User a with mismatching filter: must NOT list game
    {
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredState = HostGame::Running;
        filter.requiredType = HostGame::PublicGame;
        filter.requiredUser = String_t("a");
        AFL_CHECK_SUCCEEDS(a("61. getGames"), testee.getGames(filter, result));
        a.checkEqual("62. size", result.size(), 0U);
    }

    // User a with mismatching filter: must NOT list game
    {
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredState = HostGame::Running;
        filter.requiredUser = String_t("a");
        AFL_CHECK_SUCCEEDS(a("71. getGames"), testee.getGames(filter, result));
        a.checkEqual("72. size", result.size(), 0U);
    }
}

/** Test some more filters. */
AFL_TEST("server.host.HostGame:getGames:filter", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostGame testee(session, h.root());

    // Create a game
    h.addDefaultTools();
    int32_t gid = h.addGame(testee);
    a.checkEqual("01. addGame", gid, 1);

    // Add a tool
    StringSetKey(h.db(), "prog:tool:list").add("T");
    testee.addTool(gid, "T");

    // Matching host filter
    {
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredHost = String_t("H");
        AFL_CHECK_SUCCEEDS(a("11. getGames"), testee.getGames(filter, result));
        a.checkEqual("12. size", result.size(), 1U);
    }

    // Mismatching host filter
    {
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredHost = String_t("notH");
        AFL_CHECK_SUCCEEDS(a("21. getGames"), testee.getGames(filter, result));
        a.checkEqual("22. size", result.size(), 0U);
    }

    // Matching ship list filter
    {
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredShipList = String_t("S");
        AFL_CHECK_SUCCEEDS(a("31. getGames"), testee.getGames(filter, result));
        a.checkEqual("32. size", result.size(), 1U);
    }

    // Mismatching ship list filter
    {
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredShipList = String_t("notS");
        AFL_CHECK_SUCCEEDS(a("41. getGames"), testee.getGames(filter, result));
        a.checkEqual("42. size", result.size(), 0U);
    }

    // Matching master filter
    {
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredMaster = String_t("M");
        AFL_CHECK_SUCCEEDS(a("51. getGames"), testee.getGames(filter, result));
        a.checkEqual("52. size", result.size(), 1U);
    }

    // Mismatching master filter
    {
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredMaster = String_t("notM");
        AFL_CHECK_SUCCEEDS(a("61. getGames"), testee.getGames(filter, result));
        a.checkEqual("62. size", result.size(), 0U);
    }

    // Matching tool filter
    {
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredTool = String_t("T");
        AFL_CHECK_SUCCEEDS(a("71. getGames"), testee.getGames(filter, result));
        a.checkEqual("72. size", result.size(), 1U);
    }

    // Mismatching tool filter
    {
        afl::data::IntegerList_t result;
        HostGame::Filter filter;
        filter.requiredTool = String_t("notT");
        AFL_CHECK_SUCCEEDS(a("81. getGames"), testee.getGames(filter, result));
        a.checkEqual("82. size", result.size(), 0U);
    }
}

/** Test resetToTurn(), failure cases.
    Cannot test the non-failure cases with reasonable effort here; those are tested in c2systest. */
AFL_TEST("server.host.HostGame:resetToTurn", a)
{
    TestHarness h;
    server::host::Session adminSession;
    server::host::HostGame adminInstance(adminSession, h.root());

    server::host::Session userSession;
    userSession.setUser("x");
    server::host::HostGame userInstance(userSession, h.root());

    afl::data::StringList_t config;
    config.push_back("hostHasRun");
    config.push_back("1");
    config.push_back("masterHasRun");
    config.push_back("1");

    // Cannot reset a game that is joining
    {
        int32_t gid = adminInstance.createNewGame();
        adminInstance.setType(gid, HostGame::PublicGame);
        adminInstance.setState(gid, HostGame::Joining);
        AFL_CHECK_THROWS(a("01. resetToTurn"), adminInstance.resetToTurn(gid, 1), std::exception);
    }

    // Cannot reset to unknown turn
    {
        int32_t gid = adminInstance.createNewGame();
        adminInstance.setType(gid, HostGame::PublicGame);
        adminInstance.setConfig(gid, config);
        adminInstance.setState(gid, HostGame::Running);
        AFL_CHECK_THROWS(a("11. resetToTurn"), adminInstance.resetToTurn(gid, 10), std::exception);
    }

    // Cannot reset as user
    {
        int32_t gid = adminInstance.createNewGame();
        adminInstance.setType(gid, HostGame::PublicGame);
        adminInstance.setConfig(gid, config);
        adminInstance.setState(gid, HostGame::Running);
        AFL_CHECK_THROWS(a("21. resetToTurn"), userInstance.resetToTurn(gid, 1), std::exception);
    }
}
