/**
  *  \file test/game/pcc/gamefoldertest.cpp
  *  \brief Test for game::pcc::GameFolder
  */

#include "game/pcc/gamefolder.hpp"

#include "afl/io/internaldirectory.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/net/internalnetworkstack.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/browser/account.hpp"
#include "game/browser/session.hpp"
#include "game/browser/unsupportedaccountfolder.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/game.hpp"
#include "game/pcc/browserhandler.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/files.hpp"
#include "game/test/staticpage.hpp"
#include "game/test/webpage.hpp"
#include "game/test/webserver.hpp"
#include "game/turn.hpp"
#include "game/turnloader.hpp"
#include "util/profiledirectory.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::container::PtrVector;
using afl::io::FileSystem;
using afl::io::InternalDirectory;
using afl::io::InternalFileSystem;
using afl::net::InternalNetworkStack;
using afl::string::NullTranslator;
using afl::sys::InternalEnvironment;
using afl::sys::Log;
using game::Element;
using game::Root;
using game::browser::Account;
using game::browser::Folder;
using game::browser::LoadGameRootTask_t;
using game::browser::Session;
using game::browser::UnsupportedAccountFolder;
using game::config::HostConfiguration;
using game::config::UserConfiguration;
using game::pcc::BrowserHandler;
using game::pcc::GameFolder;
using game::test::StaticPage;
using game::test::WebPage;
using game::test::WebServer;
using util::ProfileDirectory;

namespace {
    class RootReceiver {
     public:
        RootReceiver()
            : m_result()
            { }

        void take(Ptr<Root> r)
            { m_result = r; }

        Ptr<Root> get() const
            { return m_result; }

     private:
        Ptr<Root> m_result;
    };

    InternalEnvironment& prepareEnvironment(InternalEnvironment& env)
    {
        env.setSettingsDirectoryName("/home");
        return env;
    }

    InternalFileSystem& prepareFileSystem(InternalFileSystem& fs)
    {
        fs.createDirectory("/home");
        return fs;
    }

    struct Environment {
        // Network
        Ref<InternalNetworkStack> networkStack;
        WebServer webServer;

        // Browser
        InternalFileSystem fs;
        InternalEnvironment env;
        NullTranslator tx;
        Log log;
        ProfileDirectory profile;
        Session session;

        // BrowserHandler
        Ref<InternalDirectory> specDir;
        BrowserHandler handler;

        // Account
        Ref<Account> acct;

        Environment()
            : networkStack(InternalNetworkStack::create()),
              webServer(*networkStack),
              fs(),
              env(),
              tx(),
              log(),
              profile(prepareEnvironment(env), prepareFileSystem(fs)),
              session(fs, tx, log, profile),
              specDir(InternalDirectory::create("spec")),
              handler(session.browser(), webServer.manager(), specDir, profile),
              acct(Account::create())
            {
                acct->setType("pcc");
                acct->setUser("id");
                acct->setHost("example.com");
                acct->setName("Account Name");
                acct->setEncoded("api_token", "key", true);
                acct->setEncoded("api_user", "id", true);

                static const WebPage::Response FILE_RESPONSE[] = {
                    { 0, 0, "api_token:key|dir:u/id|action:lsgame", 0,
                      "{\"reply\":["
                      "{\"conflict\":[],\"finished\":0,\"game\":17,\"hosttime\":0,\"hostversion\":\"PHost 4.1e\","
                      "\"missing\":[\"pconfig.src\"],\"name\":\"Game Name\",\"path\":\"u/id/one\",\"races\":{\"7\":\"The Crystal Confederation\"}},"
                      "{\"conflict\":[],\"finished\":0,\"game\":0,\"hosttime\":0,\"hostversion\":\"PHost 3.4l\",\"missing\":"
                      "[\"race.nm\",\"beamspec.dat\",\"engspec.dat\",\"hullspec.dat\",\"pconfig.src\",\"planet.nm\","
                      "\"torpspec.dat\",\"truehull.dat\",\"xyplan.dat\"],\"name\":\"\",\"path\":\"u/id/two\",\"races\":"
                      "{\"9\":\"The Robotic Imperium\"}}],\"result\":1}" },
                };
                webServer.addNewPage("example.com:443", "/api/file.cgi", new WebPage(FILE_RESPONSE));
            }
    };

