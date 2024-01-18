/**
  *  \file test/server/host/hostturntest.cpp
  *  \brief Test for server::host::HostTurn
  */

#include "server/host/hostturn.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/string/format.hpp"
#include "afl/test/testrunner.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/host/game.hpp"
#include "server/host/hostgame.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "util/processrunner.hpp"

using afl::net::redis::HashKey;
using afl::net::redis::IntegerKey;
using afl::net::redis::StringKey;
using afl::net::redis::StringSetKey;
using afl::string::Format;
using server::host::Game;
using server::interface::HostGame;
using server::interface::HostTurn;

namespace {
    const char* DEFAULT_TIMESTAMP = "22-11-199911:22:33";
    const char* ALTERNATE_TIMESTAMP = "22-11-199912:34:56";

    const int SLOT_NR = 3;

    class TestHarness {
     public:
        TestHarness(bool ustt)
            : m_db(), m_hostFile(), m_userFile(), m_null(), m_mail(m_null), m_runner(), m_fs(afl::io::FileSystem::getInstance()),
              m_root(m_db, m_hostFile, m_userFile, m_mail, m_runner, m_fs, makeConfig(ustt)),
              m_hostFileClient(m_hostFile)
            { }

        server::host::Root& root()
            { return m_root; }

        afl::net::CommandHandler& db()
            { return m_db; }

        server::interface::FileBase& hostFile()
            { return m_hostFileClient; }

        void addUser(String_t userId);

        int32_t createNewGame(HostGame::Type type, HostGame::State state);

        int32_t prepareGame(const char* timestamp);

        String_t createTurn(const char* timestamp);

     private:
        static server::host::Configuration makeConfig(bool ustt);

        afl::net::redis::InternalDatabase m_db;
        server::file::InternalFileServer m_hostFile;
        server::file::InternalFileServer m_userFile;
        afl::net::NullCommandHandler m_null;
        server::interface::MailQueueClient m_mail;
        util::ProcessRunner m_runner;
        afl::io::FileSystem& m_fs;
        server::host::Root m_root;
        server::interface::FileBaseClient m_hostFileClient;
    };
}

void
TestHarness::addUser(String_t userId)
{
    StringSetKey(m_db, "user:all").add(userId);
    StringKey(m_db, "uid:" + userId).set(userId);
    HashKey(m_db, "user:" + userId + ":profile").stringField("email").set(userId + "@examp.le");
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

int32_t
TestHarness::prepareGame(const char* timestamp)
{
    // Create dummy scripts
    hostFile().createDirectoryTree("bin");
    hostFile().createDirectoryTree("defaults");
    hostFile().putFile("bin/checkturn.sh", "exit 0");

    // Create users
    addUser("ua");

    // Create a game
    int32_t gid = createNewGame(HostGame::PublicGame, HostGame::Running);

    // Configure the game
    {
        Game g(root(), gid);
        g.pushPlayerSlot(SLOT_NR, "ua", root());
        g.setConfig("timestamp", timestamp);
        IntegerKey(db(), Format("game:bytime:%s", timestamp)).set(gid);
    }

    return gid;
}

String_t
TestHarness::createTurn(const char* timestamp)
{
    String_t result;
    result += char(SLOT_NR); // player
    result += '\0';
    result.append(4, '\0');  // number of commands
    result += timestamp;     // timestamp
    result += "xx";          // unused
    result += "yy";          // timestamp checksum
    result.append(256, 'z'); // DOS trailer
    return result;
}

server::host::Configuration
TestHarness::makeConfig(bool ustt)
{
    server::host::Configuration config;
    config.workDirectory = "/tmp";
    config.usersSeeTemporaryTurns = ustt;
    return config;
}

/********************************* Tests *********************************/



/** Test turn file upload (submit()).
    This creates a test setup, where the checkturn script produces a hardcoded result. */
AFL_TEST("server.host.HostTurn:submit", a)
{
    // Prepare defaults
    TestHarness h(false);
    int32_t gid = h.prepareGame(DEFAULT_TIMESTAMP);
    String_t dummyTurn = h.createTurn(DEFAULT_TIMESTAMP);

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());

    String_t fileName = Format("games/0001/in/player%d.trn", SLOT_NR);

    // - Upload a simple turn
    {
        HostTurn::Result result = testee.submit(dummyTurn, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing);

        // Check result
        a.checkEqual("01. state",         result.state, HostTurn::GreenTurn);
        a.checkEqual("02. gameId",        result.gameId, gid);
        a.checkEqual("03. slot",          result.slot, SLOT_NR);
        a.checkEqual("04. previousState", result.previousState, HostTurn::MissingTurn);
        a.checkEqual("05. userId",        result.userId, "");

        // Verify that turn is in inbox folder
        a.checkEqual("11. getFile", h.hostFile().getFile(fileName), dummyTurn);
    }

    // - Now classify the turn as red
    h.hostFile().putFile("bin/checkturn.sh", "exit 2");
    {
        HostTurn::Result result = testee.submit(dummyTurn + "qqq", afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing);

        // Result must be red
        a.checkEqual("21. state", result.state, HostTurn::RedTurn);

        // Turn unchanged
        a.checkEqual("31. getFile", h.hostFile().getFile(fileName), dummyTurn);
    }
}

