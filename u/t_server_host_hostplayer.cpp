/**
  *  \file u/t_server_host_hostplayer.cpp
  *  \brief Test for server::host::HostPlayer
  */

#include "server/host/hostplayer.hpp"

#include "t_server_host.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/stringlistkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/host/cron.hpp"
#include "server/host/game.hpp"
#include "server/host/hostgame.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "u/files.hpp"
#include "util/processrunner.hpp"

using server::interface::HostGame;
using server::interface::HostPlayer;
using afl::string::Format;
using afl::net::redis::StringKey;
using afl::net::redis::StringListKey;
using afl::net::redis::StringSetKey;
using afl::net::redis::HashKey;

namespace {
    class TestHarness {
     public:
        TestHarness()
            : m_db(), m_hostFile(), m_userFile(), m_null(), m_mail(m_null), m_runner(), m_fs(),
              m_root(m_db, m_hostFile, m_userFile, m_mail, m_runner, m_fs, server::host::Configuration()),
              m_hostFileClient(m_hostFile), m_userFileClient(m_userFile)
            { }

        server::host::Root& root()
            { return m_root; }

        afl::net::CommandHandler& db()
            { return m_db; }

        server::interface::FileBase& hostFile()
            { return m_hostFileClient; }

        server::interface::FileBase& userFile()
            { return m_userFileClient; }

        int32_t createNewGame(HostGame::Type type, HostGame::State state);

        void addUsers();

        void addDefaultRaceNames();

     private:
        afl::net::redis::InternalDatabase m_db;
        server::file::InternalFileServer m_hostFile;
        server::file::InternalFileServer m_userFile;
        afl::net::NullCommandHandler m_null;
        server::interface::MailQueueClient m_mail;
        util::ProcessRunner m_runner;
        afl::io::NullFileSystem m_fs;
        server::host::Root m_root;
        server::interface::FileBaseClient m_hostFileClient;
        server::interface::FileBaseClient m_userFileClient;
    };

    class CronMock : public server::host::Cron,
                     public afl::test::CallReceiver
    {
     public:
        CronMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual Event_t getGameEvent(int32_t gameId)
            {
                checkCall(Format("getGameEvent(%d)", gameId));
                return consumeReturnValue<Event_t>();
            }

        virtual void listGameEvents(std::vector<Event_t>& /*result*/)
            { }

        virtual void handleGameChange(int32_t gameId)
            { checkCall(Format("handleGameChange(%d)", gameId)); }

        virtual void suspendScheduler(server::Time_t absTime)
            { checkCall(Format("suspendScheduler(%d)", absTime)); }
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

void
TestHarness::addDefaultRaceNames()
{
    server::interface::FileBaseClient hostFile(m_hostFile);
    hostFile.createDirectoryTree("defaults");
    hostFile.putFile("defaults/race.nm", afl::string::fromBytes(getDefaultRaceNames()));
}

void
TestHarness::addUsers()
{
    for (int i = 1; i <= 20; ++i) {
        String_t userId = Format("u%d", i);
        StringSetKey(m_db, "user:all").add(userId);
        StringKey(m_db, Format("uid:%s", userId)).set(userId);
    }
}

/** Test basic join() behaviour.
    Commands must be accepted and notify the scheduler. */
void
TestServerHostHostPlayer::testJoin()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    CronMock cron("testJoin");
    h.root().setCron(&cron);
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    TS_ASSERT_EQUALS(gid, 1);

    // Join users
    for (int i = 1; i <= 10; ++i) {
        testee.join(gid, i, Format("u%d", i));
    }

    // Joining the final user must start the game
    cron.expectCall("handleGameChange(1)");
    testee.join(gid, 11, "u11");

    // Resigning will again notify the scheduler
    cron.expectCall("handleGameChange(1)");
    testee.resign(gid, 7, "u7");

    cron.checkFinish();
}

/** Test join() failure cases, admin access. */
void
TestServerHostHostPlayer::testJoinFail()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    TS_ASSERT_EQUALS(gid, 1);

    // Error: game does not exist
    TS_ASSERT_THROWS(testee.join(77, 1, "u1"), std::exception);

    // Error: slot does not exist
    TS_ASSERT_THROWS(testee.join(gid, 99, "u1"), std::exception);

