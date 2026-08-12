/**
  *  \file test/server/host/talkadaptertest.cpp
  *  \brief Test for server::host::TalkAdapter
  */

#include "server/host/talkadapter.hpp"

#include "afl/container/ptrmap.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/string/format.hpp"
#include "afl/test/testrunner.hpp"
#include "server/host/game.hpp"
#include "server/host/root.hpp"
#include "server/host/user.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "util/processrunner.hpp"
#include <map>

using afl::net::redis::HashKey;
using afl::net::redis::IntegerSetKey;
using afl::net::redis::StringKey;
using afl::string::Format;
using server::host::Game;
using server::host::User;

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
        typedef std::map<String_t, String_t> DataMap_t;
        typedef std::set<String_t> WatchSet_t;

        TalkForumMock(afl::test::Assert a)
            : m_assert(a),
              m_forumCounter(0),
              m_forumData()
            { }

        const WatchSet_t& watches() const
            { return m_watches; }

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
        virtual void watchForum(int32_t fid, String_t userId)
            { m_watches.insert(Format("%d-%s", fid, userId)); }

     private:
        afl::test::Assert m_assert;
        int m_forumCounter;
        afl::container::PtrMap<int, DataMap_t> m_forumData;
        WatchSet_t m_watches;
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
    m_assert.checkNonNull("configure > forum", p);
    while (const String_t* pKey = config.eat()) {
        const String_t* pValue = config.eat();
        m_assert.checkNonNull("configure > value", pValue);
        (*p)[*pKey] = *pValue;
    }
}

