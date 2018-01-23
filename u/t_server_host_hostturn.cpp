/**
  *  \file u/t_server_host_hostturn.cpp
  *  \brief Test for server::host::HostTurn
  */

#include "server/host/hostturn.hpp"

#include "t_server_host.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/string/format.hpp"
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
        TestHarness()
            : m_db(), m_hostFile(), m_userFile(), m_null(), m_mail(m_null), m_runner(), m_fs(afl::io::FileSystem::getInstance()),
              m_root(m_db, m_hostFile, m_userFile, m_mail, m_runner, m_fs, makeConfig()),
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
        static server::host::Configuration makeConfig();

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
TestHarness::makeConfig()
{
    server::host::Configuration config;
    config.workDirectory = "/tmp";
    return config;
}

/********************************* Tests *********************************/



/** Test turn file upload (submit()).
    This creates a test setup, where the checkturn script produces a hardcoded result. */
void
TestServerHostHostTurn::testSubmit()
{
    // Prepare defaults
    TestHarness h;
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
        TS_ASSERT_EQUALS(result.state, HostTurn::GreenTurn);
        TS_ASSERT_EQUALS(result.gameId, gid);
        TS_ASSERT_EQUALS(result.slot, SLOT_NR);
        TS_ASSERT_EQUALS(result.previousState, HostTurn::MissingTurn);
        TS_ASSERT_EQUALS(result.userId, "");

        // Verify that turn is in inbox folder
        TS_ASSERT_EQUALS(h.hostFile().getFile(fileName), dummyTurn);
    }

    // - Now classify the turn as red
    h.hostFile().putFile("bin/checkturn.sh", "exit 2");
    {
        HostTurn::Result result = testee.submit(dummyTurn + "qqq", afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing);

        // Result must be red
        TS_ASSERT_EQUALS(result.state, HostTurn::RedTurn);

        // Turn unchanged
        TS_ASSERT_EQUALS(h.hostFile().getFile(fileName), dummyTurn);
    }
}

/** Test submitting an empty file.
    Must fail with an exception. */
void
TestServerHostHostTurn::testSubmitEmpty()
{
    // Prepare defaults
    TestHarness h;

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());
    TS_ASSERT_THROWS(testee.submit(String_t(), afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing), std::exception);
}

/** Test submitting an empty file, with game Id given.
    Must fail with an exception. */
void
TestServerHostHostTurn::testSubmitEmptyGame()
{
    // Prepare defaults
    TestHarness h;
    int32_t gid = h.prepareGame(DEFAULT_TIMESTAMP);

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());
    TS_ASSERT_THROWS(testee.submit(String_t(), gid, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing), std::exception);
}

/** Test submitting a stale file, no game Id given (game cannot be determined).
    Must fail with an exception. */
void
TestServerHostHostTurn::testSubmitStale()
{
    // Prepare defaults
    TestHarness h;

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());
    TS_ASSERT_THROWS(testee.submit(h.createTurn(ALTERNATE_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing), std::exception);
}

/** Test submitting a stale file, with game Id given.
    Must produce "stale" result. */
void
TestServerHostHostTurn::testSubmitStaleGame()
{
    // Prepare defaults
    TestHarness h;
    int32_t gid = h.prepareGame(DEFAULT_TIMESTAMP);

    // Staleness is NOT (currently) determined internally by c2host, even though we could compare timestamps.
    // This is left up to the checkturn script. Hence, give it a script that reports stale.
    h.hostFile().putFile("bin/checkturn.sh", "exit 4");

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());

    HostTurn::Result result = testee.submit(h.createTurn(ALTERNATE_TIMESTAMP), gid, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing);
    TS_ASSERT_EQUALS(result.state, HostTurn::StaleTurn);
    TS_ASSERT_EQUALS(result.gameId, gid);
    TS_ASSERT_EQUALS(result.slot, SLOT_NR);
    TS_ASSERT_EQUALS(result.previousState, HostTurn::MissingTurn);
    TS_ASSERT_EQUALS(result.userId, "");
}

