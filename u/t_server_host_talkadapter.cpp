/**
  *  \file u/t_server_host_talkadapter.cpp
  *  \brief Test for server::host::TalkAdapter
  */

#include <map>
#include "server/host/talkadapter.hpp"

#include "t_server_host.hpp"
#include "afl/container/ptrmap.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "server/host/game.hpp"
#include "server/host/root.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "util/processrunner.hpp"

using afl::net::redis::StringKey;
using afl::net::redis::HashKey;
using afl::net::redis::IntegerSetKey;
using server::host::Game;

namespace {
    /** Test harness. Aggregates all our objects. */
    class TestHarness {
     public:
        TestHarness()
            : m_db(), m_null(), m_mail(m_null), m_runner(), m_fs(), m_root(m_db, m_null, m_null, m_mail, m_runner, m_fs, server::host::Configuration())
            { }

        server::host::Root& root()
            { return m_root; }

        afl::net::CommandHandler& db()
            { return m_db; }

     private:
        afl::net::redis::InternalDatabase m_db;
        afl::net::NullCommandHandler m_null;
        server::interface::MailQueueClient m_mail;
        util::ProcessRunner m_runner;
        afl::io::NullFileSystem m_fs;
        server::host::Root m_root;
    };

    /** TalkForum mock.
        Implements the add/configure/getValue operations required for TalkAdapter. */
    class TalkForumMock : public server::interface::TalkForum {
     public:
        TalkForumMock()
            : m_forumCounter(0),
              m_forumData()
            { }

        // TalkForum interface:
        virtual int32_t add(afl::base::Memory<const String_t> config);
        virtual void configure(int32_t fid, afl::base::Memory<const String_t> config);
        virtual afl::data::Value* getValue(int32_t fid, String_t keyName);
        virtual Info getInfo(int32_t /*fid*/)
            { throw std::runtime_error("unexpected"); }
        virtual void getInfo(afl::base::Memory<const int32_t> /*fids*/, afl::container::PtrVector<Info>& /*result*/)
            { throw std::runtime_error("unexpected"); }
        virtual int32_t getPermissions(int32_t /*fid*/, afl::base::Memory<const String_t> /*permissionList*/)
            { throw std::runtime_error("unexpected"); }
        virtual Size getSize(int32_t /*fid*/)
            { throw std::runtime_error("unexpected"); }
        virtual afl::data::Value* getThreads(int32_t /*fid*/, const ListParameters& /*params*/)
            { throw std::runtime_error("unexpected"); }
        virtual afl::data::Value* getStickyThreads(int32_t /*fid*/, const ListParameters& /*params*/)
            { throw std::runtime_error("unexpected"); }
        virtual afl::data::Value* getPosts(int32_t /*fid*/, const ListParameters& /*params*/)
            { throw std::runtime_error("unexpected"); }
        virtual int32_t findForum(String_t /*key*/)
            { throw std::runtime_error("unexpected"); }

     private:
        typedef std::map<String_t, String_t> DataMap_t;
        int m_forumCounter;
        afl::container::PtrMap<int, DataMap_t> m_forumData;
    };
}

int32_t
TalkForumMock::add(afl::base::Memory<const String_t> config)
{
    int32_t n = ++m_forumCounter;
    m_forumData.insertNew(n, new DataMap_t());
    configure(n, config);
    return n;
}

void
TalkForumMock::configure(int32_t fid, afl::base::Memory<const String_t> config)
{
    DataMap_t* p = m_forumData[fid];
    TS_ASSERT(p != 0);
    while (const String_t* pKey = config.eat()) {
        const String_t* pValue = config.eat();
        TS_ASSERT(pValue != 0);
        (*p)[*pKey] = *pValue;
    }
}

afl::data::Value*
TalkForumMock::getValue(int32_t fid, String_t keyName)
{
    DataMap_t* p = m_forumData[fid];
    TS_ASSERT(p != 0);

    DataMap_t::const_iterator it = p->find(keyName);
    if (it != p->end()) {
        return new afl::data::StringValue(it->second);
    } else {
        return 0;
    }
}

/********************************* Tests *********************************/

/** Test handleGameStart, standard case.
    This must create a public forum. */
