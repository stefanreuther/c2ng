/**
  *  \file u/t_server_host_resultsender.cpp
  *  \brief Test for server::host::ResultSender
  */

#include <set>
#include <map>
#include <memory>
#include "server/host/resultsender.hpp"

#include "t_server_host.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/string/format.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/host/game.hpp"
#include "server/host/gamecreator.hpp"
#include "server/host/root.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/mailqueue.hpp"
#include "util/processrunner.hpp"

using afl::net::redis::HashKey;
using afl::net::redis::StringKey;
using afl::net::redis::StringSetKey;
using afl::string::Format;
using server::host::Game;
using server::interface::FileBaseClient;

namespace {
    /*
     *  Mail Mock
     *
     *  This simulates a mail queue.
     *  It verifies the command sequence.
     *  It stashes away received messages.
     *  We mainly want to track which users recieve which attachments, so this is what we mainly track.
     */
    class MailMock : public server::interface::MailQueue {
     public:
        struct Message {
            String_t templateName;
            std::map<String_t, String_t> parameters;
            std::set<String_t> attachments;
            std::set<String_t> receivers;

            bool hasAttachment(String_t what) const
                { return attachments.find(what) != attachments.end(); }
        };
        MailMock()
            : m_current(), m_queue()
            { }
        // MailQueue methods:
        virtual void startMessage(String_t templateName, afl::base::Optional<String_t> uniqueId);
        virtual void addParameter(String_t parameterName, String_t value);
        virtual void addAttachment(String_t url);
        virtual void send(afl::base::Memory<const String_t> receivers);
        virtual void cancelMessage(String_t /*uniqueId*/)
            { }
        virtual void confirmAddress(String_t /*address*/, String_t /*key*/, afl::base::Optional<String_t> /*info*/)
            { TS_ASSERT(0); }
        virtual void requestAddress(String_t /*user*/)
            { TS_ASSERT(0); }
        virtual void runQueue()
            { TS_ASSERT(0); }
        virtual UserStatus getUserStatus(String_t /*user*/)
            { TS_ASSERT(0); return UserStatus(); }

        Message* extract(String_t receiver);
        bool empty() const;

     private:
        std::auto_ptr<Message> m_current;
        afl::container::PtrVector<Message> m_queue;
    };

