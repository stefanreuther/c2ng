/**
  *  \file test/server/host/resultsendertest.cpp
  *  \brief Test for server::host::ResultSender
  */

#include "server/host/resultsender.hpp"

#include "afl/container/ptrvector.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/string/format.hpp"
#include "afl/test/testrunner.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/host/game.hpp"
#include "server/host/gamecreator.hpp"
#include "server/host/root.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/mailqueue.hpp"
#include "server/test/mailmock.hpp"
#include "util/processrunner.hpp"
#include <map>
#include <memory>
#include <set>

using afl::net::redis::HashKey;
using afl::net::redis::StringKey;
using afl::net::redis::StringSetKey;
using afl::string::Format;
using server::host::Game;
using server::interface::FileBaseClient;
using server::test::MailMock;

namespace {

    /*
     *  Main Test Harness
     */
    class TestHarness {
     public:
        TestHarness(afl::test::Assert a)
            : m_db(), m_hostFile(), m_userFile(), m_mailQueue(a), m_runner(), m_fs(),
              m_root(m_db, m_hostFile, m_userFile, m_mailQueue, m_runner, m_fs, server::host::Configuration())
            { }

        afl::net::CommandHandler& db()
            { return m_db; }

        afl::net::CommandHandler& hostFile()
            { return m_hostFile; }

        MailMock& mailQueue()
            { return m_mailQueue; }

        server::host::Root& root()
            { return m_root; }

        int32_t addGame();

        void addUser(String_t userId);

     private:
        afl::net::redis::InternalDatabase m_db;
        server::file::InternalFileServer m_hostFile;
        server::file::InternalFileServer m_userFile;
        MailMock m_mailQueue;
        util::ProcessRunner m_runner;
        afl::io::NullFileSystem m_fs;
        server::host::Root m_root;
    };
}


/*
 *  TestHarness
 */

int32_t
TestHarness::addGame()
{
    // Create game
    server::host::GameCreator maker(m_root);
    int32_t gid = maker.createNewGame();
    maker.initializeGame(gid);
    maker.finishNewGame(gid, server::interface::HostGame::Running, server::interface::HostGame::PublicGame);

    // Place default deliverable files in outbox
    FileBaseClient f(m_hostFile);
    for (int slot = 1; slot <= Game::NUM_PLAYERS; ++slot) {
        f.putFile(Format("games/%04d/out/%d/player%d.rst", gid, slot, slot), "rst...");
        f.putFile(Format("games/%04d/out/%d/player%d.zip", gid, slot, slot), "rst zip...");
        f.putFile(Format("games/%04d/out/%d/util%d.dat", gid, slot, slot), "util...");
    }
    f.putFile(Format("games/%04d/out/all/playerfiles.zip", gid), "playerfiles...");

    return gid;
}

void
TestHarness::addUser(String_t userId)
{
    StringSetKey(m_db, "user:all").add(userId);
    StringKey(m_db, "uid:" + userId).set(userId);
    HashKey(m_db, "user:" + userId + ":profile").stringField("email").set(userId + "@examp.le");
}


/*********************** TestServerHostResultSender **********************/

/** Test simple standard behaviour.
    Uninitialized database means send defaults. */
