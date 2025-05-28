/**
  *  \file test/game/nu/specificationloadertest.cpp
  *  \brief Test for game::nu::SpecificationLoader
  */

#include "game/nu/specificationloader.hpp"

#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/net/internalnetworkstack.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/browser/account.hpp"
#include "game/browser/session.hpp"
#include "game/nu/browserhandler.hpp"
#include "game/nu/gamestate.hpp"
#include "game/player.hpp"
#include "game/spec/basichullfunction.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/webpage.hpp"
#include "game/test/webserver.hpp"
#include "util/profiledirectory.hpp"

using afl::base::Ref;
using afl::except::FileProblemException;
using afl::io::FileSystem;
using afl::io::InternalDirectory;
using afl::io::InternalFileSystem;
using afl::net::InternalNetworkStack;
using afl::string::NullTranslator;
using afl::sys::InternalEnvironment;
using afl::sys::Log;
using game::HostVersion;
using game::Root;
using game::Task_t;
using game::browser::Account;
using game::browser::Folder;
using game::browser::Session;
using game::config::HostConfiguration;
using game::nu::BrowserHandler;
using game::nu::GameState;
using game::nu::SpecificationLoader;
using game::spec::BasicHullFunction;
using game::spec::Beam;
using game::spec::Engine;
using game::spec::Hull;
using game::spec::ShipList;
using game::spec::TorpedoLauncher;
using game::test::WebPage;
using game::test::WebServer;
using util::ProfileDirectory;

namespace {
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
              handler(session.browser(), webServer.manager(), specDir)
            { }
    };
}

// Test success case.
// A: prepare specification directory and result file. Call loadShipList().
// E: action executes and reports success.
AFL_TEST("game.nu.SpecificationLoader:loadShipList", a)
{
    Environment env;
    env.specDir->openFile("hullfunc.cc", FileSystem::Create)
        ->fullWrite(afl::string::toBytes("16,c,Cloak\n"
                                         "c = C\n"
                                         "d = Cloaking Device\n"));
    env.specDir->openFile("hullfunc.usr", FileSystem::Create)
        ->fullWrite(afl::string::toBytes("3,t,HeatsTo50\n"
                                         "c = +\n"
                                         "d = Terraforming: heats to 50F\n"));

    static const WebPage::Response TURN_RESPONSE[] = {
        { 0, 0, "apikey:the_key|gameid:99", 0,
          "{"
          "  \"success\": true,"
          "  \"rst\": {"
          "    \"settings\": {"
          "      \"name\": \"Test Game\""
          "    },"
          "    \"game\": {"
          "      \"name\": \"Test Game\""
          "    },"
          "    \"player\": {"
          "      \"status\": 1,"
          "      \"statusturn\": 1,"
          "      \"accountid\": 3333,"
          "      \"username\": \"ee-player\","
          "      \"email\": \"\","
          "      \"raceid\": 8,"
          "      \"activehulls\": \"1,15,71,\","
          "      \"activeadvantages\": \"5,22,\","
          "      \"id\": 8"
          "    },"
          "    \"players\": ["
          "      {"
          "        \"status\": 1,"
          "        \"accountid\": 1111,"
          "        \"username\": \"fed-player\","
          "        \"email\": \"\","
          "        \"raceid\": 1,"
          "        \"id\": 1"
          "      },"
          "      {"
          "        \"status\": 1,"
          "        \"accountid\": 2222,"
          "        \"username\": \"lizard-player\","
          "        \"email\": \"\","
          "        \"raceid\": 2,"
          "        \"id\": 2"
          "      }"
          "    ],"
          "    \"races\": ["
          "      {"
          "        \"name\": \"Unknown\","
          "        \"shortname\": \"Unknown\","
          "        \"adjective\": \"Unknown\","
          "        \"hulls\": \"\","
          "        \"id\": 0"
          "      },"
          "      {"
          "        \"name\": \"The Solar Federation\","
          "        \"shortname\": \"The Feds\","
          "        \"adjective\": \"Fed\","
          "        \"hulls\": \"1\","
          "        \"id\": 1"
          "      },"
          "      {"
          "        \"name\": \"The Lizard Alliance\","
          "        \"shortname\": \"The Lizards\","
          "        \"adjective\": \"Lizard\","
          "        \"hulls\": \"15\","
          "        \"id\": 2"
          "      }"
          "    ],"
          "    \"hulls\": ["
          "      {"
          "        \"name\": \"Outrider Class Scout\","
          "        \"fueltank\": 260,"
          "        \"id\": 1"
          "      },"
          "      {"
          "        \"name\": \"Small Deep Space Freighter\","
          "        \"fueltank\": 200,"
          "        \"id\": 15"
          "      }"
          "    ],"
          "    \"racehulls\": ["
          "      71,"
          "      15"
          "    ],"
          "    \"beams\": ["
          "      {"
          "        \"name\": \"Laser\","
          "        \"crewkill\": 10,"
          "        \"damage\": 3,"
          "        \"id\": 1"
          "      }"
          "    ],"
          "    \"engines\": ["
          "      {"
          "        \"name\": \"StarDrive 1\","
          "        \"warp6\": 21600,"
          "        \"id\": 1"
          "      }"
          "    ],"
          "    \"torpedos\": ["
          "      {"
          "        \"name\": \"Mark 1 Photon\","
          "        \"crewkill\": 4,"
          "        \"id\": 1"
          "      }"
          "    ]"
          "  }"
          "}" },
    };
    env.webServer.addNewPage("api.example.com:443", "/game/loadturn", new WebPage(TURN_RESPONSE));

    Ref<Account> acct = Account::create();
    acct->setType("nu");
    acct->setUser("id");
    acct->setHost("example.com");
    acct->setEncoded("api_key", "the_key", true);

    Ref<GameState> st = *new GameState(env.handler, acct, 99, 0);
    SpecificationLoader testee(env.specDir, st, env.tx, env.log);

    // Target objects
    // Root must have initialized players.
    Ref<ShipList> sl = *new ShipList();
    Ref<Root> root = game::test::makeRoot(HostVersion(HostVersion::NuHost, MKVERSION(3,2,0)));
    for (int i = 0; i < 10; ++i) {
        root->playerList().create(i);
    }

    // Do it
    bool flag = false;
    std::auto_ptr<Task_t> task(testee.loadShipList(*sl, *root, game::makeResultTask(flag)));
    a.checkNonNull("01. task", task.get());

    task->call();
    a.check("02. executed", flag);

    // Verify content
    Hull* h1 = sl->hulls().get(1);
    a.checkNonNull("11. hull 1", h1);
    a.checkEqual("12. hull 1", h1->getMaxFuel(), 260);

    Hull* h15 = sl->hulls().get(15);
    a.checkNonNull("21. hull 15", h15);
    a.checkEqual("22. hull 15", h15->getMaxFuel(), 200);

    Beam* b1 = sl->beams().get(1);
    a.checkNonNull("31. beam 1", b1);
    a.checkEqual("32. beam 1", b1->getKillPower(), 10);

    Engine* e1 = sl->engines().get(1);
    a.checkNonNull("41. engine 1", e1);
    a.checkEqual("42. engine 1", e1->getFuelFactor(6).orElse(-1), 21600);

    TorpedoLauncher* t1 = sl->launchers().get(1);
    a.checkNonNull("51. torp 1", t1);
    a.checkEqual("52. torp 1", t1->getKillPower(), 4);

    a.checkEqual("61. config", root->hostConfiguration()[HostConfiguration::GameName](), "Test Game");

    const BasicHullFunction* f3 = sl->basicHullFunctions().getFunctionById(3);
    a.checkNonNull("71. func 3", f3);
    a.checkEqual("72. func 3", f3->getName(), "HeatsTo50");

    const BasicHullFunction* f16 = sl->basicHullFunctions().getFunctionById(16);
    a.checkNonNull("81. func 16", f16);
    a.checkEqual("82. func 16", f16->getName(), "Cloak");
}