    // Error: user does not exist
    TS_ASSERT_THROWS(testee.join(gid, 1, "zz"), std::exception);

    // Error: slot already taken
    TS_ASSERT_THROWS_NOTHING(testee.join(gid, 3, "u3"));
    TS_ASSERT_THROWS(testee.join(gid, 3, "u4"), std::exception);

    // Not an error: you are already on this game - not detected if we're admin
    TS_ASSERT_THROWS_NOTHING(testee.join(gid, 4, "u3"));
}

/** Test join() failure cases, user access. */
void
TestServerHostHostPlayer::testJoinFailUser()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    TS_ASSERT_EQUALS(gid, 1);
    TS_ASSERT_THROWS_NOTHING(testee.join(gid, 3, "u4"));

    // Set user context for all subsequent commands
    session.setUser("u3");

    // Error: game does not exist
    TS_ASSERT_THROWS(testee.join(77, 1, "u3"), std::exception);

    // Error: slot does not exist
    TS_ASSERT_THROWS(testee.join(gid, 99, "u3"), std::exception);

    // Error: slot already taken
    TS_ASSERT_THROWS(testee.join(gid, 3, "u3"), std::exception);

    // Error: you cannot join someone else
    TS_ASSERT_THROWS(testee.join(gid, 3, "u4"), std::exception);

    // Error: you are already on this game
    TS_ASSERT_THROWS_NOTHING(testee.join(gid, 1, "u3"));
    TS_ASSERT_THROWS(testee.join(gid, 2, "u3"), std::exception);
}

/** Test resign(), normal cases. */
void
TestServerHostHostPlayer::testResign()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    CronMock cron("testResign");
    h.root().setCron(&cron);
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    TS_ASSERT_EQUALS(gid, 1);

    // Join some users
    testee.join(gid, 1, "u1");
    testee.join(gid, 2, "u2");
    testee.join(gid, 3, "u3");
    testee.substitute(gid, 3, "u4");

    // Resign: no notification
    testee.resign(gid, 3, "u4");

    // Resign: notification
    cron.expectCall("handleGameChange(1)");
    testee.resign(gid, 3, "u3");

    cron.checkFinish();
}

/** Test resign() combo.
    Resigning all replacements resigns further replacements. */
void
TestServerHostHostPlayer::testResignCombo()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    TS_ASSERT_EQUALS(gid, 1);

    // Join 4 users to one slot
    testee.join(gid, 1, "u1");
    testee.substitute(gid, 1, "u2");
    testee.substitute(gid, 1, "u3");
    testee.substitute(gid, 1, "u4");
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users").size(), 4);

    // Resign u3
    testee.resign(gid, 1, "u3");

    // u1,u2 remain
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users").size(), 2);
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users")[0], "u1");
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users")[1], "u2");
}

/** Test resign() combo, case 2.
    Resigning a primary player resigns the whole slot. */
void
TestServerHostHostPlayer::testResignCombo2()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    TS_ASSERT_EQUALS(gid, 1);

    // Join 4 users to one slot
    testee.join(gid, 1, "u1");
    testee.substitute(gid, 1, "u2");
    testee.substitute(gid, 1, "u3");
    testee.substitute(gid, 1, "u4");
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users").size(), 4);

    // Resign u1
    testee.resign(gid, 1, "u1");

    // Nobody remains
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users").size(), 0);
}

/** Test resign() combo, permissions. */
void
TestServerHostHostPlayer::testResignComboPerm()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    TS_ASSERT_EQUALS(gid, 1);

    // Join 5 users to one slot
    testee.join(gid, 1, "u1");
    testee.substitute(gid, 1, "u2");
    testee.substitute(gid, 1, "u3");
    testee.substitute(gid, 1, "u4");
    testee.substitute(gid, 1, "u5");
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users").size(), 5);

    // Set user u3
    session.setUser("u3");

    // Cannot resign primary or previous replacement, or users who are not playing
    TS_ASSERT_THROWS(testee.resign(gid, 1, "u1"), std::exception);
    TS_ASSERT_THROWS(testee.resign(gid, 1, "u2"), std::exception);
    TS_ASSERT_THROWS(testee.resign(gid, 1, "u6"), std::exception);

    // Can resign u5
    testee.resign(gid, 1, "u5");
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users").size(), 4);

    // Can resign ourselves and our replacement
    testee.resign(gid, 1, "u3");
    
    // u1,u2 remain
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users").size(), 2);
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users")[0], "u1");
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users")[1], "u2");
}

