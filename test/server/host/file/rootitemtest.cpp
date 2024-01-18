/**
  *  \file test/server/host/file/rootitemtest.cpp
  *  \brief Test for server::host::file::RootItem
  */

#include "server/host/file/rootitem.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/host/gamecreator.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/mailqueueclient.hpp"
#include <stdexcept>

namespace {
    void createGame(afl::test::Assert a, server::host::Root& root)
    {
        // Create a game
        server::host::GameCreator crea(root);
        int32_t gameId = crea.createNewGame();
        crea.initializeGame(gameId);
        crea.finishNewGame(gameId, server::interface::HostGame::Finished, server::interface::HostGame::PublicGame);
        a.checkEqual("createGame", gameId, 1);
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
AFL_TEST("server.host.file.RootItem", a)
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
    createGame(a, root);
    createTool(root, root.shipListRoot(), "shipl");
    createTool(root, root.toolRoot(), "t");

    // Create testee
    server::host::Session session;
    server::host::file::RootItem testee(session, root);

    // Null functions
    server::host::file::Item::ItemVector_t vec;
    testee.listContent(vec);
    a.checkEqual("01. size", vec.size(), 0U);
    AFL_CHECK_THROWS(a("02. getContent"), testee.getContent(), std::runtime_error);
    a.checkEqual("03. getName", testee.getName(), testee.getInfo().name);

    // Access
    // - ship list
    std::auto_ptr<server::host::file::Item> p(testee.find("shiplist"));
    a.checkNonNull("11. find", p.get());
    a.checkEqual("12. getName", p->getName(), "shiplist");
    std::auto_ptr<server::host::file::Item> pp(p->find("shipl"));
    a.checkNonNull("13. find", pp.get());
    a.checkEqual("14. getName", pp->getName(), "shipl");

    // - tool
    p.reset(testee.find("tool"));
    a.checkNonNull("21. find", p.get());
    a.checkEqual("22. getName", p->getName(), "tool");
    pp.reset(p->find("t"));
    a.checkNonNull("23. find", pp.get());
    a.checkEqual("24. getName", pp->getName(), "t");

    // - game
    p.reset(testee.find("game"));
    a.checkNonNull("31. find", p.get());
    a.checkEqual("32. getName", p->getName(), "game");
    pp.reset(p->find("1"));
    a.checkNonNull("33. find", pp.get());
    a.checkEqual("34. getName", pp->getName(), "1");

    // - Other
    p.reset(testee.find("x"));
    a.checkNull("41. find", p.get());
}