// Test failure case.
// A: prepare invalid result file. Call loadShipList().
// E: action executes and reports failure.
AFL_TEST("game.nu.SpecificationLoader:loadShipList:error", a)
{
    Environment env;

    static const WebPage::Response TURN_RESPONSE[] = {
        { 0, 0, "apikey:the_key|gameid:99", 0,
          "{"
          "  \"success\": true,"
          "  \"rst\": {"
          "    \"player\": {"
          "      \"status\": 1,"
          "      \"statusturn\": 1,"
          "      \"accountid\": 3333,"
          "      \"username\": \"ee-player\","
          "      \"email\": \"\","
          "      \"raceid\": 8,"
          "      \"activehulls\": \"1,15,71,\","
          "      \"activeadvantages\": \"5,22,\","
          "      \"id\": \"bogus\""                     // Should be integer -> causes Loader::loadShipList() to fail
          "    }"
          "  }"
          "}" },
    };
    env.webServer.addNewPage("api.example.com:443", "/game/loadturn", new WebPage(TURN_RESPONSE));

    Ref<Account> acct = Account::create();
    acct->setType("nu");
    acct->setUser("id");
    acct->setHost("example.com");
    acct->setEncoded("api_key", "the_key", true);

    Ref<GameState> st = *new GameState(env.handler, acct, 99, 0);
    SpecificationLoader testee(env.specDir, st, env.tx, env.log);

    // Target objects
    // Root must have initialized players.
    Ref<ShipList> sl = *new ShipList();
    Ref<Root> root = game::test::makeRoot(HostVersion(HostVersion::NuHost, MKVERSION(3,2,0)));
    for (int i = 0; i < 10; ++i) {
        root->playerList().create(i);
    }

    // Do it
    bool flag = false;
    std::auto_ptr<Task_t> task(testee.loadShipList(*sl, *root, game::makeResultTask(flag)));
    a.checkNonNull("01. task", task.get());

    // Reports error
    task->call();
    a.check("02. executed", !flag);
}

// Test openSpecificationFile, success and error cases.
AFL_TEST("game.nu.SpecificationLoader:openSpecificationFile", a)
{
    Environment env;
    env.specDir->openFile("hullfunc.cc", FileSystem::Create)
        ->fullWrite(afl::string::toBytes("abcdef"));

    Ref<Account> acct = Account::create();
    acct->setType("nu");
    acct->setUser("id");
    acct->setHost("example.com");
    acct->setEncoded("api_key", "the_key", true);

    Ref<GameState> st = *new GameState(env.handler, acct, 99, 0);
    SpecificationLoader testee(env.specDir, st, env.tx, env.log);

    // Success
    a.checkEqual("01. success", testee.openSpecificationFile("hullfunc.cc")->getSize(), 6U);

    // Failure
    AFL_CHECK_THROWS(a("02. fail"), testee.openSpecificationFile("none.txt"), FileProblemException);
}