/** Test substitute() behaviour. */
void
TestServerHostHostPlayer::testSubstitute()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    TS_ASSERT_EQUALS(gid, 1);

    // Join 5 users to one slot
    testee.join(gid, 1, "u1");
    testee.substitute(gid, 1, "u2");
    testee.substitute(gid, 1, "u3");
    testee.substitute(gid, 1, "u4");
    testee.substitute(gid, 1, "u5");
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users").size(), 5);

    // Substitute u3: this will drop everyone after u3
    testee.substitute(gid, 1, "u3");
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users").size(), 3);
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users")[2], "u3");

    // Substitute u4: will add
    testee.substitute(gid, 1, "u4");
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users").size(), 4);
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users")[3], "u4");
}

/** Test substitute() behaviour, user version. */
void
TestServerHostHostPlayer::testSubstituteUser()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    TS_ASSERT_EQUALS(gid, 1);

    // Join 5 users to one slot
    testee.join(gid, 1, "u1");
    testee.substitute(gid, 1, "u2");
    testee.substitute(gid, 1, "u3");
    testee.substitute(gid, 1, "u4");
    testee.substitute(gid, 1, "u5");
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users").size(), 5);

    // Set as user u3
    session.setUser("u3");

    // Try to substitute u2: not possible because they are before us
    TS_ASSERT_THROWS(testee.substitute(gid, 1, "u2"), std::exception);

    // Try to substitute u4: ok, kicks u5
    TS_ASSERT_THROWS_NOTHING(testee.substitute(gid, 1, "u4"));
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users").size(), 4);

    // Substitute u9: ok, replaces u5 by u9
    TS_ASSERT_THROWS_NOTHING(testee.substitute(gid, 1, "u9"));
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users").size(), 4);
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users")[2], "u3");
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users")[3], "u9");

    // Substitute u3: kicks everyone up to u3
    TS_ASSERT_THROWS_NOTHING(testee.substitute(gid, 1, "u3"));
    TS_ASSERT_EQUALS(StringListKey(h.db(), "game:1:player:1:users").size(), 3);
}

/** Test substitute() behaviour, empty slot.
    This must fail. */
void
TestServerHostHostPlayer::testSubstituteEmpty()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    TS_ASSERT_EQUALS(gid, 1);

    // Substitute into empty slot, fails
    TS_ASSERT_THROWS(testee.substitute(gid, 2, "u2"), std::exception);
}

/** Test add(). */
void
TestServerHostHostPlayer::testAddPlayer()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();
    session.setUser("u3");

    // Create a private game
    int32_t gid = h.createNewGame(HostGame::PrivateGame, HostGame::Joining);
    TS_ASSERT_EQUALS(gid, 1);

    // Game access initially not allowed to user
    TS_ASSERT_THROWS(server::host::HostGame(session, h.root()).getInfo(gid), std::exception);

    // Player cannot add themselves
    TS_ASSERT_THROWS(testee.add(gid, "u3"), std::exception);

    // Add player to that game using admin permissions
    {
        server::host::Session adminSession;
        server::host::HostPlayer(adminSession, h.root()).add(gid, "u3");
    }

    // Game access now works
    TS_ASSERT_THROWS_NOTHING(server::host::HostGame(session, h.root()).getInfo(gid));
}

