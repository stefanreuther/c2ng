/**
  *  \file test/server/host/hostplayertest.cpp
  *  \brief Test for server::host::HostPlayer
  */

#include "server/host/hostplayer.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/stringlistkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/files.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/host/cron.hpp"
#include "server/host/game.hpp"
#include "server/host/hostgame.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/mailqueueclient.hpp"
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
    hostFile.putFile("defaults/race.nm", afl::string::fromBytes(game::test::getDefaultRaceNames()));
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
AFL_TEST("server.host.HostPlayer:join", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    CronMock cron(a);
    h.root().setCron(&cron);
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    a.checkEqual("01. createNewGame", gid, 1);

    // Join users
    for (int i = 1; i <= 10; ++i) {
        cron.expectCall("handleGameChange(1)");
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
AFL_TEST("server.host.HostPlayer:join:error:admin", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    a.checkEqual("01. createNewGame", gid, 1);

    // Error: game does not exist
    AFL_CHECK_THROWS(a("11. wrong game"), testee.join(77, 1, "u1"), std::exception);

    // Error: slot does not exist
    AFL_CHECK_THROWS(a("21. wrong slot"), testee.join(gid, 99, "u1"), std::exception);

    // Error: user does not exist
    AFL_CHECK_THROWS(a("31. wrong user"), testee.join(gid, 1, "zz"), std::exception);

    // Error: slot already taken
    AFL_CHECK_SUCCEEDS(a("41. slot open"), testee.join(gid, 3, "u3"));
    AFL_CHECK_THROWS  (a("42. slot taken"), testee.join(gid, 3, "u4"), std::exception);

    // Not an error: you are already on this game - not detected if we're admin
    AFL_CHECK_SUCCEEDS(a("51. multi-join"), testee.join(gid, 4, "u3"));
}

/** Test join() failure cases, user access. */
AFL_TEST("server.host.HostPlayer:join:error:user", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    a.checkEqual("01. createNewGame", gid, 1);
    AFL_CHECK_SUCCEEDS(a("02. join"), testee.join(gid, 3, "u4"));

    // Set user context for all subsequent commands
    session.setUser("u3");

    // Error: game does not exist
    AFL_CHECK_THROWS(a("11. wrong game"), testee.join(77, 1, "u3"), std::exception);

    // Error: slot does not exist
    AFL_CHECK_THROWS(a("21. wrong slot"), testee.join(gid, 99, "u3"), std::exception);

    // Error: slot already taken
    AFL_CHECK_THROWS(a("31. slot taken"), testee.join(gid, 3, "u3"), std::exception);

    // Error: you cannot join someone else
    AFL_CHECK_THROWS(a("41. join other"), testee.join(gid, 3, "u4"), std::exception);

    // Error: you are already on this game
    AFL_CHECK_SUCCEEDS(a("51. join"), testee.join(gid, 1, "u3"));
    AFL_CHECK_THROWS(a("52. already joined"), testee.join(gid, 2, "u3"), std::exception);
}

/** Test resign(), normal cases. */
AFL_TEST("server.host.HostPlayer:resign", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    CronMock cron(a);
    h.root().setCron(&cron);
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    a.checkEqual("01. createNewGame", gid, 1);

    // Join some users
    cron.expectCall("handleGameChange(1)");
    testee.join(gid, 1, "u1");
    cron.expectCall("handleGameChange(1)");
    testee.join(gid, 2, "u2");
    cron.expectCall("handleGameChange(1)");
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
AFL_TEST("server.host.HostPlayer:resign:intermediate", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    a.checkEqual("01. createNewGame", gid, 1);

    // Join 4 users to one slot
    testee.join(gid, 1, "u1");
    testee.substitute(gid, 1, "u2");
    testee.substitute(gid, 1, "u3");
    testee.substitute(gid, 1, "u4");
    a.checkEqual("11. db", StringListKey(h.db(), "game:1:player:1:users").size(), 4);

    // Resign u3
    testee.resign(gid, 1, "u3");

    // u1,u2 remain
    a.checkEqual("21. db", StringListKey(h.db(), "game:1:player:1:users").size(), 2);
    a.checkEqual("22. db", StringListKey(h.db(), "game:1:player:1:users")[0], "u1");
    a.checkEqual("23. db", StringListKey(h.db(), "game:1:player:1:users")[1], "u2");
}

/** Test resign() combo, case 2.
    Resigning a primary player resigns the whole slot. */
AFL_TEST("server.host.HostPlayer:resign:primary", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    a.checkEqual("01. createNewGame", gid, 1);

    // Join 4 users to one slot
    testee.join(gid, 1, "u1");
    testee.substitute(gid, 1, "u2");
    testee.substitute(gid, 1, "u3");
    testee.substitute(gid, 1, "u4");
    a.checkEqual("11. db", StringListKey(h.db(), "game:1:player:1:users").size(), 4);

    // Resign u1
    testee.resign(gid, 1, "u1");

    // Nobody remains
    a.checkEqual("21. db", StringListKey(h.db(), "game:1:player:1:users").size(), 0);
}

/** Test resign() combo, permissions. */
AFL_TEST("server.host.HostPlayer:resign:permissions", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    a.checkEqual("01. createNewGame", gid, 1);

    // Join 5 users to one slot
    testee.join(gid, 1, "u1");
    testee.substitute(gid, 1, "u2");
    testee.substitute(gid, 1, "u3");
    testee.substitute(gid, 1, "u4");
    testee.substitute(gid, 1, "u5");
    a.checkEqual("11. db", StringListKey(h.db(), "game:1:player:1:users").size(), 5);

    // Set user u3
    session.setUser("u3");

    // Cannot resign primary or previous replacement, or users who are not playing
    AFL_CHECK_THROWS(a("21. resign"), testee.resign(gid, 1, "u1"), std::exception);
    AFL_CHECK_THROWS(a("22. resign"), testee.resign(gid, 1, "u2"), std::exception);
    AFL_CHECK_THROWS(a("23. resign"), testee.resign(gid, 1, "u6"), std::exception);

    // Can resign u5
    testee.resign(gid, 1, "u5");
    a.checkEqual("31. db", StringListKey(h.db(), "game:1:player:1:users").size(), 4);

    // Can resign ourselves and our replacement
    testee.resign(gid, 1, "u3");

    // u1,u2 remain
    a.checkEqual("41. resign", StringListKey(h.db(), "game:1:player:1:users").size(), 2);
    a.checkEqual("42. resign", StringListKey(h.db(), "game:1:player:1:users")[0], "u1");
    a.checkEqual("43. resign", StringListKey(h.db(), "game:1:player:1:users")[1], "u2");
}

/** Test substitute() behaviour. */
AFL_TEST("server.host.HostPlayer:substitute", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    a.checkEqual("01. createNewGame", gid, 1);

    // Join 5 users to one slot
    testee.join(gid, 1, "u1");
    testee.substitute(gid, 1, "u2");
    testee.substitute(gid, 1, "u3");
    testee.substitute(gid, 1, "u4");
    testee.substitute(gid, 1, "u5");
    a.checkEqual("11. db", StringListKey(h.db(), "game:1:player:1:users").size(), 5);

    // Substitute u3: this will drop everyone after u3
    testee.substitute(gid, 1, "u3");
    a.checkEqual("21. db", StringListKey(h.db(), "game:1:player:1:users").size(), 3);
    a.checkEqual("22. db", StringListKey(h.db(), "game:1:player:1:users")[2], "u3");

    // Substitute u4: will add
    testee.substitute(gid, 1, "u4");
    a.checkEqual("31. db", StringListKey(h.db(), "game:1:player:1:users").size(), 4);
    a.checkEqual("32. db", StringListKey(h.db(), "game:1:player:1:users")[3], "u4");
}

/** Test substitute() behaviour, user version. */
AFL_TEST("server.host.HostPlayer:substitute:user", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    a.checkEqual("01. createNewGame", gid, 1);

    // Join 5 users to one slot
    testee.join(gid, 1, "u1");
    testee.substitute(gid, 1, "u2");
    testee.substitute(gid, 1, "u3");
    testee.substitute(gid, 1, "u4");
    testee.substitute(gid, 1, "u5");
    a.checkEqual("11. db", StringListKey(h.db(), "game:1:player:1:users").size(), 5);

    // Set as user u3
    session.setUser("u3");

    // Try to substitute u2: not possible because they are before us
    AFL_CHECK_THROWS(a("21. substitute"), testee.substitute(gid, 1, "u2"), std::exception);

    // Try to substitute u4: ok, kicks u5
    AFL_CHECK_SUCCEEDS(a("31. substitute"), testee.substitute(gid, 1, "u4"));
    a.checkEqual("32. db", StringListKey(h.db(), "game:1:player:1:users").size(), 4);

    // Substitute u9: ok, replaces u5 by u9
    AFL_CHECK_SUCCEEDS(a("41. substitute"), testee.substitute(gid, 1, "u9"));
    a.checkEqual("42. db", StringListKey(h.db(), "game:1:player:1:users").size(), 4);
    a.checkEqual("43. db", StringListKey(h.db(), "game:1:player:1:users")[2], "u3");
    a.checkEqual("44. db", StringListKey(h.db(), "game:1:player:1:users")[3], "u9");

    // Substitute u3: kicks everyone up to u3
    AFL_CHECK_SUCCEEDS(a("51. substitute"), testee.substitute(gid, 1, "u3"));
    a.checkEqual("52. db", StringListKey(h.db(), "game:1:player:1:users").size(), 3);
}

/** Test substitute() behaviour, empty slot.
    This must fail. */
AFL_TEST("server.host.HostPlayer:substitute:empty", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    a.checkEqual("01. createNewGame", gid, 1);

    // Substitute into empty slot, fails
    AFL_CHECK_THROWS(a("11. substitute"), testee.substitute(gid, 2, "u2"), std::exception);
}

/** Test add(). */
AFL_TEST("server.host.HostPlayer:add", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();
    session.setUser("u3");

    // Create a private game
    int32_t gid = h.createNewGame(HostGame::PrivateGame, HostGame::Joining);
    a.checkEqual("01. createNewGame", gid, 1);

    // Game access initially not allowed to user
    AFL_CHECK_THROWS(a("11. getInfo"), server::host::HostGame(session, h.root()).getInfo(gid), std::exception);

    // Player cannot add themselves
    AFL_CHECK_THROWS(a("21. add"), testee.add(gid, "u3"), std::exception);

    // Add player to that game using admin permissions
    {
        server::host::Session adminSession;
        server::host::HostPlayer(adminSession, h.root()).add(gid, "u3");
    }

    // Game access now works
    AFL_CHECK_SUCCEEDS(a("31. getInfo"), server::host::HostGame(session, h.root()).getInfo(gid));
}

/** Get getInfo(), list(). */
AFL_TEST("server.host.HostPlayer:getInfo", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();
    h.addDefaultRaceNames();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    a.checkEqual("01. createNewGame", gid, 1);

    // Join some users
    testee.join(gid, 1, "u1");
    testee.substitute(gid, 1, "u2");
    testee.join(gid, 7, "u3");
    testee.join(gid, 11, "u4");

    // Get information about a slot
    {
        HostPlayer::Info i = testee.getInfo(gid, 1);
        a.checkEqual("11. longName",      i.longName, "The Solar Federation");
        a.checkEqual("12. shortName",     i.shortName, "The Feds");
        a.checkEqual("13. adjectiveName", i.adjectiveName, "Fed");
        a.checkEqual("14. userIds",       i.userIds.size(), 2U);
        a.checkEqual("15. userIds",       i.userIds[0], "u1");
        a.checkEqual("16. userIds",       i.userIds[1], "u2");
        a.checkEqual("17. numEditable",   i.numEditable, 2);
        a.checkEqual("18. joinable",      i.joinable, false);
    }
    {
        HostPlayer::Info i = testee.getInfo(gid, 7);
        a.checkEqual("19. userIds", i.userIds.size(), 1U);
        a.checkEqual("20. userIds", i.userIds[0], "u3");
    }
    {
        HostPlayer::Info i = testee.getInfo(gid, 9);
        a.checkEqual("21. userIds", i.userIds.size(), 0U);
        a.checkEqual("22. joinable", i.joinable, true);
    }

    // List
    // FIXME: test all=true vs all=false!
    {
        std::map<int,HostPlayer::Info> result;
        testee.list(gid, false, result);
        a.checkEqual("31. size", result.size(), 11U);
        for (int i = 1; i <= 11; ++i) {
            a.check("32. result", result.find(i) != result.end());
        }
        a.checkEqual("33. shortName 1", result[1].shortName, "The Feds");
        a.checkEqual("34. shortName 9", result[9].shortName, "The Robots");
    }
}

/** Test setDirectory(), getDirectory(). */
AFL_TEST("server.host.HostPlayer:setDirectory", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();
    h.userFile().createDirectoryAsUser("u4home", "u4");

    // Create a game and join a user
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    a.checkEqual("01. createNewGame", gid, 1);
    testee.join(gid, 3, "u4");

    // Directory name initially unset
    a.checkEqual("11. getDirectory", testee.getDirectory(gid, "u4"), "");

    // Set directory
    AFL_CHECK_SUCCEEDS(a("21. setDirectory"), testee.setDirectory(gid, "u4", "u4home/x/y"));

    // Query
    a.checkEqual("31. getDirectory", testee.getDirectory(gid, "u4"), "u4home/x/y");

    // Verify
    a.checkEqual("41. getFileInformation", h.userFile().getFileInformation("u4home/x/y").type, server::interface::FileBase::IsDirectory);
    a.checkEqual("42. getDirectoryIntegerProperty", h.userFile().getDirectoryIntegerProperty("u4home/x/y", "game"), gid);
}

/** Test setDirectory(), permission error case.
    Setting the directory to a non-writable area must fail, and not change the game config. */
AFL_TEST("server.host.HostPlayer:setDirectory:error:permissions", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();

    // Create a game and join a user
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    a.checkEqual("01. createNewGame", gid, 1);
    testee.join(gid, 3, "u4");

    // Set directory.
    // Fails because we didn't create the parent directory.
    AFL_CHECK_THROWS(a("11. setDirectory"), testee.setDirectory(gid, "u4", "u4home/x/y"), std::exception);

    // Query. Must still be empty.
    a.checkEqual("21. getDirectory", testee.getDirectory(gid, "u4"), "");
}

/** Test setDirectory(), user error case.
    Setting the directory for a different user is refused. */
AFL_TEST("server.host.HostPlayer:setDirectory:error:user", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();
    h.userFile().createDirectoryAsUser("u4home", "u4");
    h.userFile().createDirectoryAsUser("u1home", "u1");

    // Create a game and join a user
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    a.checkEqual("01. createNewGame", gid, 1);
    testee.join(gid, 3, "u4");

    // Set directory as user u1
    session.setUser("u1");
    AFL_CHECK_THROWS(a("11. setDirectory"), testee.setDirectory(gid, "u4", "u1home/x/y"), std::exception);
    AFL_CHECK_THROWS(a("12. setDirectory"), testee.setDirectory(gid, "u4", "u4home/x/y"), std::exception);

    // Query
    AFL_CHECK_THROWS(a("21. getDirectory"), testee.getDirectory(gid, "u4"), std::exception);

    // Query as admin, it didn't change
    session.setUser(String_t());
    a.checkEqual("31. getDirectory", testee.getDirectory(gid, "u4"), "");
}

/** Test setDirectory(), subscription error case.
    Setting the directory fails if you're not subscribed. */
AFL_TEST("server.host.HostPlayer:setDirectory:error:subscription", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();
    h.userFile().createDirectoryAsUser("u4home", "u4");

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    a.checkEqual("01. createNewGame", gid, 1);

    // Set directory, fails because we're not subscribed
    AFL_CHECK_THROWS(a("11. setDirectory"), testee.setDirectory(gid, "u4", "u4home/x/y"), std::exception);

    // Query, fails because we're not subscribed
    AFL_CHECK_THROWS(a("21. getDirectory"), testee.getDirectory(gid, "u4"), std::exception);
}

/** Test setDirectory(), error during directory change. */
AFL_TEST("server.host.HostPlayer:setDirectory:error:change", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();
    h.userFile().createDirectoryAsUser("u4home", "u4");

    // Create a game and join a user
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    a.checkEqual("01. createNewGame", gid, 1);
    testee.join(gid, 3, "u4");

    // Set directory, works
    AFL_CHECK_SUCCEEDS(a("11. setDirectory"), testee.setDirectory(gid, "u4", "u4home/x/y"));
    a.checkEqual("12. getDirectory", testee.getDirectory(gid, "u4"), "u4home/x/y");
    a.checkEqual("13. getDirectoryIntegerProperty", h.userFile().getDirectoryIntegerProperty("u4home/x/y", "game"), gid);

    // Move to different place, fails
    AFL_CHECK_THROWS(a("21. setDirectory"), testee.setDirectory(gid, "u4", "elsewhere/y"), std::exception);

    // Configuration unchanged
    a.checkEqual("31. getDirectory", testee.getDirectory(gid, "u4"), "u4home/x/y");
}

/** Test setDirectory(), conflict case.
    A game must refuse pointing its directory at the same place as another game. */
AFL_TEST("server.host.HostPlayer:setDirectory:error:conflict", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();
    h.userFile().createDirectoryAsUser("u4home", "u4");

    // Create two games and join a user
    int32_t gid1 = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    int32_t gid2 = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    a.checkEqual("01", gid1, 1);
    a.checkEqual("02", gid2, 2);
    testee.join(gid1, 3, "u4");
    testee.join(gid2, 4, "u4");

    // Set directory, works
    AFL_CHECK_SUCCEEDS(a("11. setDirectory"), testee.setDirectory(gid1, "u4", "u4home/x/y"));
    a.checkEqual("12. getDirectory", testee.getDirectory(gid1, "u4"), "u4home/x/y");
    a.checkEqual("13. getDirectoryIntegerProperty", h.userFile().getDirectoryIntegerProperty("u4home/x/y", "game"), gid1);

    // Set other game's directory the same as this one, must fail and leave the configuration unchanged
    AFL_CHECK_THROWS(a("21. setDirectory"), testee.setDirectory(gid2, "u4", "u4home/x/y"), std::exception);
    a.checkEqual("22. getDirectory", testee.getDirectory(gid1, "u4"), "u4home/x/y");
    a.checkEqual("23. getDirectory", testee.getDirectory(gid2, "u4"), "");
    a.checkEqual("24. getDirectoryIntegerProperty", h.userFile().getDirectoryIntegerProperty("u4home/x/y", "game"), gid1);
}

/** Test setDirectory(), move case. */
AFL_TEST("server.host.HostPlayer:setDirectory:move", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();
    h.userFile().createDirectoryAsUser("u4home", "u4");

    // Create a game and join a user
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    a.checkEqual("01. createNewGame", gid, 1);
    testee.join(gid, 3, "u4");

    // Set directory
    AFL_CHECK_SUCCEEDS(a("11. setDirectory"), testee.setDirectory(gid, "u4", "u4home/x/y"));
    a.checkEqual("12. getDirectoryIntegerProperty", h.userFile().getDirectoryIntegerProperty("u4home/x/y", "game"), gid);

    // Move
    AFL_CHECK_SUCCEEDS(a("21. setDirectory"), testee.setDirectory(gid, "u4", "u4home/a/b"));
    a.checkEqual("22. getDirectoryIntegerProperty", h.userFile().getDirectoryIntegerProperty("u4home/a/b", "game"), gid);
    a.checkEqual("23. getDirectoryIntegerProperty", h.userFile().getDirectoryIntegerProperty("u4home/x/y", "game"), 0);
}

/** Test checkFile(). */
AFL_TEST("server.host.HostPlayer:checkFile", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();
    h.userFile().createDirectoryAsUser("u3home", "u3");

    // Create a game and join two users
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
    a.checkEqual("01. createNewGame", gid, 1);
    testee.join(gid, 1, "u1");
    testee.join(gid, 3, "u3");
    testee.setDirectory(gid, "u3", "u3home/x");

    // Check with no directory name: Stale for 1 because they have not set a directory
    a.checkEqual("11", testee.checkFile(1, "u1", "xyplan.dat", afl::base::Nothing), HostPlayer::Stale);
    a.checkEqual("12", testee.checkFile(1, "u3", "xyplan.dat", afl::base::Nothing), HostPlayer::Refuse);
    a.checkEqual("13", testee.checkFile(1, "u1", "fizz.bin", afl::base::Nothing), HostPlayer::Stale);
    a.checkEqual("14", testee.checkFile(1, "u3", "fizz.bin", afl::base::Nothing), HostPlayer::Allow);

    // Check with wrong directory name
    a.checkEqual("21", testee.checkFile(1, "u1", "xyplan.dat", String_t("a")), HostPlayer::Stale);
    a.checkEqual("22", testee.checkFile(1, "u3", "xyplan.dat", String_t("a")), HostPlayer::Stale);
    a.checkEqual("23", testee.checkFile(1, "u1", "fizz.bin", String_t("a")), HostPlayer::Stale);
    a.checkEqual("24", testee.checkFile(1, "u3", "fizz.bin", String_t("a")), HostPlayer::Stale);

    // Check with correct directory name
    a.checkEqual("31", testee.checkFile(1, "u1", "xyplan.dat", String_t("u3home/x")), HostPlayer::Stale);
    a.checkEqual("32", testee.checkFile(1, "u3", "xyplan.dat", String_t("u3home/x")), HostPlayer::Refuse);
    a.checkEqual("33", testee.checkFile(1, "u1", "fizz.bin", String_t("u3home/x")), HostPlayer::Stale);
    a.checkEqual("34", testee.checkFile(1, "u3", "fizz.bin", String_t("u3home/x")), HostPlayer::Allow);

    // Turn files: must refuse turns that don't match the player
    a.checkEqual("41", testee.checkFile(1, "u1", "player1.trn", afl::base::Nothing), HostPlayer::Stale);
    a.checkEqual("42", testee.checkFile(1, "u1", "player3.trn", afl::base::Nothing), HostPlayer::Stale);
    a.checkEqual("43", testee.checkFile(1, "u3", "player1.trn", afl::base::Nothing), HostPlayer::Refuse);
    a.checkEqual("44", testee.checkFile(1, "u3", "player3.trn", afl::base::Nothing), HostPlayer::Turn);
    a.checkEqual("45", testee.checkFile(1, "u3", "player99.trn", afl::base::Nothing), HostPlayer::Refuse);
}

/** Test join/resign/substitute in wrong game state. */
AFL_TEST("server.host.HostPlayer:wrong-game-state", a)
{
    TestHarness h;
    server::host::Session session;
    server::host::HostPlayer testee(session, h.root());
    h.addUsers();

    // Create a game
    int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Preparing);

    // Operations fail
    AFL_CHECK_THROWS(a("01. join"),       testee.join(gid, 1, "u1"), std::exception);
    AFL_CHECK_THROWS(a("02. substitute"), testee.substitute(gid, 1, "u2"), std::exception);
    AFL_CHECK_THROWS(a("03. resign"),     testee.resign(gid, 1, "u2"), std::exception);

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
    AFL_CHECK_THROWS(a("11. join"),       testee.join(gid, 4, "u1"), std::exception);
    AFL_CHECK_THROWS(a("12. substitute"), testee.substitute(gid, 3, "u2"), std::exception);
    AFL_CHECK_THROWS(a("13. resign"),     testee.resign(gid, 1, "u2"), std::exception);
}

/** Test game settings. */
AFL_TEST("server.host.HostPlayer:settings", a)
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
    a.checkEqual("01. createNewGame", gid1, 1);
    a.checkEqual("02. createNewGame", gid2, 2);
    HostPlayer(rootSession, h.root()).join(gid1, 3, "u4");
    HostPlayer(rootSession, h.root()).join(gid2, 4, "u4");

    // Initial value: empty
    // - success cases: root, player themselves
    a.checkEqual("11. initial mailgametype", HostPlayer(rootSession, h.root()).get(gid1, "u4", "mailgametype"), "");
    a.checkEqual("12. initial mailgametype", HostPlayer(rootSession, h.root()).get(gid2, "u4", "mailgametype"), "");
    a.checkEqual("13. initial mailgametype", HostPlayer(userSession, h.root()).get(gid1, "u4", "mailgametype"), "");
    a.checkEqual("14. initial mailgametype", HostPlayer(userSession, h.root()).get(gid2, "u4", "mailgametype"), "");

    // - failure cases: different player, player not on game
    AFL_CHECK_THROWS(a("21. get wrong player"), HostPlayer(otherSession, h.root()).get(gid1, "u4", "mailgametype"), std::exception);
    AFL_CHECK_THROWS(a("22. get wrong player"), HostPlayer(rootSession, h.root()).get(gid1, "u77", "mailgametype"), std::exception);

    // Change it
    // - success cases: root, player themselves
    AFL_CHECK_SUCCEEDS(a("31. set mailgametype"), HostPlayer(userSession, h.root()).set(gid1, "u4", "mailgametype", "zip"));
    AFL_CHECK_SUCCEEDS(a("32. set mailgametype"), HostPlayer(rootSession, h.root()).set(gid2, "u4", "mailgametype", "rst"));

    // - failure cases: different player, player not on game
    AFL_CHECK_THROWS(a("41. set mailgametype"), HostPlayer(otherSession, h.root()).set(gid2, "u4",  "mailgametype", "info"), std::exception);
    AFL_CHECK_THROWS(a("42. set mailgametype"), HostPlayer(rootSession,  h.root()).set(gid2, "u77", "mailgametype", "info"), std::exception);

    // Verify
    a.checkEqual("51. mailgametype", HostPlayer(rootSession, h.root()).get(gid1, "u4", "mailgametype"), "zip");
    a.checkEqual("52. mailgametype", HostPlayer(rootSession, h.root()).get(gid2, "u4", "mailgametype"), "rst");
    a.checkEqual("53. mailgametype", HostPlayer(userSession, h.root()).get(gid1, "u4", "mailgametype"), "zip");
    a.checkEqual("54. mailgametype", HostPlayer(userSession, h.root()).get(gid2, "u4", "mailgametype"), "rst");
}

/** Test joining with profile permissions. */
AFL_TEST("server.host.HostPlayer:join:profile-permission", a)
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
        AFL_CHECK_SUCCEEDS(a("01. join"), HostPlayer(allowedSession, h.root()).join(gid, 1, "u1"));
    }
    // - u2 can not join
    {
        int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
        AFL_CHECK_THROWS(a("02. join disabled"), HostPlayer(forbiddenSession, h.root()).join(gid, 2, "u2"), std::exception);
    }
    // - u3 can join
    {
        int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
        AFL_CHECK_SUCCEEDS(a("03. join"), HostPlayer(defaultSession, h.root()).join(gid, 3, "u3"));
    }
    // - root can join anyone
    {
        int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
        AFL_CHECK_SUCCEEDS(a("04. join as admin"), HostPlayer(rootSession, h.root()).join(gid, 1, "u1"));
        AFL_CHECK_SUCCEEDS(a("05. join as admin"), HostPlayer(rootSession, h.root()).join(gid, 2, "u2"));
        AFL_CHECK_SUCCEEDS(a("06. join as admin"), HostPlayer(rootSession, h.root()).join(gid, 3, "u3"));
    }
}