    // Turn file matching getResultFile30(), renaming ship #32 to 'Renamed'
    const uint8_t TURN_FILE[] = {
        0x07, 0x00, 0x01, 0x00, 0x00, 0x00, 0x30, 0x32, 0x2d, 0x30, 0x32, 0x2d, 0x32, 0x30, 0x31, 0x36,
        0x32, 0x30, 0x3a, 0x34, 0x34, 0x3a, 0x30, 0x32, 0x00, 0x00, 0x87, 0x03, 0x00, 0x22, 0x00, 0x00,
        0x00, 0x07, 0x00, 0x20, 0x00, 0x52, 0x65, 0x6e, 0x61, 0x6d, 0x65, 0x64, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x60, 0x13, 0x00, 0x00, 0x43, 0x32, 0x4e,
        0x47, 0x5e, 0x04, 0x00, 0x00, 0x36, 0x07, 0x00, 0x00, 0xe7, 0x09, 0x00, 0x00, 0x80, 0x06, 0x00,
        0x00, 0x50, 0x14, 0x00, 0x00, 0xe8, 0x20, 0x00, 0x00, 0x7b, 0x22, 0x00, 0x00, 0xb0, 0x2c, 0x00,
        0x00, 0x29, 0x2e, 0x00, 0x00, 0xe8, 0x3a, 0x00, 0x00, 0x3d, 0x40, 0x00, 0x00, 0x80, 0x13, 0x00,
        0x00, 0xeb, 0x4b, 0x00, 0x00, 0xf0, 0x49, 0x00, 0x00, 0xe3, 0x49, 0x00, 0x00, 0xa0, 0x5c, 0x00,
        0x00, 0x31, 0x57, 0x00, 0x00, 0xc6, 0x6c, 0x00, 0x00, 0x97, 0x5d, 0x00, 0x00, 0xc8, 0x73, 0x00,
        0x00, 0xb5, 0x6b, 0x00, 0x00, 0xc0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00, 0x00, 0x27, 0x00,
        0x00, 0xa0, 0x28, 0x00, 0x00, 0x10, 0x04, 0x00, 0x00, 0xce, 0x06, 0x00, 0x00, 0x35, 0x0a, 0x00,
        0x00, 0x80, 0x06, 0x00, 0x00, 0x89, 0x12, 0x00, 0x00, 0x3e, 0x16, 0x00, 0x00, 0x60, 0x0b, 0x00,
        0x00, 0x00, 0x0d, 0x00, 0x00, 0xa0, 0x0e, 0x00, 0x00, 0x40, 0x10, 0x00, 0x00, 0xe0, 0x11, 0x00,
        0x00, 0x80, 0x13, 0x00, 0x00, 0x20, 0x15, 0x00, 0x00, 0xc0, 0x16, 0x00, 0x00, 0x60, 0x18, 0x00,
        0x00, 0x00, 0x1a, 0x00, 0x00, 0xa0, 0x1b, 0x00, 0x00, 0x40, 0x1d, 0x00, 0x00, 0xe0, 0x1e, 0x00,
        0x00, 0x80, 0x20, 0x00, 0x00, 0x20, 0x22, 0x00, 0x00, 0xc0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00,
        0x00, 0x00, 0x27, 0x00, 0x00, 0xa0, 0x28, 0x00, 0x00, 0x4b, 0x68, 0x07, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0xe8, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
}

// Test basic/simple operations
AFL_TEST("game.pcc.GameFolder:basics", a)
{
    Environment env;
    GameFolder testee(env.handler, env.acct, "u/id/one", 0);

    Ref<UserConfiguration> ruc = UserConfiguration::create();
    UserConfiguration& uc = *ruc;

    // Accessors
    a.check         ("01. canEnter",              !testee.canEnter());
    a.checkEqual    ("02. getName",                testee.getName(), "Game Name (#17)");
    a.checkDifferent("03. getText",                testee.getDescription().getText(), "");
    a.check         ("04. setLocalDirectoryName", !testee.setLocalDirectoryName("foo"));
    a.check         ("05. loadConfiguration",     !testee.loadConfiguration(uc));
    a.checkEqual    ("06. getKind",                testee.getKind(), Folder::kGame);

    // loadContent
    PtrVector<Folder> content;
    AFL_CHECK_SUCCEEDS(a("11. loadContent"), testee.loadContent(content));
    a.check("12. content", content.empty());

    // Dummies
    AFL_CHECK_SUCCEEDS(a("21. saveConfiguration"), testee.saveConfiguration(uc));

    a.check("31. isSame", testee.isSame(testee));

    GameFolder other(env.handler, Account::create(), "u/id/one", 0);
    a.check("32. isSame", !testee.isSame(other));

    UnsupportedAccountFolder other2(env.tx, env.acct);
    a.check("33. isSame", !testee.isSame(other2));
}

// Variation of getName() test, no name given
AFL_TEST("game.pcc.GameFolder:basics:2", a)
{
    Environment env;
    GameFolder testee(env.handler, env.acct, "u/id/two", 1);
    a.checkEqual("02. getName", testee.getName(), "id/two");
}

// Variation of getName() test, using bad hint
AFL_TEST("game.pcc.GameFolder:bad-hint", a)
{
    Environment env;
    GameFolder testee(env.handler, env.acct, "u/id/one", 99);
    a.checkEqual("02. getName", testee.getName(), "Game Name (#17)");
}

// Test loadGameRoot() + TurnLoader
AFL_TEST("game.pcc.GameFolder:loadGameRoot", a)
{
    // Some files in specDir, some on server, to exercise that both are used
    Environment env;
    env.specDir->openFile("race.nm", FileSystem::Create)
        ->fullWrite(game::test::getDefaultRaceNames());
    env.specDir->openFile("storm.nm", FileSystem::Create)
        ->fullWrite(game::test::getDefaultIonStormNames());

    // Respond to both 'lsgame' and 'ls' commands
    static const WebPage::Response FILE_RESPONSE[] = {
        { 0, 0, "api_token:key|dir:u/id|action:lsgame", 0,
          "{\"reply\":["
          "{\"conflict\":[],\"finished\":0,\"game\":17,\"hosttime\":0,\"hostversion\":\"PHost 4.1e\","
          "\"missing\":[\"pconfig.src\"],\"name\":\"Game Name\",\"path\":\"u/id/one\",\"races\":{\"7\":\"The Crystal Confederation\"}}],\"result\":1}" },
        { 0, 0, "api_token:key|dir:u/id/one|action:ls", 0,
          "{\"reply\":["
          "{\"id\":\"71b31ba04b78aac743677556bb6fc8f1831a4c26\",\"name\":\"pconfig.src\",\"size\":13,\"type\":\"file\",\"url\":\"/file.cgi/id/dir/one/pconfig.src\"},"
          "{\"id\":\"71b31ba04b78aac743677556bb6fc8f1831a4c27\",\"name\":\"player7.rst\",\"size\":1300,\"type\":\"file\",\"url\":\"/file.cgi/id/dir/one/player7.rst\"},"
          "{\"id\":\"71b31ba04b78aac743677556bb6fc8f1831a4c28\",\"name\":\"player7.trn\",\"size\":313,\"type\":\"file\",\"url\":\"/file.cgi/id/dir/one/player7.trn\"},"
          "{\"id\":\"71b31ba04b78aac743677556bb6fc8f1831a4c29\",\"name\":\"xyplan.dat\",\"size\":130,\"type\":\"file\",\"url\":\"/file.cgi/id/dir/one/xyplan.dat\"},"
          "{\"id\":\"71b31ba04b78aac743677556bb6fc8f1831a4c2a\",\"name\":\"planet.nm\",\"size\":1300,\"type\":\"file\",\"url\":\"/file.cgi/id/dir/one/planet.nm\"}"
          "],\"result\":1}" }
    };

    // Report a pconfig.src file. This proves that the root is correctly loaded.
    static const WebPage::Response CONFIG_RESPONSE[] = {
        { "GET", 0, "api_token:key", 0, "% phost\ngamename = Loaded Name\n" }
    };
    env.webServer.reset();
    env.webServer.addNewPage("example.com:443", "/api/file.cgi", new WebPage(FILE_RESPONSE));
    env.webServer.addNewPage("example.com:443", "/file.cgi/id/dir/one/pconfig.src", new WebPage(CONFIG_RESPONSE));
    env.webServer.addNewPage("example.com:443", "/file.cgi/id/dir/one/player7.rst", new StaticPage("application/octet-stream", game::test::getResultFile30()));
    env.webServer.addNewPage("example.com:443", "/file.cgi/id/dir/one/player7.trn", new StaticPage("application/octet-stream", TURN_FILE));
    env.webServer.addNewPage("example.com:443", "/file.cgi/id/dir/one/xyplan.dat", new StaticPage("application/octet-stream", game::test::getDefaultPlanetCoordinates()));
    env.webServer.addNewPage("example.com:443", "/file.cgi/id/dir/one/planet.nm", new StaticPage("application/octet-stream", game::test::getDefaultPlanetNames()));

    // Actual test
    GameFolder testee(env.handler, env.acct, "u/id/one", 0);
    Ref<UserConfiguration> ruc = UserConfiguration::create();
    UserConfiguration& uc = *ruc;
    RootReceiver recv;
    testee.loadGameRoot(uc, std::auto_ptr<LoadGameRootTask_t>(LoadGameRootTask_t::makeBound(&recv, &RootReceiver::take)))
        ->call();

    a.checkNonNull("01. result", recv.get().get());
    a.checkEqual("02. name", recv.get()->hostConfiguration()[HostConfiguration::GameName](), "Loaded Name");

    // Verify the turn loader
    a.checkNonNull("11. turnLoader", recv.get()->getTurnLoader().get());
    {
        game::Session session(env.tx, env.fs);
        session.setRoot(recv.get());
        session.setShipList(new game::spec::ShipList());
        session.setGame(new game::Game());


        bool loadFlag = false;
        recv.get()->getTurnLoader()->loadCurrentTurn(*session.getGame(), 7, *recv.get(), session, game::makeResultTask(loadFlag))
            ->call();
        a.check("21. loaded", loadFlag);
        a.checkEqual("22. msg", session.getGame()->currentTurn().inbox().getNumMessages(), 7U);
        a.checkEqual("23. owner", session.getGame()->currentTurn().universe().planets().get(388)->getOwner().orElse(0), 7);
        a.checkEqual("24. fuel", session.getGame()->currentTurn().universe().planets().get(388)->getCargo(Element::Neutronium).orElse(0), 2020);
        a.checkEqual("25. name", session.getGame()->currentTurn().universe().ships().get(32)->getName(), "Renamed");
    }
}