/** Get getInfo(), list(). */
void
TestServerHostHostPlayer::testSlotInfo()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();
    h.addDefaultRaceNames();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    TS_ASSERT_EQUALS(gid, 1);

    // Join some users
    testee.join(gid, 1, "u1");
    testee.substitute(gid, 1, "u2");
    testee.join(gid, 7, "u3");
    testee.join(gid, 11, "u4");

    // Get information about a slot
    {
        HostPlayer::Info i = testee.getInfo(gid, 1);
        TS_ASSERT_EQUALS(i.longName, "The Solar Federation");
        TS_ASSERT_EQUALS(i.shortName, "The Feds");
        TS_ASSERT_EQUALS(i.adjectiveName, "Fed");
        TS_ASSERT_EQUALS(i.userIds.size(), 2U);
        TS_ASSERT_EQUALS(i.userIds[0], "u1");
        TS_ASSERT_EQUALS(i.userIds[1], "u2");
        TS_ASSERT_EQUALS(i.numEditable, 2);
        TS_ASSERT_EQUALS(i.joinable, false);
    }
    {
        HostPlayer::Info i = testee.getInfo(gid, 7);
        TS_ASSERT_EQUALS(i.userIds.size(), 1U);
        TS_ASSERT_EQUALS(i.userIds[0], "u3");
    }
    {
        HostPlayer::Info i = testee.getInfo(gid, 9);
        TS_ASSERT_EQUALS(i.userIds.size(), 0U);
        TS_ASSERT_EQUALS(i.joinable, true);
    }

    // List
    // FIXME: test all=true vs all=false!
    {
        std::map<int,HostPlayer::Info> result;
        testee.list(gid, false, result);
        TS_ASSERT_EQUALS(result.size(), 11U);
        for (int i = 1; i <= 11; ++i) {
            TS_ASSERT_DIFFERS(result.find(i), result.end());
        }
        TS_ASSERT_EQUALS(result[1].shortName, "The Feds");
        TS_ASSERT_EQUALS(result[9].shortName, "The Robots");
    }
}

/** Test setDirectory(), getDirectory(). */
void
TestServerHostHostPlayer::testDirectory()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();
    h.userFile().createDirectoryAsUser("u4home", "u4");

    // Create a game and join a user
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    TS_ASSERT_EQUALS(gid, 1);
    testee.join(gid, 3, "u4");

    // Directory name initially unset
    TS_ASSERT_EQUALS(testee.getDirectory(gid, "u4"), "");

    // Set directory
    TS_ASSERT_THROWS_NOTHING(testee.setDirectory(gid, "u4", "u4home/x/y"));

    // Query
    TS_ASSERT_EQUALS(testee.getDirectory(gid, "u4"), "u4home/x/y");

    // Verify
    TS_ASSERT_EQUALS(h.userFile().getFileInformation("u4home/x/y").type, server::interface::FileBase::IsDirectory);
    TS_ASSERT_EQUALS(h.userFile().getDirectoryIntegerProperty("u4home/x/y", "game"), gid);
}

/** Test setDirectory(), permission error case.
    Setting the directory to a non-writable area must fail, and not change the game config. */
void
TestServerHostHostPlayer::testDirectoryErrorFilePerm()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();

    // Create a game and join a user
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    TS_ASSERT_EQUALS(gid, 1);
    testee.join(gid, 3, "u4");

    // Set directory.
    // Fails because we didn't create the parent directory.
    TS_ASSERT_THROWS(testee.setDirectory(gid, "u4", "u4home/x/y"), std::exception);

    // Query. Must still be empty.
    TS_ASSERT_EQUALS(testee.getDirectory(gid, "u4"), "");
}

/** Test setDirectory(), user error case.
    Setting the directory for a different user is refused. */
void
TestServerHostHostPlayer::testDirectoryErrorUserPerm()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();
    h.userFile().createDirectoryAsUser("u4home", "u4");
    h.userFile().createDirectoryAsUser("u1home", "u1");

    // Create a game and join a user
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    TS_ASSERT_EQUALS(gid, 1);
    testee.join(gid, 3, "u4");

    // Set directory as user u1
    session.setUser("u1");
    TS_ASSERT_THROWS(testee.setDirectory(gid, "u4", "u1home/x/y"), std::exception);
    TS_ASSERT_THROWS(testee.setDirectory(gid, "u4", "u4home/x/y"), std::exception);

    // Query
    TS_ASSERT_THROWS(testee.getDirectory(gid, "u4"), std::exception);

    // Query as admin, it didn't change
    session.setUser(String_t());
    TS_ASSERT_EQUALS(testee.getDirectory(gid, "u4"), "");
}

/** Test setDirectory(), subscription error case.
    Setting the directory fails if you're not subscribed. */
