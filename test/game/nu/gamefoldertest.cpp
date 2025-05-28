/**
  *  \file test/game/nu/gamefoldertest.cpp
  *  \brief Test for game::nu::GameFolder
  */

#include "game/nu/gamefolder.hpp"

#include "afl/io/filemapping.hpp"
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
#include "game/nu/browserhandler.hpp"
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
using game::Game;
using game::Root;
using game::Task_t;
using game::browser::Account;
using game::browser::Folder;
using game::browser::LoadGameRootTask_t;
using game::browser::Session;
using game::browser::UnsupportedAccountFolder;
using game::config::HostConfiguration;
using game::config::UserConfiguration;
using game::nu::BrowserHandler;
using game::nu::GameFolder;
using game::spec::ShipList;
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
              handler(session.browser(), webServer.manager(), specDir),
              acct(Account::create())
            {
                acct->setType("nu");
                acct->setUser("id");
                acct->setHost("example.com");
                acct->setName("Account Name");
                acct->setEncoded("api_key", "the_key", true);

                static const WebPage::Response LIST_RESPONSE[] = {
                    { 0, 0, "apikey:the_key", 0,
                      "{\"games\": ["
                      "{\"game\":{\"id\":11111,\"name\":\"First Game\",\"description\":\"First description\",\"slots\":11},\"player\":{\"id\":7,\"raceid\":7,\"username\":\"me\"}},"
                      "{\"game\":{\"id\":22222,\"name\":\"Second Game\",\"description\":\"Second description\",\"slots\":2},\"player\": {\"raceid\":7,\"id\":1,\"username\":\"me\"}}"
                      "],\"created\": [],\"success\": true}" },
                };
                webServer.addNewPage("api.example.com:443", "/account/mygames", new WebPage(LIST_RESPONSE));

                static const WebPage::Response LOAD_RESPONSE[] = {
                    { 0, 0, "apikey:the_key", 0,
                      "{\"success\":true,\"account\":{\"username\":\"J.User\"}}" },
                };
                webServer.addNewPage("api.example.com:443", "/account/load", new WebPage(LOAD_RESPONSE));
            }
    };

    void addTurnResponse(Environment& env)
    {
        static const WebPage::Response TURN_RESPONSE[] = {
            { 0, 0, "apikey:the_key|gameid:11111", 0,
              "{"
              "  \"success\": true,"
              "  \"rst\": {"
              "    \"settings\": {"
              "      \"hostcompleted\": \"4/12/2012 9:04:45 PM\""
              "    },"
              "    \"game\": {"
              "      \"turn\": 90"
              "    },"
              "    \"planets\": [],"
              "    \"ships\": [],"
              "    \"ionstorms\": [],"
              "    \"starbases\": [],"
              "    \"stock\": [],"
              "    \"minefields\": [],"
              "    \"vcrs\": []"
              "  }"
              "}" },
        };
        env.webServer.addNewPage("api.example.com:443", "/game/loadturn", new WebPage(TURN_RESPONSE));
    }
}

// Test basic/simple operations
AFL_TEST("game.nu.GameFolder:basics", a)
{
    Environment env;
    GameFolder testee(env.handler, env.acct, 11111, 0);

    UserConfiguration uc;

    // Accessors
    a.check     ("01. canEnter",              !testee.canEnter());
    a.checkEqual("02. getName",                testee.getName(), "First Game (11111)");
    a.checkEqual("03. getText",                testee.getDescription().getText(), "First description");
    a.checkEqual("04. getKind",                testee.getKind(), Folder::kGame);

    // loadContent
    PtrVector<Folder> content;
    AFL_CHECK_SUCCEEDS(a("11. loadContent"), testee.loadContent(content));
    a.check("12. content", content.empty());

    // isSame
    // - compare to self
    a.check("31. isSame", testee.isSame(testee));

    // - different account, different Id
    GameFolder other(env.handler, Account::create(), 22222, 1);
    a.check("32. isSame", !testee.isSame(other));

    // - different account, same Id
    GameFolder other2(env.handler, Account::create(), 11111, 1);
    a.check("33. isSame", !testee.isSame(other2));

    // - same account, different Id
    GameFolder other3(env.handler, env.acct, 22222, 1);
    a.check("34. isSame", !testee.isSame(other3));

    // - same account, same Id
    GameFolder clone(env.handler, env.acct, 11111, 1);
    a.check("35. isSame", testee.isSame(clone));

    // - type other
    UnsupportedAccountFolder other4(env.tx, env.acct);
    a.check("36. isSame", !testee.isSame(other4));
}

