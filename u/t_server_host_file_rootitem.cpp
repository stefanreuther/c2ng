/**
  *  \file u/t_server_host_file_rootitem.cpp
  *  \brief Test for server::host::file::RootItem
  */

#include <stdexcept>
#include "server/host/file/rootitem.hpp"

#include "t_server_host_file.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/host/gamecreator.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/mailqueueclient.hpp"

namespace {
    void createGame(server::host::Root& root)
    {
        // Create a game
        server::host::GameCreator crea(root);
        int32_t gameId = crea.createNewGame();
        crea.initializeGame(gameId);
        crea.finishNewGame(gameId, server::interface::HostGame::Finished, server::interface::HostGame::PublicGame);
        TS_ASSERT_EQUALS(gameId, 1);
    }

    void createTool(server::host::Root& root, server::host::Root::ToolTree t, String_t id)
    {
        t.all().add(id);
        t.byName(id).stringField("description").set("Description for " + id);
        t.byName(id).stringField("path").set(id + "dir");

        server::interface::FileBaseClient file(root.hostFile());
        file.createDirectory(id + "dir");
        file.putFile(id + "dir/file.txt", "content");
    }
}

/** Simple test. */
void
TestServerHostFileRootItem::testIt()
{
    // Build a Root
    afl::net::redis::InternalDatabase db;
    server::file::InternalFileServer hostFile;
    server::file::InternalFileServer userFile;
    afl::net::NullCommandHandler null;
    server::interface::MailQueueClient mailQueue(null);
    util::ProcessRunner checkturnRunner;
    afl::io::NullFileSystem fs;
    server::host::Root root(db, hostFile, userFile, mailQueue, checkturnRunner, fs, server::host::Configuration());

    // Create stuff
    createGame(root);
    createTool(root, root.shipListRoot(), "shipl");
    createTool(root, root.toolRoot(), "t");

    // Create testee
    server::host::Session session;
    server::host::file::RootItem testee(session, root);

    // Null functions
    server::host::file::Item::ItemVector_t vec;
    testee.listContent(vec);
    TS_ASSERT_EQUALS(vec.size(), 0U);
    TS_ASSERT_THROWS(testee.getContent(), std::runtime_error);
    TS_ASSERT_EQUALS(testee.getName(), testee.getInfo().name);

    // Access
    // - ship list
    std::auto_ptr<server::host::file::Item> p(testee.find("shiplist"));
    TS_ASSERT(p.get() != 0);
    TS_ASSERT_EQUALS(p->getName(), "shiplist");
    std::auto_ptr<server::host::file::Item> pp(p->find("shipl"));
    TS_ASSERT(pp.get() != 0);
    TS_ASSERT_EQUALS(pp->getName(), "shipl");

    // - tool
    p.reset(testee.find("tool"));
    TS_ASSERT(p.get() != 0);
    TS_ASSERT_EQUALS(p->getName(), "tool");
    pp.reset(p->find("t"));
    TS_ASSERT(pp.get() != 0);
    TS_ASSERT_EQUALS(pp->getName(), "t");

    // - game
    p.reset(testee.find("game"));
    TS_ASSERT(p.get() != 0);
    TS_ASSERT_EQUALS(p->getName(), "game");
    pp.reset(p->find("1"));
    TS_ASSERT(pp.get() != 0);
    TS_ASSERT_EQUALS(pp->getName(), "1");

    // - Other
    p.reset(testee.find("x"));
    TS_ASSERT(p.get() == 0);
}