void
TestServerHostHostPlayer::testDirectoryErrorGame()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();
    h.userFile().createDirectoryAsUser("u4home", "u4");

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    TS_ASSERT_EQUALS(gid, 1);

    // Set directory, fails because we're not subscribed
    TS_ASSERT_THROWS(testee.setDirectory(gid, "u4", "u4home/x/y"), std::exception);

    // Query, fails because we're not subscribed
    TS_ASSERT_THROWS(testee.getDirectory(gid, "u4"), std::exception);
}

/** Test setDirectory(), error during directory change. */
void
TestServerHostHostPlayer::testDirectoryErrorChange()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();
    h.userFile().createDirectoryAsUser("u4home", "u4");

    // Create a game and join a user
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    TS_ASSERT_EQUALS(gid, 1);
    testee.join(gid, 3, "u4");

    // Set directory, works
    TS_ASSERT_THROWS_NOTHING(testee.setDirectory(gid, "u4", "u4home/x/y"));
    TS_ASSERT_EQUALS(testee.getDirectory(gid, "u4"), "u4home/x/y");
    TS_ASSERT_EQUALS(h.userFile().getDirectoryIntegerProperty("u4home/x/y", "game"), gid);

    // Move to different place, fails
    TS_ASSERT_THROWS(testee.setDirectory(gid, "u4", "elsewhere/y"), std::exception);

    // Configuration unchanged
    TS_ASSERT_EQUALS(testee.getDirectory(gid, "u4"), "u4home/x/y");
}

/** Test setDirectory(), conflict case.
    A game must refuse pointing its directory at the same place as another game. */
void
TestServerHostHostPlayer::testDirectoryConflict()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();
    h.userFile().createDirectoryAsUser("u4home", "u4");

    // Create two games and join a user
    int32_t gid1 = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    int32_t gid2 = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    TS_ASSERT_EQUALS(gid1, 1);
    TS_ASSERT_EQUALS(gid2, 2);
    testee.join(gid1, 3, "u4");
    testee.join(gid2, 4, "u4");

    // Set directory, works
    TS_ASSERT_THROWS_NOTHING(testee.setDirectory(gid1, "u4", "u4home/x/y"));
    TS_ASSERT_EQUALS(testee.getDirectory(gid1, "u4"), "u4home/x/y");
    TS_ASSERT_EQUALS(h.userFile().getDirectoryIntegerProperty("u4home/x/y", "game"), gid1);

    // Set other game's directory the same as this one, must fail and leave the configuration unchanged
    TS_ASSERT_THROWS(testee.setDirectory(gid2, "u4", "u4home/x/y"), std::exception);
    TS_ASSERT_EQUALS(testee.getDirectory(gid1, "u4"), "u4home/x/y");
    TS_ASSERT_EQUALS(testee.getDirectory(gid2, "u4"), "");
    TS_ASSERT_EQUALS(h.userFile().getDirectoryIntegerProperty("u4home/x/y", "game"), gid1);
}

/** Test setDirectory(), move case. */
void
TestServerHostHostPlayer::testDirectoryMove()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();
    h.userFile().createDirectoryAsUser("u4home", "u4");

    // Create a game and join a user
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    TS_ASSERT_EQUALS(gid, 1);
    testee.join(gid, 3, "u4");

    // Set directory
    TS_ASSERT_THROWS_NOTHING(testee.setDirectory(gid, "u4", "u4home/x/y"));
    TS_ASSERT_EQUALS(h.userFile().getDirectoryIntegerProperty("u4home/x/y", "game"), gid);

    // Move
    TS_ASSERT_THROWS_NOTHING(testee.setDirectory(gid, "u4", "u4home/a/b"));
    TS_ASSERT_EQUALS(h.userFile().getDirectoryIntegerProperty("u4home/a/b", "game"), gid);
    TS_ASSERT_EQUALS(h.userFile().getDirectoryIntegerProperty("u4home/x/y", "game"), 0);
}