    /*
     *  Main Test Harness
     */
    class TestHarness {
     public:
        TestHarness()
            : m_db(), m_hostFile(), m_userFile(), m_mailQueue(), m_runner(), m_fs(),
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

void
MailMock::startMessage(String_t templateName, afl::base::Optional<String_t> /*uniqueId*/)
{
    TS_ASSERT(m_current.get() == 0);
    m_current.reset(new Message());
    m_current->templateName = templateName;
}

void
MailMock::addParameter(String_t parameterName, String_t value)
{
    TS_ASSERT(m_current.get() != 0);
    TS_ASSERT(m_current->parameters.find(parameterName) == m_current->parameters.end());
    m_current->parameters.insert(std::make_pair(parameterName, value));
}

void
MailMock::addAttachment(String_t url)
{
    TS_ASSERT(m_current.get() != 0);
    m_current->attachments.insert(url);
}

void
MailMock::send(afl::base::Memory<const String_t> receivers)
{
    TS_ASSERT(m_current.get() != 0);
    while (const String_t* p = receivers.eat()) {
        m_current->receivers.insert(*p);
    }
    m_queue.pushBackNew(m_current.release());
}

MailMock::Message*
MailMock::extract(String_t receiver)
{
    for (size_t i = 0; i < m_queue.size(); ++i) {
        Message* p = m_queue[i];
        std::set<String_t>::iterator it = p->receivers.find(receiver);
        if (it != p->receivers.end()) {
            p->receivers.erase(it);
            return p;
        }
    }
    return 0;
}

bool
MailMock::empty() const
{
    for (size_t i = 0; i < m_queue.size(); ++i) {
        if (!m_queue[i]->receivers.empty()) {
            return false;
        }
    }
    return true;
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
void
TestServerHostResultSender::testSimple()
{
    TestHarness h;

    // Add a game and join a user to it
    int32_t gid = h.addGame();
    TS_ASSERT_EQUALS(gid, 1);
    h.addUser("q");
    Game g(h.root(), gid);
    g.pushPlayerSlot(5, "q", h.root());

    // Send results
    server::host::ResultSender(h.root(), g).sendAllResults();

    // Verify.
    MailMock::Message* p = h.mailQueue().extract("user:q");
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/player5.zip"));
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/all/playerfiles.zip"));

    TS_ASSERT(h.mailQueue().empty());
}

/** Test multiple users on a game. */
void
TestServerHostResultSender::testMulti()
{
    TestHarness h;

    // Add a game and join users to it (p1,p2 for privs, b for borg)
    int32_t gid = h.addGame();
    TS_ASSERT_EQUALS(gid, 1);
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
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/player5.zip"));
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/all/playerfiles.zip"));

    p = h.mailQueue().extract("user:p2");
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/player5.zip"));
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/all/playerfiles.zip"));

    p = h.mailQueue().extract("user:b");
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/6/player6.zip"));
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/all/playerfiles.zip"));

    TS_ASSERT(h.mailQueue().empty());
}

/** Test differently-configured users on a game. */
void
TestServerHostResultSender::testConfig()
{
    TestHarness h;

    // Add a game and join users to it (p1,p2 for privs, b for borg)
    int32_t gid = h.addGame();
    TS_ASSERT_EQUALS(gid, 1);
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
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/1/player1.zip"));
    TS_ASSERT_EQUALS(p->parameters["gameid"], "1");
    TS_ASSERT_EQUALS(p->parameters["gameurl"], "1-test-config");
    TS_ASSERT_EQUALS(p->attachments.size(), 1U);

    p = h.mailQueue().extract("user:b");
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/1/player1.rst"));
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/1/util1.dat"));
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/all/playerfiles.zip"));
    TS_ASSERT_EQUALS(p->attachments.size(), 3U);

    p = h.mailQueue().extract("user:c");
    TS_ASSERT(p->attachments.empty());
    TS_ASSERT_EQUALS(p->parameters["gameid"], "1");
    TS_ASSERT_EQUALS(p->parameters["gameurl"], "1-test-config");

    p = h.mailQueue().extract("user:d");
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/3/player3.rst"));
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/3/util3.dat"));
    TS_ASSERT_EQUALS(p->attachments.size(), 2U);

    TS_ASSERT(h.mailQueue().empty());
}

/** Test configuration using user profile.
    Default is zip (as we have seen in testSimple); use player's profile to configure it to "rst". */
void
TestServerHostResultSender::testProfile()
{
    TestHarness h;

    // Add a game and join a user to it
    int32_t gid = h.addGame();
    TS_ASSERT_EQUALS(gid, 1);
    h.addUser("q");
    Game g(h.root(), gid);
    g.pushPlayerSlot(5, "q", h.root());
    HashKey(h.db(), "user:q:profile").stringField("mailgametype").set("rst");

    // Send results
    server::host::ResultSender(h.root(), g).sendAllResults();

    // Verify.
    MailMock::Message* p = h.mailQueue().extract("user:q");
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/player5.rst"));
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/util5.dat"));
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/all/playerfiles.zip"));

    TS_ASSERT(h.mailQueue().empty());
}

/** Test configuration using user profile.
    Same thing; use default profile instead. */
void
TestServerHostResultSender::testDefaultProfile()
{
    TestHarness h;

    // Add a game and join a user to it
    int32_t gid = h.addGame();
    TS_ASSERT_EQUALS(gid, 1);
    h.addUser("q");
    Game g(h.root(), gid);
    g.pushPlayerSlot(5, "q", h.root());
    HashKey(h.db(), "default:profile").stringField("mailgametype").set("rst");

    // Send results
    server::host::ResultSender(h.root(), g).sendAllResults();

    // Verify.
    MailMock::Message* p = h.mailQueue().extract("user:q");
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/player5.rst"));
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/util5.dat"));
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/all/playerfiles.zip"));

    TS_ASSERT(h.mailQueue().empty());
}

/** Test configuration using user profile.
    Same thing; use default profile AND user profile instead. */
void
TestServerHostResultSender::testProfileOverride()
{
    TestHarness h;

    // Add a game and join a user to it
    int32_t gid = h.addGame();
    TS_ASSERT_EQUALS(gid, 1);
    h.addUser("q");
    Game g(h.root(), gid);
    g.pushPlayerSlot(5, "q", h.root());
    HashKey(h.db(), "user:q:profile").stringField("mailgametype").set("rst");
    HashKey(h.db(), "default:profile").stringField("mailgametype").set("info");

    // Send results
    server::host::ResultSender(h.root(), g).sendAllResults();

    // Verify.
    MailMock::Message* p = h.mailQueue().extract("user:q");
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/player5.rst"));
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/util5.dat"));
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/all/playerfiles.zip"));

    TS_ASSERT(h.mailQueue().empty());
}

/** Test configuration using game override.
    Same thing; use default profile, user profile AND game override. */
void
TestServerHostResultSender::testGameOverride()
{
    TestHarness h;

    // Add a game and join a user to it
    int32_t gid = h.addGame();
    TS_ASSERT_EQUALS(gid, 1);
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
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/player5.rst"));
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/util5.dat"));
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/all/playerfiles.zip"));

    TS_ASSERT(h.mailQueue().empty());
}

/** Test configuration using game override explicitly set to "default".
    This means the user profile is to be used. */
void
TestServerHostResultSender::testGameDefault()
{
    TestHarness h;

    // Add a game and join a user to it
    int32_t gid = h.addGame();
    TS_ASSERT_EQUALS(gid, 1);
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
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/player5.rst"));
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/5/util5.dat"));
    TS_ASSERT(p->hasAttachment("c2file://127.0.0.1:7776/games/0001/out/all/playerfiles.zip"));

    TS_ASSERT(h.mailQueue().empty());
}

