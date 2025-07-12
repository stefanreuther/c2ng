/**
  *  \file test/server/play/fs/sessiontest.cpp
  *  \brief Test for server::play::fs::Session
  */

#include "server/play/fs/session.hpp"
#include "afl/base/stoppable.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/net/internalnetworkstack.hpp"
#include "afl/net/name.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/resp/protocolhandler.hpp"
#include "afl/net/server.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/sys/thread.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/session.hpp"
#include "game/specificationloader.hpp"
#include "game/test/files.hpp"
#include "game/turn.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/interface/filebaseclient.hpp"

using afl::base::Enumerator;
using afl::base::Ptr;
using afl::base::Ref;
using afl::except::FileProblemException;
using afl::io::Directory;
using afl::io::DirectoryEntry;
using afl::io::FileSystem;
using afl::io::InternalDirectory;
using afl::io::Stream;
using afl::net::InternalNetworkStack;
using afl::net::Name;
using afl::net::NetworkStack;
using game::Root;
using server::interface::FileBaseClient;
using server::play::fs::Session;

namespace {
    /* Server mock.

       Caveat emptor:
       * this treats all connections as one session
       * no locking. Make sure that no background (network) accesses happen when client() is used. */
    class ServerMock : private afl::net::ProtocolHandlerFactory {
     public:
        ServerMock(NetworkStack& net, Name name)
            : m_filer(),
              m_server(net.listen(name, 10), *this),
              m_thread("ServerMock", m_server),
              m_client(m_filer)
            {
                m_thread.start();
            }
        ~ServerMock()
            {
                m_server.stop();
                m_thread.join();
            }

        virtual afl::net::ProtocolHandler* create()
            { return new afl::net::resp::ProtocolHandler(m_filer); }

        FileBaseClient& client()
            { return m_client; }

     private:
        server::file::InternalFileServer m_filer;
        afl::net::Server m_server;
        afl::sys::Thread m_thread;
        FileBaseClient m_client;
    };
}

/* Test Session::loadRoot().
   In particular, this tests the interaction between game::Session and util::ServerDirectory as game directory,
   namely, the flush()-after-save. */
AFL_TEST("server.play.fs.Session", a)
{
    // File server
    const Name netAddr("host", "port");
    Ref<InternalNetworkStack> net = InternalNetworkStack::create();
    ServerMock server(*net, netAddr);
    server.client().createDirectoryAsUser("dir", "fred");
    server.client().putFile("dir/beamspec.dat", afl::string::fromBytes(game::test::getDefaultBeams()));
    server.client().putFile("dir/engspec.dat",  afl::string::fromBytes(game::test::getDefaultEngines()));
    server.client().putFile("dir/hullspec.dat", afl::string::fromBytes(game::test::getDefaultHulls()));
    server.client().putFile("dir/planet.nm",    afl::string::fromBytes(game::test::getDefaultPlanetNames()));
    server.client().putFile("dir/player7.rst",  afl::string::fromBytes(game::test::getResultFile30()));
    server.client().putFile("dir/race.nm",      afl::string::fromBytes(game::test::getDefaultRaceNames()));
    server.client().putFile("dir/storm.nm",     afl::string::fromBytes(game::test::getDefaultIonStormNames()));
    server.client().putFile("dir/torpspec.dat", afl::string::fromBytes(game::test::getDefaultTorpedoes()));
    server.client().putFile("dir/truehull.dat", afl::string::fromBytes(game::test::getDefaultHullAssignments()));
    server.client().putFile("dir/xyplan.dat",   afl::string::fromBytes(game::test::getDefaultPlanetCoordinates()));

    // Environment
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    afl::io::NullFileSystem fs;
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    Ref<InternalDirectory> rootDir = InternalDirectory::create("root");

    // Do it
    Ref<Session> session = Session::create(*net, netAddr, "fred");
    Ptr<Root> root = session->createRoot("/dir", tx, log, fs, rootDir, cs);
    a.checkNonNull("01. root", root.get());
    a.checkNonNull("02. turnLoader", root->getTurnLoader().get());

    // Load game
    game::Session gs(tx, fs);
    gs.setRoot(root);
    gs.log().addListener(log);

    bool slOK = false;
    gs.setShipList(new game::spec::ShipList());
    root->specificationLoader().loadShipList(*gs.getShipList(), *root, game::makeResultTask(slOK))->call();
    a.check("11. loadShipList", slOK);

    bool turnOK = false;
    gs.setGame(new game::Game());
    root->getTurnLoader()->loadCurrentTurn(*gs.getGame(), 7, *root, gs, game::makeResultTask(turnOK))->call();
    a.check("21. loadCurrentTurn", turnOK);

    // Save again
    bool saveOK = false;
    gs.getGame()->currentTurn().setCommandPlayers(game::PlayerSet_t(7));
    gs.getGame()->setViewpointPlayer(7);
    gs.save(game::TurnLoader::SaveOptions_t(), game::makeResultTask(saveOK))->call();
    a.check("31. saveCurrentTurn", saveOK);
    a.checkDifferent("32. turn", server.client().getFile("dir/player7.trn"), "");
}