/** Test checkFile(). */
void
TestServerHostHostPlayer::testCheckFile()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();
    h.userFile().createDirectoryAsUser("u3home", "u3");

    // Create a game and join two users
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    TS_ASSERT_EQUALS(gid, 1);
    testee.join(gid, 1, "u1");
    testee.join(gid, 3, "u3");
    testee.setDirectory(gid, "u3", "u3home/x");

    // Check with no directory name: Stale for 1 because they have not set a directory
    TS_ASSERT_EQUALS(testee.checkFile(1, "u1", "xyplan.dat", afl::base::Nothing), HostPlayer::Stale);
    TS_ASSERT_EQUALS(testee.checkFile(1, "u3", "xyplan.dat", afl::base::Nothing), HostPlayer::Refuse);
    TS_ASSERT_EQUALS(testee.checkFile(1, "u1", "fizz.bin", afl::base::Nothing), HostPlayer::Stale);
    TS_ASSERT_EQUALS(testee.checkFile(1, "u3", "fizz.bin", afl::base::Nothing), HostPlayer::Allow);

    // Check with wrong directory name
    TS_ASSERT_EQUALS(testee.checkFile(1, "u1", "xyplan.dat", String_t("a")), HostPlayer::Stale);
    TS_ASSERT_EQUALS(testee.checkFile(1, "u3", "xyplan.dat", String_t("a")), HostPlayer::Stale);
    TS_ASSERT_EQUALS(testee.checkFile(1, "u1", "fizz.bin", String_t("a")), HostPlayer::Stale);
    TS_ASSERT_EQUALS(testee.checkFile(1, "u3", "fizz.bin", String_t("a")), HostPlayer::Stale);

    // Check with correct directory name
    TS_ASSERT_EQUALS(testee.checkFile(1, "u1", "xyplan.dat", String_t("u3home/x")), HostPlayer::Stale);
    TS_ASSERT_EQUALS(testee.checkFile(1, "u3", "xyplan.dat", String_t("u3home/x")), HostPlayer::Refuse);
    TS_ASSERT_EQUALS(testee.checkFile(1, "u1", "fizz.bin", String_t("u3home/x")), HostPlayer::Stale);
    TS_ASSERT_EQUALS(testee.checkFile(1, "u3", "fizz.bin", String_t("u3home/x")), HostPlayer::Allow);

    // Turn files: must refuse turns that don't match the player
    TS_ASSERT_EQUALS(testee.checkFile(1, "u1", "player1.trn", afl::base::Nothing), HostPlayer::Stale);
    TS_ASSERT_EQUALS(testee.checkFile(1, "u1", "player3.trn", afl::base::Nothing), HostPlayer::Stale);
    TS_ASSERT_EQUALS(testee.checkFile(1, "u3", "player1.trn", afl::base::Nothing), HostPlayer::Refuse);
    TS_ASSERT_EQUALS(testee.checkFile(1, "u3", "player3.trn", afl::base::Nothing), HostPlayer::Turn);
    TS_ASSERT_EQUALS(testee.checkFile(1, "u3", "player99.trn", afl::base::Nothing), HostPlayer::Refuse);
}

/** Test join/resign/substitute in wrong game state. */
void
TestServerHostHostPlayer::testGameState()
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Preparing);

    // Operations fail
    TS_ASSERT_THROWS(testee.join(gid, 1, "u1"), std::exception);
    TS_ASSERT_THROWS(testee.substitute(gid, 1, "u2"), std::exception);
    TS_ASSERT_THROWS(testee.resign(gid, 1, "u2"), std::exception);

    // Make it joining, add users, finish
    {
        server::host::Game g(h.root(), gid);
        g.setState(HostGame::Joining, h.root().getForum(), h.root());
        g.pushPlayerSlot(1, "u1", h.root());
        g.pushPlayerSlot(1, "u2", h.root());
        g.pushPlayerSlot(2, "u3", h.root());
        g.pushPlayerSlot(3, "u4", h.root());
        g.setState(HostGame::Finished, h.root().getForum(), h.root());
    }

    // Operations still fail
    TS_ASSERT_THROWS(testee.join(gid, 4, "u1"), std::exception);
    TS_ASSERT_THROWS(testee.substitute(gid, 3, "u2"), std::exception);
    TS_ASSERT_THROWS(testee.resign(gid, 1, "u2"), std::exception);
}