/** Test submitting an empty file.
    Must fail with an exception. */
AFL_TEST("server.host.HostTurn:submit:empty", a)
{
    // Prepare defaults
    TestHarness h(false);

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());
    AFL_CHECK_THROWS(a, testee.submit(String_t(), afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing), std::exception);
}

/** Test submitting an empty file, with game Id given.
    Must fail with an exception. */
AFL_TEST("server.host.HostTurn:submit:empty:game-id-given", a)
{
    // Prepare defaults
    TestHarness h(false);
    int32_t gid = h.prepareGame(DEFAULT_TIMESTAMP);

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());
    AFL_CHECK_THROWS(a, testee.submit(String_t(), gid, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing), std::exception);
}

/** Test submitting a stale file, no game Id given (game cannot be determined).
    Must fail with an exception. */
AFL_TEST("server.host.HostTurn:submit:stale", a)
{
    // Prepare defaults
    TestHarness h(false);

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());
    AFL_CHECK_THROWS(a, testee.submit(h.createTurn(ALTERNATE_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing), std::exception);
}

/** Test submitting a stale file, with game Id given.
    Must produce "stale" result. */
AFL_TEST("server.host.HostTurn:submit:stale:game-id-given", a)
{
    // Prepare defaults
    TestHarness h(false);
    int32_t gid = h.prepareGame(DEFAULT_TIMESTAMP);

    // Staleness is NOT (currently) determined internally by c2host, even though we could compare timestamps.
    // This is left up to the checkturn script. Hence, give it a script that reports stale.
    h.hostFile().putFile("bin/checkturn.sh", "exit 4");

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());

    HostTurn::Result result = testee.submit(h.createTurn(ALTERNATE_TIMESTAMP), gid, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing);
    a.checkEqual("01. state",         result.state, HostTurn::StaleTurn);
    a.checkEqual("02. gameId",        result.gameId, gid);
    a.checkEqual("03. slot",          result.slot, SLOT_NR);
    a.checkEqual("04. previousState", result.previousState, HostTurn::MissingTurn);
    a.checkEqual("05. userId",        result.userId, "");
}

/** Test submitting as wrong user.
    Must be rejected. */
AFL_TEST("server.host.HostTurn:submit:wrong-user", a)
{
    // Prepare defaults
    TestHarness h(false);
    int32_t gid = h.prepareGame(DEFAULT_TIMESTAMP);

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());
    session.setUser("z");
    AFL_CHECK_THROWS(a("01. submit"), testee.submit(h.createTurn(DEFAULT_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing), std::exception);

    // Specifying targets does not change outcome
    AFL_CHECK_THROWS(a("11. submit"), testee.submit(h.createTurn(DEFAULT_TIMESTAMP), gid, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing), std::exception);
    AFL_CHECK_THROWS(a("12. submit"), testee.submit(h.createTurn(DEFAULT_TIMESTAMP), gid, SLOT_NR, afl::base::Nothing, afl::base::Nothing), std::exception);
}

/** Test submitting via email.
    Must succeed. */
AFL_TEST("server.host.HostTurn:submit:email", a)
{
    // Prepare defaults
    TestHarness h(false);
    int32_t gid = h.prepareGame(DEFAULT_TIMESTAMP);

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());

    HostTurn::Result result = testee.submit(h.createTurn(DEFAULT_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, String_t("ua@examp.le"), afl::base::Nothing);
    a.checkEqual("01. state",         result.state, HostTurn::GreenTurn);
    a.checkEqual("02. gameId",        result.gameId, gid);
    a.checkEqual("03. slot",          result.slot, SLOT_NR);
    a.checkEqual("04. previousState", result.previousState, HostTurn::MissingTurn);
    a.checkEqual("05. userId",        result.userId, "ua");
}

/** Test submitting via email, differing case.
    Must succeed. */
