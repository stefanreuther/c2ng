/**
  *  \file u/t_server_host_hostspecificationimpl.cpp
  *  \brief Test for server::host::HostSpecificationImpl
  */

#include "server/host/hostspecificationimpl.hpp"

#include "t_server_host.hpp"
#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/host/hostgame.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "server/interface/hostspecificationclient.hpp"
#include "server/interface/hostspecificationserver.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "util/processrunner.hpp"

using afl::data::Access;
using afl::data::Hash;
using afl::net::redis::HashKey;
using afl::net::redis::StringKey;
using afl::net::redis::StringSetKey;
using server::Value_t;
using server::host::HostGame;

namespace {
    class PublisherMock : public afl::test::CallReceiver,
                          public server::host::spec::Publisher
    {
     public:
        PublisherMock(afl::test::Assert a)
            : CallReceiver(a),
              Publisher()
            { }

        virtual afl::data::Hash::Ref_t getSpecificationData(String_t pathName, String_t flakPath, const afl::data::StringList_t& /*keys*/)
            {
                checkCall("getSpecificationData(" + pathName + "," + flakPath + ")");
                return consumeReturnValue<Hash::Ref_t>();
            }
    };


    class Environment {
     public:
        Environment()
            : m_db(), m_hostFile(), m_userFile(), m_null(), m_mail(m_null), m_runner(), m_fs(),
              m_root(m_db, m_hostFile, m_userFile, m_mail, m_runner, m_fs, server::host::Configuration())
            { }

        server::host::Root& root()
            { return m_root; }

        afl::net::CommandHandler& db()
            { return m_db; }

        void addDefaultShipList();
        void addDefaultPrograms();
        void addFlakTool(String_t name);

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

    void Environment::addDefaultShipList()
    {
        HashKey(m_db, "prog:sl:prog:S").stringField("kind").set("shiplist");
        HashKey(m_db, "prog:sl:prog:S").stringField("path").set("path/to/S");
        StringKey(m_db, "prog:sl:default").set("S");
        StringSetKey(m_db, "prog:sl:list").add("S");
    }

    void Environment::addDefaultPrograms()
    {
        HashKey(m_db, "prog:host:prog:H").stringField("kind").set("host");
        HashKey(m_db, "prog:master:prog:M").stringField("kind").set("master");
        StringKey(m_db, "prog:host:default").set("H");
        StringKey(m_db, "prog:master:default").set("M");
        StringSetKey(m_db, "prog:host:list").add("H");
        StringSetKey(m_db, "prog:master:list").add("M");
    }

    void Environment::addFlakTool(String_t name)
    {
        HashKey(m_db, "prog:tool:prog:" + name).stringField("kind").set("combat");
        HashKey(m_db, "prog:tool:prog:" + name).stringField("path").set("flakpath");
        StringSetKey(m_db, "prog:tool:list").add(name);
    }

    Hash::Ref_t makeHash()
    {
        Hash::Ref_t hv = Hash::create();
        hv->setNew("a", server::makeIntegerValue(3));
        return hv;
    }