AFL_TEST("server.host.ResultSender:basic", a)
{
    TestHarness h(a);

    // Add a game and join a user to it
    int32_t gid = h.addGame();
    a.checkEqual("01. addGame", gid, 1);
    h.addUser("q");
    Game g(h.root(), gid);
    g.pushPlayerSlot(5, "q", h.root());

    // Send results
    server::host::ResultSender(h.root(), g).sendAllResults();

    // Verify.
    MailMock::Message* p = h.mailQueue().extract("user:q");
    a.check("11. zip",  p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/player5.zip"));
    a.check("12. pf",   p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/all/playerfiles.zip"));
    a.check("13. rst", !p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/player5.rst"));

    a.check("21. empty", h.mailQueue().empty());
}

/** Test multiple users on a game. */
AFL_TEST("server.host.ResultSender:multi", a)
{
    TestHarness h(a);

    // Add a game and join users to it (p1,p2 for privs, b for borg)
    int32_t gid = h.addGame();
    a.checkEqual("01. addGame", gid, 1);
    h.addUser("p1");
    h.addUser("p2");
    h.addUser("b");
    Game g(h.root(), gid);
    g.pushPlayerSlot(5, "p1", h.root());
    g.pushPlayerSlot(5, "p2", h.root());
    g.pushPlayerSlot(6, "b", h.root());

    // Send results
    server::host::ResultSender(h.root(), g).sendAllResults();

    // Verify.
    MailMock::Message* p = h.mailQueue().extract("user:p1");
    a.check("11. zip",  p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/player5.zip"));
    a.check("12. pf",   p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/all/playerfiles.zip"));
    a.check("13. rst", !p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/player5.rst"));

    p = h.mailQueue().extract("user:p2");
    a.check("21. zip",  p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/player5.zip"));
    a.check("22. pf",   p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/all/playerfiles.zip"));
    a.check("23. rst", !p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/player5.rst"));

    p = h.mailQueue().extract("user:b");
    a.check("31. zip", p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/6/player6.zip"));
    a.check("32. pf",   p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/all/playerfiles.zip"));
    a.check("33. rst", !p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/6/player6.rst"));

    a.check("41. empty", h.mailQueue().empty());
}

/** Test differently-configured users on a game. */
AFL_TEST("server.host.ResultSender:config", a)
{
    TestHarness h(a);

    // Add a game and join users to it (p1,p2 for privs, b for borg)
    int32_t gid = h.addGame();
    a.checkEqual("01. addGame", gid, 1);
    Game g(h.root(), gid);
    g.setName("test config", h.root().getForum());

    // User a: Fed, has player files, wants zipped results
    h.addUser("a");
    g.pushPlayerSlot(1, "a", h.root());
    g.setPlayerConfigInt("a", "hasPlayerFiles", 1);
    g.setPlayerConfig("a", "mailgametype", "zip");

    // User b: Also Fed, does not have player files, wants raw results
    h.addUser("b");
    g.pushPlayerSlot(1, "b", h.root());
    g.setPlayerConfig("b", "mailgametype", "rst");

    // User c: Lizard, wants just info
    h.addUser("c");
    g.pushPlayerSlot(2, "c", h.root());
    g.setPlayerConfig("c", "mailgametype", "info");

    // User d: Bird, has player files, wants result
    h.addUser("d");
    g.pushPlayerSlot(3, "d", h.root());
    g.setPlayerConfigInt("d", "hasPlayerFiles", 1);
    g.setPlayerConfig("d", "mailgametype", "rst");

    // Send results
    server::host::ResultSender(h.root(), g).sendAllResults();

    // Verify.
    MailMock::Message* p = h.mailQueue().extract("user:a");
    a.check("11. zip", p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/1/player1.zip"));
    a.check("12. rst", !p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/1/player1.rst"));
    a.checkEqual("13. gameid", p->parameters["gameid"], "1");
    a.checkEqual("14. gameurl", p->parameters["gameurl"], "1-test-config");
    a.checkEqual("15. att", p->attachments.size(), 1U);

    p = h.mailQueue().extract("user:b");
    a.check("21. rst", p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/1/player1.rst"));
    a.check("22. uti", p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/1/util1.dat"));
    a.check("23. pf",  p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/all/playerfiles.zip"));
    a.checkEqual("24. att", p->attachments.size(), 3U);

    p = h.mailQueue().extract("user:c");
    a.check("31. att", p->attachments.empty());
    a.checkEqual("32. gameid", p->parameters["gameid"], "1");
    a.checkEqual("33. gameurl", p->parameters["gameurl"], "1-test-config");

    p = h.mailQueue().extract("user:d");
    a.check("41. rst", p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/3/player3.rst"));
    a.check("42. uti", p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/3/util3.dat"));
    a.checkEqual("43. att", p->attachments.size(), 2U);

    a.check("51. empty", h.mailQueue().empty());
}

/** Test configuration using user profile.
    Default is zip (as we have seen in testSimple); use player's profile to configure it to "rst". */
AFL_TEST("server.host.ResultSender:config:profile", a)
{
    TestHarness h(a);

    // Add a game and join a user to it
    int32_t gid = h.addGame();
    a.checkEqual("01. addGame", gid, 1);
    h.addUser("q");
    Game g(h.root(), gid);
    g.pushPlayerSlot(5, "q", h.root());
    HashKey(h.db(), "user:q:profile").stringField("mailgametype").set("rst");

    // Send results
    server::host::ResultSender(h.root(), g).sendAllResults();

    // Verify.
    MailMock::Message* p = h.mailQueue().extract("user:q");
    a.check("11. rst", p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/player5.rst"));
    a.check("12. uti", p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/util5.dat"));
    a.check("13. pf",  p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/all/playerfiles.zip"));

    a.check("21. empty", h.mailQueue().empty());
}

/** Test configuration using user profile.
    Same thing; use default profile instead. */
AFL_TEST("server.host.ResultSender:config:default-profile", a)
{
    TestHarness h(a);

    // Add a game and join a user to it
    int32_t gid = h.addGame();
    a.checkEqual("01. addGame", gid, 1);
    h.addUser("q");
    Game g(h.root(), gid);
    g.pushPlayerSlot(5, "q", h.root());
    HashKey(h.db(), "default:profile").stringField("mailgametype").set("rst");

    // Send results
    server::host::ResultSender(h.root(), g).sendAllResults();

    // Verify.
    MailMock::Message* p = h.mailQueue().extract("user:q");
    a.check("11. rst", p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/player5.rst"));
    a.check("12. uti", p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/util5.dat"));
    a.check("13. pf",  p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/all/playerfiles.zip"));

    a.check("21. empty", h.mailQueue().empty());
}

/** Test configuration using user profile.
    Same thing; use default profile AND user profile instead. */
AFL_TEST("server.host.ResultSender:config:both-profiles", a)
{
    TestHarness h(a);

    // Add a game and join a user to it
    int32_t gid = h.addGame();
    a.checkEqual("01. addGame", gid, 1);
    h.addUser("q");
    Game g(h.root(), gid);
    g.pushPlayerSlot(5, "q", h.root());
    HashKey(h.db(), "user:q:profile").stringField("mailgametype").set("rst");
    HashKey(h.db(), "default:profile").stringField("mailgametype").set("info");

    // Send results
    server::host::ResultSender(h.root(), g).sendAllResults();

    // Verify.
    MailMock::Message* p = h.mailQueue().extract("user:q");
    a.check("11. rst", p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/player5.rst"));
    a.check("12. dat", p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/util5.dat"));
    a.check("13. pf",  p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/all/playerfiles.zip"));

    a.check("21. empty", h.mailQueue().empty());
}

/** Test configuration using game override.
    Same thing; use default profile, user profile AND game override. */
AFL_TEST("server.host.ResultSender:config:per-game", a)
{
    TestHarness h(a);

    // Add a game and join a user to it
    int32_t gid = h.addGame();
    a.checkEqual("01. addGame", gid, 1);
    h.addUser("q");
    Game g(h.root(), gid);
    g.pushPlayerSlot(5, "q", h.root());
    g.setPlayerConfig("q", "mailgametype", "rst");
    HashKey(h.db(), "user:q:profile").stringField("mailgametype").set("zip");
    HashKey(h.db(), "default:profile").stringField("mailgametype").set("info");

    // Send results
    server::host::ResultSender(h.root(), g).sendAllResults();

    // Verify.
    MailMock::Message* p = h.mailQueue().extract("user:q");
    a.check("11. rst", p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/player5.rst"));
    a.check("12. dat", p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/util5.dat"));
    a.check("13. pf",  p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/all/playerfiles.zip"));

    a.check("21. empty", h.mailQueue().empty());
}

/** Test configuration using game override explicitly set to "default".
    This means the user profile is to be used. */
AFL_TEST("server.host.ResultSender:config:game-default", a)
{
    TestHarness h(a);

    // Add a game and join a user to it
    int32_t gid = h.addGame();
    a.checkEqual("01. addGame", gid, 1);
    h.addUser("q");
    Game g(h.root(), gid);
    g.pushPlayerSlot(5, "q", h.root());
    g.setPlayerConfig("q", "mailgametype", "default");
    HashKey(h.db(), "user:q:profile").stringField("mailgametype").set("rst");
    HashKey(h.db(), "default:profile").stringField("mailgametype").set("zip");

    // Send results
    server::host::ResultSender(h.root(), g).sendAllResults();

    // Verify.
    MailMock::Message* p = h.mailQueue().extract("user:q");
    a.check("11. rst", p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/player5.rst"));
    a.check("12. dat", p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/util5.dat"));
    a.check("13. pf",  p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/all/playerfiles.zip"));

    a.check("21. empty", h.mailQueue().empty());
}

/** Test sending extra files.
    Mail in "rst" format must also include files that are not explicitly known to c2host. */
AFL_TEST("server.host.ResultSender:extra-files", a)
{
    TestHarness h(a);

    // Add a game and join a user to it
    int32_t gid = h.addGame();
    a.checkEqual("01. addGame", gid, 1);
    h.addUser("q");
    Game g(h.root(), gid);
    g.pushPlayerSlot(5, "q", h.root());
    HashKey(h.db(), "default:profile").stringField("mailgametype").set("rst");

    // Add extra files
    FileBaseClient(h.hostFile()).putFile("games/0001/out/5/flak5.dat", "flak...");
    FileBaseClient(h.hostFile()).putFile("games/0001/out/5/extra.txt", "extra");
    FileBaseClient(h.hostFile()).putFile("games/0001/out/5/x", "x");

    // Send results
    server::host::ResultSender(h.root(), g).sendAllResults();

    // Verify.
    MailMock::Message* p = h.mailQueue().extract("user:q");
    a.check("11. extra", p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/extra.txt"));
    a.check("12. flak",  p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/flak5.dat"));
    a.check("13. rst",   p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/player5.rst"));
    a.check("14. uti",   p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/util5.dat"));
    a.check("15. x",     p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/x"));
    a.check("16. pf",    p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/all/playerfiles.zip"));
    a.check("17. zip",  !p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/player5.zip"));

    a.check("21. empty", h.mailQueue().empty());
}