/** Test join limit handling. */
AFL_TEST("server.host.HostPlayer:join-limit", a)
{
    using server::host::HostPlayer;
    using server::host::Game;

    TestHarness h;
    h.addUsers();

    // User
    HashKey(h.db(), "user:u3:profile").intField("rank").set(3);
    HashKey(h.db(), "user:u3:profile").intField("rankpoints").set(777);
    HashKey(h.db(), "user:u3:profile").intField("turnsplayed").set(77);

    server::host::Session userSession;
    userSession.setUser("u3");

    HostPlayer userInstance(userSession, h.root());

    // Base case
    {
        int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
        a.checkEqual("01. joinable", userInstance.getInfo(gid, 3).joinable, true);
        AFL_CHECK_SUCCEEDS(a("02. join"), userInstance.join(gid, 3, "u3"));
        a.checkEqual("03. userIds", userInstance.getInfo(gid, 3).userIds[0], "u3");
    }

    // Minimum level violated
    {
        int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
        Game(h.root(), gid).minRankLevelToJoin().set(4);
        a.checkEqual("11. joinable", userInstance.getInfo(gid, 3).joinable, false);
        AFL_CHECK_THROWS(a("12. join"), userInstance.join(gid, 3, "u3"), std::exception);
        a.check("13. userIds", userInstance.getInfo(gid, 3).userIds.empty());
    }

    // Maximum level violated
    {
        int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
        Game(h.root(), gid).maxRankLevelToJoin().set(2);
        a.checkEqual("21. joinable", userInstance.getInfo(gid, 3).joinable, false);
        AFL_CHECK_THROWS(a("22. join"), userInstance.join(gid, 3, "u3"), std::exception);
        a.check("23. userIds", userInstance.getInfo(gid, 3).userIds.empty());
    }

    // Minimum skill violated
    {
        int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
        Game(h.root(), gid).minRankPointsToJoin().set(10000);
        a.checkEqual("31. joinable", userInstance.getInfo(gid, 3).joinable, false);
        AFL_CHECK_THROWS(a("32. join"), userInstance.join(gid, 3, "u3"), std::exception);
        a.check("33. userIds", userInstance.getInfo(gid, 3).userIds.empty());
    }

    // Maximum skill violated
    {
        int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
        Game(h.root(), gid).maxRankPointsToJoin().set(500);
        a.checkEqual("41. joinable", userInstance.getInfo(gid, 3).joinable, false);
        AFL_CHECK_THROWS(a("42. join"), userInstance.join(gid, 3, "u3"), std::exception);
        a.check("43. userIds", userInstance.getInfo(gid, 3).userIds.empty());
    }

    // All tests pass
    {
        int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
        Game(h.root(), gid).minRankLevelToJoin().set(3);
        Game(h.root(), gid).maxRankLevelToJoin().set(3);
        Game(h.root(), gid).minRankPointsToJoin().set(777);
        Game(h.root(), gid).maxRankPointsToJoin().set(777);
        a.checkEqual("51. joinable", userInstance.getInfo(gid, 3).joinable, true);
        AFL_CHECK_SUCCEEDS(a("52. join"), userInstance.join(gid, 3, "u3"));
        a.checkEqual("53. userIds", userInstance.getInfo(gid, 3).userIds[0], "u3");
    }

    // All tests pass with margin
    {
        int32_t gid = h.createNewGame(HostGame::PublicGame, HostGame::Joining);
        Game(h.root(), gid).minRankLevelToJoin().set(1);
        Game(h.root(), gid).maxRankLevelToJoin().set(4);
        Game(h.root(), gid).minRankPointsToJoin().set(400);
        Game(h.root(), gid).maxRankPointsToJoin().set(900);
        a.checkEqual("61. joinable", userInstance.getInfo(gid, 3).joinable, true);
        AFL_CHECK_SUCCEEDS(a("62. join"), userInstance.join(gid, 3, "u3"));
        a.checkEqual("63. userIds", userInstance.getInfo(gid, 3).userIds[0], "u3");
    }
}