afl::data::Value*
TalkForumMock::getValue(int32_t fid, String_t keyName)
{
    DataMap_t* p = m_forumData[fid];
    m_assert.checkNonNull("getValue > forum", p);

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
AFL_TEST("server.host.TalkAdapter:handleGameStart", a)
{
    TestHarness h;
    TalkForumMock m(a);

    // Create the game
    StringKey(h.db(), "game:19:name").set("The 2nd Game");
    IntegerSetKey(h.db(), "game:all").add(19);
    Game g(h.root(), 19);

    // Test
    server::host::TalkAdapter(m).handleGameStart(g, server::interface::HostGame::PublicGame);

    // Verify
    int32_t fid = HashKey(h.db(), "game:19:settings").intField("forum").get();
    a.checkDifferent("01. forum id", fid, 0);
    a.checkEqual("02. name",       m.getStringValue(fid, "name"), "The 2nd Game");
    a.checkEqual("03. newsgroup",  m.getStringValue(fid, "newsgroup"), "planetscentral.games.19-the-2nd-game");
    a.checkEqual("04. parent",     m.getStringValue(fid, "parent"), "active");
    a.checkEqual("05. key",        m.getStringValue(fid, "key"), "the 0012nd game");
    a.checkEqual("06. readperm",   m.getStringValue(fid, "readperm"), "all");
    a.checkEqual("07. writeperm",  m.getStringValue(fid, "writeperm"), "-u:anon,p:allowpost");
    a.checkEqual("08. answerperm", m.getStringValue(fid, "answerperm"), "-u:anon,p:allowpost");
}

/** Test handleGameStart, private game.
    This must create a private (access-controlled) forum. */
AFL_TEST("server.host.TalkAdapter:handleGameStart:private", a)
{
    TestHarness h;
    TalkForumMock m(a);

    // Create the game
    StringKey(h.db(), "game:27:name").set("Private Game");
    IntegerSetKey(h.db(), "game:all").add(27);
    Game g(h.root(), 27);

    // Test
    server::host::TalkAdapter(m).handleGameStart(g, server::interface::HostGame::PrivateGame);

    // Verify
    int32_t fid = HashKey(h.db(), "game:27:settings").intField("forum").get();
    a.checkDifferent("01. forum id", fid, 0);
    a.checkEqual("02. name",       m.getStringValue(fid, "name"), "Private Game");
    a.checkEqual("03. newsgroup",  m.getStringValue(fid, "newsgroup"), "planetscentral.games.27-private-game");
    a.checkEqual("04. parent",     m.getStringValue(fid, "parent"), "active-unlisted");
    a.checkEqual("05. key",        m.getStringValue(fid, "key"), "private game");
    a.checkEqual("06. readperm",   m.getStringValue(fid, "readperm"), "g:27");
    a.checkEqual("07. writeperm",  m.getStringValue(fid, "writeperm"), "g:27");
    a.checkEqual("08. answerperm", m.getStringValue(fid, "answerperm"), "g:27");
}

/** Test handleGameEnd, game has no forum.
    This should not mess with the forums. */
AFL_TEST("server.host.TalkAdapter:handleGameEnd:no-forum", a)
{
    TestHarness h;
    TalkForumMock m(a);

    // Create the game
    StringKey(h.db(), "game:9:name").set("Game");
    IntegerSetKey(h.db(), "game:all").add(9);
    Game g(h.root(), 9);

    // Test
    server::host::TalkAdapter(m).handleGameEnd(g, server::interface::HostGame::PublicGame);

    // Still no forum
    a.checkEqual("01. forum id", HashKey(h.db(), "game:9:settings").intField("forum").get(), 0);
}

/** Test handleGameEnd, normal case.
    This should move the forum from active* to finished*. */
AFL_TEST("server.host.TalkAdapter:handleGameEnd:normal", a)
{
    TestHarness h;
    TalkForumMock m(a);

    // Create the game
    StringKey(h.db(), "game:9:name").set("Game");
    IntegerSetKey(h.db(), "game:all").add(9);
    Game g(h.root(), 9);

    // Create and retire game
    server::host::TalkAdapter(m).handleGameStart(g, server::interface::HostGame::PrivateGame);
    server::host::TalkAdapter(m).handleGameEnd(g, server::interface::HostGame::PrivateGame);

    // Verify
    int32_t fid = HashKey(h.db(), "game:9:settings").intField("forum").get();
    a.checkDifferent("01. forum id", fid, 0);
    a.checkEqual("02. parent", m.getStringValue(fid, "parent"), "finished-unlisted");
}

/** Test handleGameEnd, forum has been moved.
    This must not move the forum. */
AFL_TEST("server.host.TalkAdapter:handleGameEnd:moved-forum", a)
{
    TestHarness h;
    TalkForumMock m(a);

    // Create the game
    StringKey(h.db(), "game:9:name").set("Game");
    IntegerSetKey(h.db(), "game:all").add(9);
    Game g(h.root(), 9);

    // Create game
    server::host::TalkAdapter(m).handleGameStart(g, server::interface::HostGame::PrivateGame);
    int32_t fid = HashKey(h.db(), "game:9:settings").intField("forum").get();
    a.checkDifferent("01. forum id", fid, 0);

    // Move forum
    String_t params[] = { "parent", "elsewhere" };
    m.configure(fid, params);

    // End game
    server::host::TalkAdapter(m).handleGameEnd(g, server::interface::HostGame::PrivateGame);

    // Verify
    a.checkEqual("11. forum id", HashKey(h.db(), "game:9:settings").intField("forum").get(), fid);
    a.checkEqual("12. parent", m.getStringValue(fid, "parent"), "elsewhere");
}

/** Test handleGameNameChange, game has no forum.
    This must not mess with the forums. */
AFL_TEST("server.host.TalkAdapter:handleGameNameChange:no-forum", a)
{
    TestHarness h;
    TalkForumMock m(a);

    // Create the game
    StringKey(h.db(), "game:9:name").set("Game");
    IntegerSetKey(h.db(), "game:all").add(9);
    Game g(h.root(), 9);

    // Test
    server::host::TalkAdapter(m).handleGameNameChange(g, "Game");

    // Still no forum
    a.checkEqual("01. forum id", HashKey(h.db(), "game:9:settings").intField("forum").get(), 0);
}

/** Test handleGameNameChange, normal case.
    This must rename the forum. */
AFL_TEST("server.host.TalkAdapter:handleGameNameChange:normal", a)
{
    TestHarness h;
    TalkForumMock m(a);

    // Create the game
    StringKey(h.db(), "game:3:name").set("Game");
    IntegerSetKey(h.db(), "game:all").add(3);
    Game g(h.root(), 3);

    // Create and verify
    server::host::TalkAdapter(m).handleGameStart(g, server::interface::HostGame::PublicGame);
    int32_t fid = HashKey(h.db(), "game:3:settings").intField("forum").get();
    a.checkDifferent("01. forum id", fid, 0);
    a.checkEqual("02. name",      m.getStringValue(fid, "name"), "Game");
    a.checkEqual("03. newsgroup", m.getStringValue(fid, "newsgroup"), "planetscentral.games.3-game");
    a.checkEqual("04. key",       m.getStringValue(fid, "key"), "game");

    // Rename
    String_t newName = "New Name";
    StringKey(h.db(), "game:3:name").set(newName);
    server::host::TalkAdapter(m).handleGameNameChange(g, newName);
    a.checkEqual("11. forum id", HashKey(h.db(), "game:3:settings").intField("forum").get(), fid);
    a.checkEqual("12. name",      m.getStringValue(fid, "name"), newName);
    a.checkEqual("13. newsgroup", m.getStringValue(fid, "newsgroup"), "planetscentral.games.3-game");  // unchanged! we don't rename newsgroups.
    a.checkEqual("14. key",       m.getStringValue(fid, "key"), "new name");
}

/** Test handleGameTypeChange, game has no forum.
    This must not mess with the forums. */
AFL_TEST("server.host.TalkAdapter:handleGameTypeChange:no-forum", a)
{
    TestHarness h;
    TalkForumMock m(a);

    // Create the game
    StringKey(h.db(), "game:9:name").set("Game");
    IntegerSetKey(h.db(), "game:all").add(9);
    Game g(h.root(), 9);

    // Test
    server::host::TalkAdapter(m).handleGameTypeChange(g, server::interface::HostGame::Joining, server::interface::HostGame::PublicGame);

    // Still no game
    a.checkEqual("01. forum id", HashKey(h.db(), "game:9:settings").intField("forum").get(), 0);
}

/** Test handleGameTypeChange, normal case.
    The forum must be moved into the correct category. */
AFL_TEST("server.host.TalkAdapter:handleGameTypeChange:normal", a)
{
    TestHarness h;
    TalkForumMock m(a);

    // Create the game
    StringKey(h.db(), "game:3:name").set("Game");
    IntegerSetKey(h.db(), "game:all").add(3);
    Game g(h.root(), 3);

    // Create and verify
    server::host::TalkAdapter(m).handleGameStart(g, server::interface::HostGame::PublicGame);
    int32_t fid = HashKey(h.db(), "game:3:settings").intField("forum").get();
    a.checkDifferent("01. forum id", fid, 0);
    a.checkEqual("02. parent",   m.getStringValue(fid, "parent"), "active");
    a.checkEqual("03. readperm", m.getStringValue(fid, "readperm"), "all");

    // Change type
    server::host::TalkAdapter(m).handleGameTypeChange(g, server::interface::HostGame::Joining, server::interface::HostGame::PrivateGame);
    a.checkEqual("11. forum id", HashKey(h.db(), "game:3:settings").intField("forum").get(), fid);
    a.checkEqual("12. parent",   m.getStringValue(fid, "parent"), "active-unlisted");
    a.checkEqual("13. readperm", m.getStringValue(fid, "readperm"), "g:3");
}

/** Test watchForum, normal case. */
AFL_TEST("server.host.TalkAdapter:handleGameJoin:normal", a)
{
    TestHarness h;
    TalkForumMock m(a);

    // Create the game
    StringKey(h.db(), "game:3:name").set("Game");
    IntegerSetKey(h.db(), "game:all").add(3);
    Game g(h.root(), 3);

    // Create forum
    server::host::TalkAdapter(m).handleGameStart(g, server::interface::HostGame::PublicGame);
    int32_t fid = HashKey(h.db(), "game:3:settings").intField("forum").get();
    a.checkEqual("01. forum id", fid, 1);

    // Join a player
    User u(h.root(), "fred");
    server::host::TalkAdapter(m).handleGameJoin(g, u);

    // Verify
    a.checkEqual("11. #subs", m.watches().size(), 1U);
    a.checkEqual("12. user", m.watches().count("1-fred"), 1U);
}

/** Test watchForum, disabled by user. */
AFL_TEST("server.host.TalkAdapter:handleGameJoin:disabled-by-user", a)
{
    TestHarness h;
    TalkForumMock m(a);

    // User profile
    HashKey(h.db(), "user:fred:profile").intField("joinautowatch").set(0);

    // Create the game
    StringKey(h.db(), "game:3:name").set("Game");
    IntegerSetKey(h.db(), "game:all").add(3);
    Game g(h.root(), 3);

    // Create forum
    server::host::TalkAdapter(m).handleGameStart(g, server::interface::HostGame::PublicGame);
    int32_t fid = HashKey(h.db(), "game:3:settings").intField("forum").get();
    a.checkEqual("01. forum id", fid, 1);

    // Join a player
    User u(h.root(), "fred");
    server::host::TalkAdapter(m).handleGameJoin(g, u);

    // Verify
    a.checkEqual("11. #subs", m.watches().size(), 0U);
}

/** Test watchForum, disabled in default profile. */
AFL_TEST("server.host.TalkAdapter:handleGameJoin:disabled-default", a)
{
    TestHarness h;
    TalkForumMock m(a);

    // User profile
    HashKey(h.db(), "default:profile").intField("joinautowatch").set(0);

    // Create the game
    StringKey(h.db(), "game:3:name").set("Game");
    IntegerSetKey(h.db(), "game:all").add(3);
    Game g(h.root(), 3);

    // Create forum
    server::host::TalkAdapter(m).handleGameStart(g, server::interface::HostGame::PublicGame);
    int32_t fid = HashKey(h.db(), "game:3:settings").intField("forum").get();
    a.checkEqual("01. forum id", fid, 1);

    // Join a player
    User u(h.root(), "fred");
    server::host::TalkAdapter(m).handleGameJoin(g, u);

    // Verify
    a.checkEqual("11. #subs", m.watches().size(), 0U);
}

/** Test watchForum, no forum. */
AFL_TEST("server.host.TalkAdapter:handleGameJoin:no-forum", a)
{
    TestHarness h;
    TalkForumMock m(a);

    // User profile
    HashKey(h.db(), "default:profile").intField("joinautowatch").set(0);

    // Create the game
    StringKey(h.db(), "game:3:name").set("Game");
    IntegerSetKey(h.db(), "game:all").add(3);
    Game g(h.root(), 3);

    // Join a player
    User u(h.root(), "fred");
    server::host::TalkAdapter(m).handleGameJoin(g, u);

    // Verify
    a.checkEqual("01. #subs", m.watches().size(), 0U);
}

/** Test watchForum, disabled by user. */
AFL_TEST("server.host.TalkAdapter:handleGameJoin:error:bad-config", a)
{
    TestHarness h;
    TalkForumMock m(a);

    // User profile
    HashKey(h.db(), "user:fred:profile").stringField("joinautowatch").set("boom");     // not an integer, will fail retrieval as integer

    // Create the game
    StringKey(h.db(), "game:3:name").set("Game");
    IntegerSetKey(h.db(), "game:all").add(3);
    Game g(h.root(), 3);

    // Create forum
    server::host::TalkAdapter(m).handleGameStart(g, server::interface::HostGame::PublicGame);
    int32_t fid = HashKey(h.db(), "game:3:settings").intField("forum").get();
    a.checkEqual("01. forum id", fid, 1);

    // Join a player
    User u(h.root(), "fred");
    server::host::TalkAdapter(m).handleGameJoin(g, u);

    // Verify
    a.checkEqual("11. #subs", m.watches().size(), 0U);
}