    afl::data::StringList_t makeKeys()
    {
        afl::data::StringList_t keys;
        keys.push_back("engspec");
        return keys;
    }
}

/** Test getShiplistData(), JSON result. */
void
TestServerHostHostSpecificationImpl::testGetShipListData()
{
    // Environment
    Environment env;
    env.addDefaultShipList();
    server::host::Session s;
    PublisherMock mock("testGetShipListData");

    // Expectation
    mock.expectCall("getSpecificationData(path/to/S,)");
    mock.provideReturnValue(makeHash());

    // Call
    std::auto_ptr<Value_t> result(server::host::HostSpecificationImpl(s, env.root(), mock).getShiplistData("S", server::interface::HostSpecification::JsonString, makeKeys()));

    // Verify result
    TS_ASSERT_EQUALS(Access(result.get()).toString(), "{\"a\":3}");
}

/** Test getShiplistData(), direct result. */
void
TestServerHostHostSpecificationImpl::testGetShipListDataDirect()
{
    // Environment
    Environment env;
    env.addDefaultShipList();
    server::host::Session s;
    PublisherMock mock("testGetShipListDataDirect");

    // Expectation
    mock.expectCall("getSpecificationData(path/to/S,)");
    mock.provideReturnValue(makeHash());

    // Call
    std::auto_ptr<Value_t> result(server::host::HostSpecificationImpl(s, env.root(), mock).getShiplistData("S", server::interface::HostSpecification::Direct, makeKeys()));

    // Verify result: direct means we can directly parse it with our means.
    TS_ASSERT_EQUALS(Access(result.get())("a").toInteger(), 3);
}

/** Test getShiplistData(), direct result, through protocol. */
void
TestServerHostHostSpecificationImpl::testGetShipListDataDirectProtocol()
{
    // Environment
    Environment env;
    env.addDefaultShipList();
    server::host::Session s;
    PublisherMock mock("testGetShipListDataDirectProtocol");

    // Expectation
    mock.expectCall("getSpecificationData(path/to/S,)");
    mock.provideReturnValue(makeHash());

    // Call
    server::host::HostSpecificationImpl impl(s, env.root(), mock);
    server::interface::HostSpecificationServer server(impl);
    server::interface::HostSpecificationClient client(server);
    std::auto_ptr<Value_t> result(client.getShiplistData("S", server::interface::HostSpecification::Direct, makeKeys()));

    // Verify result: The client/server protocol will mess up the types, but we still want to be able to directly parse it with our means.
    TS_ASSERT_EQUALS(Access(result.get())("a").toInteger(), 3);
}

/** Test getShiplistData(), direct result. */
void
TestServerHostHostSpecificationImpl::testGetShipListDataFlak()
{
    // Environment
    Environment env;
    env.addDefaultShipList();
    env.addFlakTool("flak");
    server::host::Session s;
    PublisherMock mock("testGetShipListDataFlak");

    // Expectation
    mock.expectCall("getSpecificationData(path/to/S,flakpath)");
    mock.provideReturnValue(makeHash());

    // Call
    afl::data::StringList_t list;
    list.push_back("flakconfig");
    std::auto_ptr<Value_t> result(server::host::HostSpecificationImpl(s, env.root(), mock).getShiplistData("S", server::interface::HostSpecification::Direct, list));

    // Verify result
    TS_ASSERT_EQUALS(Access(result.get())("a").toInteger(), 3);
}

/** Test getGameData(), new game. */
void
TestServerHostHostSpecificationImpl::testGetGameDataNew()
{
    // Environment
    Environment env;
    env.addDefaultShipList();
    env.addDefaultPrograms();
    server::host::Session s;

    // Create game
    HostGame g(s, env.root());
    int32_t gid = g.createNewGame();
    g.setState(gid, HostGame::Joining);
    g.setType(gid, HostGame::PublicGame);
    TS_ASSERT_EQUALS(gid, 1);

    // Expectation
    PublisherMock mock("testGetGameDataNew");
    mock.expectCall("getSpecificationData(path/to/S,)");
    mock.provideReturnValue(makeHash());

    // Call
    std::auto_ptr<Value_t> result(server::host::HostSpecificationImpl(s, env.root(), mock).getGameData(gid, server::interface::HostSpecification::JsonString, makeKeys()));

    // Verify result
    TS_ASSERT_EQUALS(Access(result.get()).toString(), "{\"a\":3}");
}

/** Test getGameData(), new game, with FLAK. */
void
TestServerHostHostSpecificationImpl::testGetGameDataFlak()
{
    // Environment
    Environment env;
    env.addDefaultShipList();
    env.addDefaultPrograms();
    env.addFlakTool("flak-2.0");
    server::host::Session s;

    // Create game
    HostGame g(s, env.root());
    int32_t gid = g.createNewGame();
    g.setState(gid, HostGame::Joining);
    g.setType(gid, HostGame::PublicGame);
    g.addTool(gid, "flak-2.0");
    TS_ASSERT_EQUALS(gid, 1);

    // Expectation
    PublisherMock mock("testGetGameDataFlak");
    mock.expectCall("getSpecificationData(path/to/S,flakpath)");
    mock.provideReturnValue(makeHash());

    // Call
    std::auto_ptr<Value_t> result(server::host::HostSpecificationImpl(s, env.root(), mock).getGameData(gid, server::interface::HostSpecification::JsonString, makeKeys()));

    // Verify result
    TS_ASSERT_EQUALS(Access(result.get()).toString(), "{\"a\":3}");
}

/** Test getGameData(), mastered game. */
void
TestServerHostHostSpecificationImpl::testGetGameDataMasteredGame()
{
    // Environment
    Environment env;
    env.addDefaultShipList();
    env.addDefaultPrograms();
    server::host::Session s;

    // Create game
    HostGame g(s, env.root());
    int32_t gid = g.createNewGame();
    g.setState(gid, HostGame::Joining);
    g.setType(gid, HostGame::PublicGame);
    TS_ASSERT_EQUALS(gid, 1);

    // Master has run
    afl::data::StringList_t sl;
    sl.push_back("masterHasRun");
    sl.push_back("1");
    g.setConfig(gid, sl);

    // Expectation
    PublisherMock mock("testGetGameDataMasteredGame");
    mock.expectCall("getSpecificationData(games/0001/data,)");
    mock.provideReturnValue(makeHash());

    // Call
    std::auto_ptr<Value_t> result(server::host::HostSpecificationImpl(s, env.root(), mock).getGameData(gid, server::interface::HostSpecification::JsonString, makeKeys()));

    // Verify result
    TS_ASSERT_EQUALS(Access(result.get()).toString(), "{\"a\":3}");
}

/** Test error cases. */
void
TestServerHostHostSpecificationImpl::testError()
{
    // Environment
    Environment env;
    env.addDefaultShipList();
    env.addDefaultPrograms();
    server::host::Session s;
    PublisherMock mock("testError");

    server::host::HostSpecificationImpl t(s, env.root(), mock);

    // Bad game Id
    TS_ASSERT_THROWS(t.getGameData(77, server::interface::HostSpecification::JsonString, makeKeys()), std::runtime_error);
    TS_ASSERT_THROWS(t.getShiplistData("whatever", server::interface::HostSpecification::JsonString, makeKeys()), std::runtime_error);
}