void
TestServerHostTalkAdapter::testGameStart()
{
    TestHarness h;
    TalkForumMock m;

    // Create the game
    StringKey(h.db(), "game:19:name").set("The 2nd Game");
    IntegerSetKey(h.db(), "game:all").add(19);
    Game g(h.root(), 19);

    // Test
    server::host::TalkAdapter(m).handleGameStart(g, server::interface::HostGame::PublicGame);

    // Verify
    int32_t fid = HashKey(h.db(), "game:19:settings").intField("forum").get();
    TS_ASSERT_DIFFERS(fid, 0);
    TS_ASSERT_EQUALS(m.getStringValue(fid, "name"), "The 2nd Game");
    TS_ASSERT_EQUALS(m.getStringValue(fid, "newsgroup"), "planetscentral.games.19-the-2nd-game");
    TS_ASSERT_EQUALS(m.getStringValue(fid, "parent"), "active");
    TS_ASSERT_EQUALS(m.getStringValue(fid, "key"), "the 0012nd game");
    TS_ASSERT_EQUALS(m.getStringValue(fid, "readperm"), "all");
    TS_ASSERT_EQUALS(m.getStringValue(fid, "writeperm"), "-u:anon,p:allowpost");
    TS_ASSERT_EQUALS(m.getStringValue(fid, "answerperm"), "-u:anon,p:allowpost");
}

/** Test handleGameStart, private game.
    This must create a private (access-controlled) forum. */
void
TestServerHostTalkAdapter::testGameStartPrivate()
{
    TestHarness h;
    TalkForumMock m;

    // Create the game
    StringKey(h.db(), "game:27:name").set("Private Game");
    IntegerSetKey(h.db(), "game:all").add(27);
    Game g(h.root(), 27);

    // Test
    server::host::TalkAdapter(m).handleGameStart(g, server::interface::HostGame::PrivateGame);

    // Verify
    int32_t fid = HashKey(h.db(), "game:27:settings").intField("forum").get();
    TS_ASSERT_DIFFERS(fid, 0);
    TS_ASSERT_EQUALS(m.getStringValue(fid, "name"), "Private Game");
    TS_ASSERT_EQUALS(m.getStringValue(fid, "newsgroup"), "planetscentral.games.27-private-game");
    TS_ASSERT_EQUALS(m.getStringValue(fid, "parent"), "active-unlisted");
    TS_ASSERT_EQUALS(m.getStringValue(fid, "key"), "private game");
    TS_ASSERT_EQUALS(m.getStringValue(fid, "readperm"), "g:27");
    TS_ASSERT_EQUALS(m.getStringValue(fid, "writeperm"), "g:27");
    TS_ASSERT_EQUALS(m.getStringValue(fid, "answerperm"), "g:27");
}

/** Test handleGameEnd, game has no forum.
    This should not mess with the forums. */
void
TestServerHostTalkAdapter::testGameEndNoForum()
{
    TestHarness h;
    TalkForumMock m;

    // Create the game
    StringKey(h.db(), "game:9:name").set("Game");
    IntegerSetKey(h.db(), "game:all").add(9);
    Game g(h.root(), 9);

    // Test
    server::host::TalkAdapter(m).handleGameEnd(g, server::interface::HostGame::PublicGame);

    // Still no game
    TS_ASSERT_EQUALS(HashKey(h.db(), "game:9:settings").intField("forum").get(), 0);
}

/** Test handleGameEnd, normal case.
    This should move the forum from active* to finished*. */
void
TestServerHostTalkAdapter::testGameEndNormal()
{
    TestHarness h;
    TalkForumMock m;

    // Create the game
    StringKey(h.db(), "game:9:name").set("Game");
    IntegerSetKey(h.db(), "game:all").add(9);
    Game g(h.root(), 9);

    // Create and retire game
    server::host::TalkAdapter(m).handleGameStart(g, server::interface::HostGame::PrivateGame);
    server::host::TalkAdapter(m).handleGameEnd(g, server::interface::HostGame::PrivateGame);

    // Verify
    int32_t fid = HashKey(h.db(), "game:9:settings").intField("forum").get();
    TS_ASSERT_DIFFERS(fid, 0);
    TS_ASSERT_EQUALS(m.getStringValue(fid, "parent"), "finished-unlisted");
}

/** Test handleGameEnd, forum has been moved.
    This must not move the forum. */
void
TestServerHostTalkAdapter::testGameEndElsewhere()
{
    TestHarness h;
    TalkForumMock m;

    // Create the game
    StringKey(h.db(), "game:9:name").set("Game");
    IntegerSetKey(h.db(), "game:all").add(9);
    Game g(h.root(), 9);

    // Create game
    server::host::TalkAdapter(m).handleGameStart(g, server::interface::HostGame::PrivateGame);
    int32_t fid = HashKey(h.db(), "game:9:settings").intField("forum").get();
    TS_ASSERT_DIFFERS(fid, 0);

    // Move forum
    String_t params[] = { "parent", "elsewhere" };
    m.configure(fid, params);

    // End game
    server::host::TalkAdapter(m).handleGameEnd(g, server::interface::HostGame::PrivateGame);

    // Verify
    TS_ASSERT_EQUALS(HashKey(h.db(), "game:9:settings").intField("forum").get(), fid);
    TS_ASSERT_EQUALS(m.getStringValue(fid, "parent"), "elsewhere");
}