// Test basic/simple operations
AFL_TEST("game.nu.GameFolder:config", a)
{
    Environment env;
    GameFolder testee(env.handler, env.acct, 11111, 0);

    env.fs.createDirectory("/gameDir");
    env.fs.openFile("/gameDir/pcc2.ini", FileSystem::Create)
        ->fullWrite(afl::string::toBytes("Export.ShipFields=Name,Hull,Id\n"));

    UserConfiguration uc;

    // Configure directory and load
    a.check("01. setLocalDirectoryName", testee.setLocalDirectoryName("/gameDir"));
    a.check("02. loadConfiguration",     testee.loadConfiguration(uc));

    // Verify content
    a.checkEqual("11. content", uc[UserConfiguration::ExportShipFields](), "Name,Hull,Id");
    a.checkEqual("12. content", uc[UserConfiguration::Game_Host](), "example.com");

    // Save
    AFL_CHECK_SUCCEEDS(a("21. saveConfiguration"), testee.saveConfiguration(uc));

    String_t content = afl::string::fromBytes(env.fs.openFile("/gameDir/pcc2.ini", FileSystem::OpenRead)
                                              ->createVirtualMapping()->get());
    a.checkDifferent("22. content", content.find("Game.Host"), String_t::npos);
}

// Test loadGameRoot
AFL_TEST("game.nu.GameFolder:loadGameRoot", a)
{
    Environment env;
    GameFolder testee(env.handler, env.acct, 11111, 0);
    addTurnResponse(env);

    // Setup
    UserConfiguration config;
    RootReceiver recv;
    std::auto_ptr<LoadGameRootTask_t> in(LoadGameRootTask_t::makeBound(&recv, &RootReceiver::take));
    std::auto_ptr<Task_t> out(testee.loadGameRoot(config, in));
    a.checkNull("01. in", in.get());
    a.checkNonNull("02. out", out.get());

    // Do it
    out->call();
    a.checkNonNull("11. root", recv.get().get());
    a.check("12. actions", recv.get()->getPossibleActions().contains(Root::aLocalSetup));
    a.check("13. actions", !recv.get()->getPossibleActions().contains(Root::aLoadEditable));
    a.checkNonNull("14. turn", recv.get()->getTurnLoader().get());

    // Turn Loader
    {
        game::Session session(env.tx, env.fs);
        session.setShipList(new ShipList());
        session.setGame(new Game());
        session.setRoot(recv.get());

        bool loadFlag = false;
        recv.get()->getTurnLoader()->loadCurrentTurn(session.getGame()->currentTurn(), *session.getGame(), 7, *recv.get(), session, game::makeResultTask(loadFlag))
            ->call();
        a.check("21. loaded", loadFlag);
        a.checkEqual("22. turn", session.getGame()->currentTurn().getTurnNumber(), 90);
    }
}

// Test loadGameRoot, with local directory
AFL_TEST("game.nu.GameFolder:loadGameRoot:local", a)
{
    Environment env;
    GameFolder testee(env.handler, env.acct, 11111, 0);
    addTurnResponse(env);

    env.fs.createDirectory("/game");
    env.acct->setGameFolderName("11111", "/game");

    // Setup
    UserConfiguration config;
    RootReceiver recv;
    std::auto_ptr<LoadGameRootTask_t> in(LoadGameRootTask_t::makeBound(&recv, &RootReceiver::take));
    std::auto_ptr<Task_t> out(testee.loadGameRoot(config, in));
    a.checkNull("01. in", in.get());
    a.checkNonNull("02. out", out.get());

    // Do it
    out->call();
    a.checkNonNull("11. root", recv.get().get());
    a.check("12. actions", recv.get()->getPossibleActions().contains(Root::aLocalSetup));
    a.check("13. actions", recv.get()->getPossibleActions().contains(Root::aLoadEditable));
    a.checkNonNull("14. turn", recv.get()->getTurnLoader().get());

    // Turn Loader
    {
        game::Session session(env.tx, env.fs);
        session.setShipList(new ShipList());
        session.setGame(new Game());
        session.setRoot(recv.get());

        bool loadFlag = false;
        recv.get()->getTurnLoader()->loadCurrentTurn(session.getGame()->currentTurn(), *session.getGame(), 7, *recv.get(), session, game::makeResultTask(loadFlag))
            ->call();
        a.check("21. loaded", loadFlag);
        a.checkEqual("22. turn", session.getGame()->currentTurn().getTurnNumber(), 90);
    }
}