AFL_TEST("server.host.HostTurn:submit:email:different-case", a)
{
    // Prepare defaults
    TestHarness h(false);
    int32_t gid = h.prepareGame(DEFAULT_TIMESTAMP);

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());

    HostTurn::Result result = testee.submit(h.createTurn(DEFAULT_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, String_t("UA@Examp.LE"), afl::base::Nothing);
    a.checkEqual("01. state",         result.state, HostTurn::GreenTurn);
    a.checkEqual("02. gameId",        result.gameId, gid);
    a.checkEqual("03. slot",          result.slot, SLOT_NR);
    a.checkEqual("04. previousState", result.previousState, HostTurn::MissingTurn);
    a.checkEqual("05. userId",        result.userId, "ua");
}

/** Test submitting via email, wrong address.
    Must fail. */
AFL_TEST("server.host.HostTurn:submit:email:wrong-address", a)
{
    // Prepare defaults
    TestHarness h(false);
    h.prepareGame(DEFAULT_TIMESTAMP);

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());

    AFL_CHECK_THROWS(a, testee.submit(h.createTurn(DEFAULT_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, String_t("who@examp.le"), afl::base::Nothing), std::exception);
}

/** Test submitting via email, user context.
    Must fail; this is an admin-only feature. */
AFL_TEST("server.host.HostTurn:submit:email:user-context", a)
{
    // Prepare defaults
    TestHarness h(false);
    h.prepareGame(DEFAULT_TIMESTAMP);

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());
    session.setUser("ua");

    AFL_CHECK_THROWS(a, testee.submit(h.createTurn(DEFAULT_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, String_t("ua@examp.le"), afl::base::Nothing), std::exception);
}

/** Test submitting via email, stale file.
    Must fail. */
AFL_TEST("server.host.HostTurn:submit:email:stale", a)
{
    // Prepare defaults
    TestHarness h(false);

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());

    AFL_CHECK_THROWS(a, testee.submit(h.createTurn(ALTERNATE_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, String_t("ua@examp.le"), afl::base::Nothing), std::exception);
}