/** Test handleGameNameChange, game has no forum.
    This must not mess with the forums. */
void
TestServerHostTalkAdapter::testNameChangeNoForum()
{
    TestHarness h;
    TalkForumMock m;

    // Create the game
    StringKey(h.db(), "game:9:name").set("Game");
    IntegerSetKey(h.db(), "game:all").add(9);
    Game g(h.root(), 9);

    // Test
    server::host::TalkAdapter(m).handleGameNameChange(g, "Game");

    // Still no game
    TS_ASSERT_EQUALS(HashKey(h.db(), "game:9:settings").intField("forum").get(), 0);
}

/** Test handleGameNameChange, normal case.
    This must rename the forum. */
void
TestServerHostTalkAdapter::testNameChangeNormal()
{
    TestHarness h;
    TalkForumMock m;

    // Create the game
    StringKey(h.db(), "game:3:name").set("Game");
    IntegerSetKey(h.db(), "game:all").add(3);
    Game g(h.root(), 3);

    // Create and verify
    server::host::TalkAdapter(m).handleGameStart(g, server::interface::HostGame::PublicGame);
    int32_t fid = HashKey(h.db(), "game:3:settings").intField("forum").get();
    TS_ASSERT_DIFFERS(fid, 0);
    TS_ASSERT_EQUALS(m.getStringValue(fid, "name"), "Game");
    TS_ASSERT_EQUALS(m.getStringValue(fid, "newsgroup"), "planetscentral.games.3-game");
    TS_ASSERT_EQUALS(m.getStringValue(fid, "key"), "game");

    // Rename
    String_t newName = "New Name";
    StringKey(h.db(), "game:3:name").set(newName);
    server::host::TalkAdapter(m).handleGameNameChange(g, newName);
    TS_ASSERT_EQUALS(HashKey(h.db(), "game:3:settings").intField("forum").get(), fid);
    TS_ASSERT_EQUALS(m.getStringValue(fid, "name"), newName);
    TS_ASSERT_EQUALS(m.getStringValue(fid, "newsgroup"), "planetscentral.games.3-game");  // unchanged! we don't rename newsgroups.
    TS_ASSERT_EQUALS(m.getStringValue(fid, "key"), "new name");
}

/** Test handleGameTypeChange, game has no forum.
    This must not mess with the forums. */
void
TestServerHostTalkAdapter::testTypeChangeNoForum()
{
    TestHarness h;
    TalkForumMock m;

    // Create the game
    StringKey(h.db(), "game:9:name").set("Game");
    IntegerSetKey(h.db(), "game:all").add(9);
    Game g(h.root(), 9);

    // Test
    server::host::TalkAdapter(m).handleGameTypeChange(g, server::interface::HostGame::Joining, server::interface::HostGame::PublicGame);

    // Still no game
    TS_ASSERT_EQUALS(HashKey(h.db(), "game:9:settings").intField("forum").get(), 0);
}

/** Test handleGameTypeChange, normal case.
    The forum must be moved into the correct category. */
void
TestServerHostTalkAdapter::testTypeChangeNormal()
{
    TestHarness h;
    TalkForumMock m;

    // Create the game
    StringKey(h.db(), "game:3:name").set("Game");
    IntegerSetKey(h.db(), "game:all").add(3);
    Game g(h.root(), 3);

    // Create and verify
    server::host::TalkAdapter(m).handleGameStart(g, server::interface::HostGame::PublicGame);
    int32_t fid = HashKey(h.db(), "game:3:settings").intField("forum").get();
    TS_ASSERT_DIFFERS(fid, 0);
    TS_ASSERT_EQUALS(m.getStringValue(fid, "parent"), "active");
    TS_ASSERT_EQUALS(m.getStringValue(fid, "readperm"), "all");

    // Change type
    server::host::TalkAdapter(m).handleGameTypeChange(g, server::interface::HostGame::Joining, server::interface::HostGame::PrivateGame);
    TS_ASSERT_EQUALS(HashKey(h.db(), "game:3:settings").intField("forum").get(), fid);
    TS_ASSERT_EQUALS(m.getStringValue(fid, "parent"), "active-unlisted");
    TS_ASSERT_EQUALS(m.getStringValue(fid, "readperm"), "g:3");
}