/** Test submitting as wrong user.
    Must be rejected. */
void
TestServerHostHostTurn::testSubmitWrongUser()
{
    // Prepare defaults
    TestHarness h;
    int32_t gid = h.prepareGame(DEFAULT_TIMESTAMP);

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());
    session.setUser("z");
    TS_ASSERT_THROWS(testee.submit(h.createTurn(DEFAULT_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing), std::exception);

    // Specifying targets does not change outcome
    TS_ASSERT_THROWS(testee.submit(h.createTurn(DEFAULT_TIMESTAMP), gid, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing), std::exception);
    TS_ASSERT_THROWS(testee.submit(h.createTurn(DEFAULT_TIMESTAMP), gid, SLOT_NR, afl::base::Nothing, afl::base::Nothing), std::exception);
}

/** Test submitting via email.
    Must succeed. */
void
TestServerHostHostTurn::testSubmitEmail()
{
    // Prepare defaults
    TestHarness h;
    int32_t gid = h.prepareGame(DEFAULT_TIMESTAMP);

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());

    HostTurn::Result result = testee.submit(h.createTurn(DEFAULT_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, String_t("ua@examp.le"), afl::base::Nothing);
    TS_ASSERT_EQUALS(result.state, HostTurn::GreenTurn);
    TS_ASSERT_EQUALS(result.gameId, gid);
    TS_ASSERT_EQUALS(result.slot, SLOT_NR);
    TS_ASSERT_EQUALS(result.previousState, HostTurn::MissingTurn);
    TS_ASSERT_EQUALS(result.userId, "ua");
}

/** Test submitting via email, differing case.
    Must succeed. */
void
TestServerHostHostTurn::testSubmitEmailCase()
{
    // Prepare defaults
    TestHarness h;
    int32_t gid = h.prepareGame(DEFAULT_TIMESTAMP);

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());

    HostTurn::Result result = testee.submit(h.createTurn(DEFAULT_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, String_t("UA@Examp.LE"), afl::base::Nothing);
    TS_ASSERT_EQUALS(result.state, HostTurn::GreenTurn);
    TS_ASSERT_EQUALS(result.gameId, gid);
    TS_ASSERT_EQUALS(result.slot, SLOT_NR);
    TS_ASSERT_EQUALS(result.previousState, HostTurn::MissingTurn);
    TS_ASSERT_EQUALS(result.userId, "ua");
}

/** Test submitting via email, wrong address.
    Must fail. */
void
TestServerHostHostTurn::testSubmitWrongEmail()
{
    // Prepare defaults
    TestHarness h;
    h.prepareGame(DEFAULT_TIMESTAMP);

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());

    TS_ASSERT_THROWS(testee.submit(h.createTurn(DEFAULT_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, String_t("who@examp.le"), afl::base::Nothing), std::exception);
}

/** Test submitting via email, user context.
    Must fail; this is an admin-only feature. */
void
TestServerHostHostTurn::testSubmitEmailUser()
{
    // Prepare defaults
    TestHarness h;
    h.prepareGame(DEFAULT_TIMESTAMP);

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());
    session.setUser("ua");

    TS_ASSERT_THROWS(testee.submit(h.createTurn(DEFAULT_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, String_t("ua@examp.le"), afl::base::Nothing), std::exception);
}

/** Test submitting via email, stale file.
    Must fail. */
void
TestServerHostHostTurn::testSubmitEmailStale()
{
    // Prepare defaults
    TestHarness h;

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());

    TS_ASSERT_THROWS(testee.submit(h.createTurn(ALTERNATE_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, String_t("ua@examp.le"), afl::base::Nothing), std::exception);
}