/** Test statuses. */
AFL_TEST("server.host.HostTurn:submit:status", a)
{
    // Prepare defaults
    TestHarness h(false);
    int32_t gid = h.prepareGame(DEFAULT_TIMESTAMP);

    // Three different contexts
    server::host::Session adminSession;
    server::host::HostGame admin(adminSession, h.root());

    server::host::Session player1Session;
    server::host::HostGame player1(player1Session, h.root());
    player1Session.setUser("ua");

    server::host::Session player2Session;
    server::host::HostGame player2(player2Session, h.root());
    player2Session.setUser("ub");

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());

    // Submit a correct turn
    AFL_CHECK_SUCCEEDS(a("01. submit"), testee.submit(h.createTurn(DEFAULT_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing));

    // Read out state in three contexts
    HostGame::Info i = admin.getInfo(gid);
    a.check("11. turnStates", i.turnStates.isValid());
    a.checkEqual("12. turnStates", (*i.turnStates.get())[2], Game::TurnGreen);

    i = player1.getInfo(gid);
    a.check("21. turnStates", i.turnStates.isValid());
    a.checkEqual("22. turnStates", (*i.turnStates.get())[2], Game::TurnGreen);

    i = player2.getInfo(gid);
    a.check("31. turnStates", i.turnStates.isValid());
    a.checkEqual("32. turnStates", (*i.turnStates.get())[2], Game::TurnGreen);

    // Mark temporary
    AFL_CHECK_SUCCEEDS(a("41. setTemporary"), testee.setTemporary(gid, SLOT_NR, true));

    // Read out state in three contexts
    i = admin.getInfo(gid);
    a.check("51. turnStates", i.turnStates.isValid());
    a.checkEqual("52. turnStates", (*i.turnStates.get())[2], Game::TurnGreen | Game::TurnIsTemporary);

    i = player1.getInfo(gid);
    a.check("61. turnStates", i.turnStates.isValid());
    a.checkEqual("62. turnStates", (*i.turnStates.get())[2], Game::TurnGreen | Game::TurnIsTemporary);

    i = player2.getInfo(gid);
    a.check("71. turnStates", i.turnStates.isValid());
    a.checkEqual("72. turnStates", (*i.turnStates.get())[2], Game::TurnGreen);  // other player does not see Temporary flag, disabled in config

    // Submit a yellow turn
    h.hostFile().putFile("bin/checkturn.sh", "exit 1");
    AFL_CHECK_SUCCEEDS(a("81. submit"), testee.submit(h.createTurn(DEFAULT_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing));

    // Read out state in three contexts
    i = admin.getInfo(gid);
    a.check("91. turnStates", i.turnStates.isValid());
    a.checkEqual("92. turnStates", (*i.turnStates.get())[2], Game::TurnYellow);

    i = player1.getInfo(gid);
    a.check("101. turnStates", i.turnStates.isValid());
    a.checkEqual("102. turnStates", (*i.turnStates.get())[2], Game::TurnYellow);

    i = player2.getInfo(gid);
    a.check("111. turnStates", i.turnStates.isValid());
    a.checkEqual("112. turnStates", (*i.turnStates.get())[2], Game::TurnGreen);  // other player does not see Yellow

    // Mark temporary
    AFL_CHECK_SUCCEEDS(a("121. setTemporary"), testee.setTemporary(gid, SLOT_NR, true));

    // Read out state in three contexts
    i = admin.getInfo(gid);
    a.check("131. turnStates", i.turnStates.isValid());
    a.checkEqual("132. turnStates", (*i.turnStates.get())[2], Game::TurnYellow | Game::TurnIsTemporary);

    i = player1.getInfo(gid);
    a.check("141. turnStates", i.turnStates.isValid());
    a.checkEqual("142. turnStates", (*i.turnStates.get())[2], Game::TurnYellow | Game::TurnIsTemporary);

    i = player2.getInfo(gid);
    a.check("151. turnStates", i.turnStates.isValid());
    a.checkEqual("152. turnStates", (*i.turnStates.get())[2], Game::TurnGreen);  // other player does not see Yellow or Temporary flag
}

/** Test statuses, with the "users see temporary turns" option enabled. */
AFL_TEST("server.host.HostTurn:submit:temp-visible", a)
{
    // Prepare defaults
    TestHarness h(true);
    int32_t gid = h.prepareGame(DEFAULT_TIMESTAMP);

    // Only testing the "player2" context here
    server::host::Session player2Session;
    server::host::HostGame player2(player2Session, h.root());
    player2Session.setUser("ub");

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());

    // Submit a correct turn
    AFL_CHECK_SUCCEEDS(a("01. submit"), testee.submit(h.createTurn(DEFAULT_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing));

    // Read out state
    HostGame::Info i = player2.getInfo(gid);
    a.check("11. turnStates", i.turnStates.isValid());
    a.checkEqual("12. turnStates", (*i.turnStates.get())[2], Game::TurnGreen);

    // Mark temporary
    AFL_CHECK_SUCCEEDS(a("21. setTemporary"), testee.setTemporary(gid, SLOT_NR, true));

    // Read out state
    i = player2.getInfo(gid);
    a.check("31. turnStates", i.turnStates.isValid());
    a.checkEqual("32. turnStates", (*i.turnStates.get())[2], Game::TurnGreen | Game::TurnIsTemporary);  // Temporary flag now visible

    // Submit a yellow turn
    h.hostFile().putFile("bin/checkturn.sh", "exit 1");
    AFL_CHECK_SUCCEEDS(a("41. submit"), testee.submit(h.createTurn(DEFAULT_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing));

    // Read out state
    i = player2.getInfo(gid);
    a.check("51. turnStates", i.turnStates.isValid());
    a.checkEqual("52. turnStates", (*i.turnStates.get())[2], Game::TurnGreen);  // other player does not see Yellow

    // Mark temporary
    AFL_CHECK_SUCCEEDS(a("61. setTemporary"), testee.setTemporary(gid, SLOT_NR, true));

    // Read out state
    i = player2.getInfo(gid);
    a.check("71. turnStates", i.turnStates.isValid());
    a.checkEqual("72. turnStates", (*i.turnStates.get())[2], Game::TurnGreen | Game::TurnIsTemporary);  // Temporary flag now visible
}

/** Test errors in setTemporary. */
AFL_TEST("server.host.HostTurn:setTemporary:error", a)
{
    // Prepare defaults
    TestHarness h(false);
    int32_t gid = h.prepareGame(DEFAULT_TIMESTAMP);

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());

    // Cannot set temporary if there is no turn
    AFL_CHECK_THROWS(a("01. setTemporary"), testee.setTemporary(gid, SLOT_NR, true), std::exception);

    // Upload a turn
    AFL_CHECK_SUCCEEDS(a("11. submit"), testee.submit(h.createTurn(DEFAULT_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing));

    // Cannot set temporary as different user
    session.setUser("z");
    AFL_CHECK_THROWS(a("21. setTemporary"), testee.setTemporary(gid, SLOT_NR, true), std::exception);
}