/** Test game settings. */
void
TestServerHostHostPlayer::testGetSet()
{
    using server::host::HostPlayer;

    TestHarness h;
    server::host::Session rootSession;
    server::host::Session userSession;
    server::host::Session otherSession;
    h.addUsers();
    userSession.setUser("u4");
    otherSession.setUser("u9");

    // Create two games and join a user
    int32_t gid1 = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    int32_t gid2 = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    TS_ASSERT_EQUALS(gid1, 1);
    TS_ASSERT_EQUALS(gid2, 2);
    HostPlayer(rootSession, h.root()).join(gid1, 3, "u4");
    HostPlayer(rootSession, h.root()).join(gid2, 4, "u4");

    // Initial value: empty
    // - success cases: root, player themselves
    TS_ASSERT_EQUALS(HostPlayer(rootSession, h.root()).get(gid1, "u4", "mailgametype"), "");
    TS_ASSERT_EQUALS(HostPlayer(rootSession, h.root()).get(gid2, "u4", "mailgametype"), "");
    TS_ASSERT_EQUALS(HostPlayer(userSession, h.root()).get(gid1, "u4", "mailgametype"), "");
    TS_ASSERT_EQUALS(HostPlayer(userSession, h.root()).get(gid2, "u4", "mailgametype"), "");

    // - failure cases: different player, player not on game
    TS_ASSERT_THROWS(HostPlayer(otherSession, h.root()).get(gid1, "u4", "mailgametype"), std::exception);
    TS_ASSERT_THROWS(HostPlayer(rootSession, h.root()).get(gid1, "u77", "mailgametype"), std::exception);

    // Change it
    // - success cases: root, player themselves
    TS_ASSERT_THROWS_NOTHING(HostPlayer(userSession, h.root()).set(gid1, "u4", "mailgametype", "zip"));
    TS_ASSERT_THROWS_NOTHING(HostPlayer(rootSession, h.root()).set(gid2, "u4", "mailgametype", "rst"));

    // - failure cases: different player, player not on game
    TS_ASSERT_THROWS(HostPlayer(otherSession, h.root()).set(gid2, "u4",  "mailgametype", "info"), std::exception);
    TS_ASSERT_THROWS(HostPlayer(rootSession,  h.root()).set(gid2, "u77", "mailgametype", "info"), std::exception);

    // Verify
    TS_ASSERT_EQUALS(HostPlayer(rootSession, h.root()).get(gid1, "u4", "mailgametype"), "zip");
    TS_ASSERT_EQUALS(HostPlayer(rootSession, h.root()).get(gid2, "u4", "mailgametype"), "rst");
    TS_ASSERT_EQUALS(HostPlayer(userSession, h.root()).get(gid1, "u4", "mailgametype"), "zip");
    TS_ASSERT_EQUALS(HostPlayer(userSession, h.root()).get(gid2, "u4", "mailgametype"), "rst");
}

/** Test joining with profile permissions. */
void
TestServerHostHostPlayer::testProfilePermission()
{
    using server::host::HostPlayer;

    TestHarness h;
    h.addUsers();

    // Session that has joining allowed in profile
    server::host::Session allowedSession;
    allowedSession.setUser("u1");
    HashKey(h.db(), "user:u1:profile").intField("allowjoin").set(1);

    // Session that has joining disabled in profile
    server::host::Session forbiddenSession;
    forbiddenSession.setUser("u2");
    HashKey(h.db(), "user:u2:profile").intField("allowjoin").set(0);

    // Session that says nothing in profile
    server::host::Session defaultSession;
    defaultSession.setUser("u3");

    // Admin session
    server::host::Session rootSession;

    // Do it
    // - u1 can join
    {
        int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
        TS_ASSERT_THROWS_NOTHING(HostPlayer(allowedSession, h.root()).join(gid, 1, "u1"));
    }
    // - u2 can not join
    {
        int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
        TS_ASSERT_THROWS(HostPlayer(forbiddenSession, h.root()).join(gid, 2, "u2"), std::exception);
    }
    // - u3 can join
    {
        int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
        TS_ASSERT_THROWS_NOTHING(HostPlayer(defaultSession, h.root()).join(gid, 3, "u3"));
    }
    // - root can join anyone
    {
        int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
        TS_ASSERT_THROWS_NOTHING(HostPlayer(rootSession, h.root()).join(gid, 1, "u1"));
        TS_ASSERT_THROWS_NOTHING(HostPlayer(rootSession, h.root()).join(gid, 2, "u2"));
        TS_ASSERT_THROWS_NOTHING(HostPlayer(rootSession, h.root()).join(gid, 3, "u3"));
    }
}