/** Test statuses. */
void
TestServerHostHostTurn::testStatus()
{
    // Prepare defaults
    TestHarness h;
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
    TS_ASSERT_THROWS_NOTHING(testee.submit(h.createTurn(DEFAULT_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing));

    // Read out state in three contexts
    HostGame::Info i = admin.getInfo(gid);
    TS_ASSERT(i.turnStates.isValid());
    TS_ASSERT_EQUALS((*i.turnStates.get())[2], Game::TurnGreen);

    i = player1.getInfo(gid);
    TS_ASSERT(i.turnStates.isValid());
    TS_ASSERT_EQUALS((*i.turnStates.get())[2], Game::TurnGreen);

    i = player2.getInfo(gid);
    TS_ASSERT(i.turnStates.isValid());
    TS_ASSERT_EQUALS((*i.turnStates.get())[2], Game::TurnGreen);

    // Mark temporary
    TS_ASSERT_THROWS_NOTHING(testee.setTemporary(gid, SLOT_NR, true));

    // Read out state in three contexts
    i = admin.getInfo(gid);
    TS_ASSERT(i.turnStates.isValid());
    TS_ASSERT_EQUALS((*i.turnStates.get())[2], Game::TurnGreen | Game::TurnIsTemporary);

    i = player1.getInfo(gid);
    TS_ASSERT(i.turnStates.isValid());
    TS_ASSERT_EQUALS((*i.turnStates.get())[2], Game::TurnGreen | Game::TurnIsTemporary);

    i = player2.getInfo(gid);
    TS_ASSERT(i.turnStates.isValid());
    TS_ASSERT_EQUALS((*i.turnStates.get())[2], Game::TurnGreen);  // other player does not see Temporary flag

    // Submit a yellow turn
    h.hostFile().putFile("bin/checkturn.sh", "exit 1");
    TS_ASSERT_THROWS_NOTHING(testee.submit(h.createTurn(DEFAULT_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing));

    // Read out state in three contexts
    i = admin.getInfo(gid);
    TS_ASSERT(i.turnStates.isValid());
    TS_ASSERT_EQUALS((*i.turnStates.get())[2], Game::TurnYellow);

    i = player1.getInfo(gid);
    TS_ASSERT(i.turnStates.isValid());
    TS_ASSERT_EQUALS((*i.turnStates.get())[2], Game::TurnYellow);

    i = player2.getInfo(gid);
    TS_ASSERT(i.turnStates.isValid());
    TS_ASSERT_EQUALS((*i.turnStates.get())[2], Game::TurnGreen);  // other player does not see Yellow

    // Mark temporary
    TS_ASSERT_THROWS_NOTHING(testee.setTemporary(gid, SLOT_NR, true));

    // Read out state in three contexts
    i = admin.getInfo(gid);
    TS_ASSERT(i.turnStates.isValid());
    TS_ASSERT_EQUALS((*i.turnStates.get())[2], Game::TurnYellow | Game::TurnIsTemporary);

    i = player1.getInfo(gid);
    TS_ASSERT(i.turnStates.isValid());
    TS_ASSERT_EQUALS((*i.turnStates.get())[2], Game::TurnYellow | Game::TurnIsTemporary);

    i = player2.getInfo(gid);
    TS_ASSERT(i.turnStates.isValid());
    TS_ASSERT_EQUALS((*i.turnStates.get())[2], Game::TurnGreen);  // other player does not see Yellow or Temporary flag
}

/** Test errors in setTemporary. */
void
TestServerHostHostTurn::testStatusErrors()
{
    // Prepare defaults
    TestHarness h;
    int32_t gid = h.prepareGame(DEFAULT_TIMESTAMP);

    // Test
    server::host::Session session;
    server::host::HostTurn testee(session, h.root());

    // Cannot set temporary if there is no turn
    TS_ASSERT_THROWS(testee.setTemporary(gid, SLOT_NR, true), std::exception);

    // Upload a turn
    TS_ASSERT_THROWS_NOTHING(testee.submit(h.createTurn(DEFAULT_TIMESTAMP), afl::base::Nothing, afl::base::Nothing, afl::base::Nothing, afl::base::Nothing));

    // Cannot set temporary as different user
    session.setUser("z");
    TS_ASSERT_THROWS(testee.setTemporary(gid, SLOT_NR, true), std::exception);
}