// Test loadGameRoot, with local directory
AFL_TEST("game.nu.GameFolder:loadGameRoot:local", a)
{
    Environment env;
    GameFolder testee(env.handler, env.acct, 11111, 0);
    addTurnResponse(env);

    env.fs.createDirectory("/game");
    env.acct->setGameFolderName("11111", "/game");

    // Setup
    UserConfiguration config;
    RootReceiver recv;
    std::auto_ptr<LoadGameRootTask_t> in(LoadGameRootTask_t::makeBound(&recv, &RootReceiver::take));
    std::auto_ptr<Task_t> out(testee.loadGameRoot(config, in));
    a.checkNull("01. in", in.get());
    a.checkNonNull("02. out", out.get());

    // Do it
    out->call();
    a.checkNonNull("11. root", recv.get().get());
    a.check("12. actions", recv.get()->getPossibleActions().contains(Root::aLocalSetup));
    a.check("13. actions", recv.get()->getPossibleActions().contains(Root::aLoadEditable));
    a.checkNonNull("14. turn", recv.get()->getTurnLoader().get());

    // Turn Loader
    {
        game::Session session(env.tx, env.fs);
        session.setShipList(new ShipList());
        session.setGame(new Game());
        session.setRoot(recv.get());

        bool loadFlag = false;
        recv.get()->getTurnLoader()->loadCurrentTurn(session.getGame()->currentTurn(), *session.getGame(), 7, *recv.get(), session, game::makeResultTask(loadFlag))
            ->call();
        a.check("21. loaded", loadFlag);
        a.checkEqual("22. turn", session.getGame()->currentTurn().getTurnNumber(), 90);
    }
}

// Test loadGameRoot, with local directory
AFL_TEST("game.nu.GameFolder:loadGameRoot:local", a)
{
    Environment env;
    GameFolder testee(env.handler, env.acct, 11111, 0);
    addTurnResponse(env);

    env.fs.createDirectory("/game");
    env.acct->setGameFolderName("11111", "/game");

    // Setup
    UserConfiguration config;
    RootReceiver recv;
    std::auto_ptr<LoadGameRootTask_t> in(LoadGameRootTask_t::makeBound(&recv, &RootReceiver::take));
    std::auto_ptr<Task_t> out(testee.loadGameRoot(config, in));
    a.checkNull("01. in", in.get());
    a.checkNonNull("02. out", out.get());

    // Do it
    out->call();
    a.checkNonNull("11. root", recv.get().get());
    a.check("12. actions", recv.get()->getPossibleActions().contains(Root::aLocalSetup));
    a.check("13. actions", recv.get()->getPossibleActions().contains(Root::aLoadEditable));
    a.checkNonNull("14. turn", recv.get()->getTurnLoader().get());

    // Turn Loader
    {
        game::Session session(env.tx, env.fs);
        session.setShipList(new ShipList());
        session.setGame(new Game());
        session.setRoot(recv.get());

        bool loadFlag = false;
        recv.get()->getTurnLoader()->loadCurrentTurn(session.getGame()->currentTurn(), *session.getGame(), 7, *recv.get(), session, game::makeResultTask(loadFlag))
            ->call();
        a.check("21. loaded", loadFlag);
        a.checkEqual("22. turn", session.getGame()->currentTurn().getTurnNumber(), 90);
    }
}

// Test loadGameRoot, local directory lost
AFL_TEST("game.nu.GameFolder:loadGameRoot:lost", a)
{
    Environment env;
    GameFolder testee(env.handler, env.acct, 11111, 0);
    addTurnResponse(env);

    env.acct->setGameFolderName("11111", "/game");

    // Setup
    UserConfiguration config;
    RootReceiver recv;
    std::auto_ptr<LoadGameRootTask_t> in(LoadGameRootTask_t::makeBound(&recv, &RootReceiver::take));
    std::auto_ptr<Task_t> out(testee.loadGameRoot(config, in));
    a.checkNull("01. in", in.get());
    a.checkNonNull("02. out", out.get());

    // Do it
    out->call();
    a.checkNonNull("11. root", recv.get().get());
    a.check("12. actions", recv.get()->getPossibleActions().contains(Root::aLocalSetup));
    a.check("13. actions", !recv.get()->getPossibleActions().contains(Root::aLoadEditable));
    a.checkNonNull("14. turn", recv.get()->getTurnLoader().get());

    // Turn Loader
    {
        game::Session session(env.tx, env.fs);
        session.setShipList(new ShipList());
        session.setGame(new Game());
        session.setRoot(recv.get());

        bool loadFlag = false;
        recv.get()->getTurnLoader()->loadCurrentTurn(session.getGame()->currentTurn(), *session.getGame(), 7, *recv.get(), session, game::makeResultTask(loadFlag))
            ->call();
        a.check("21. loaded", loadFlag);
        a.checkEqual("22. turn", session.getGame()->currentTurn().getTurnNumber(), 90);
    }

    // Directory removed
    a.checkNull("31. dir", env.acct->getGameFolderName("11111"));
}
