/**
  *  \file test/game/nu/turnloadertest.cpp
  *  \brief Test for game::nu::TurnLoader
  */

#include "game/nu/turnloader.hpp"

#include "afl/io/internalfilesystem.hpp"
#include "afl/net/internalnetworkstack.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/browser/account.hpp"
#include "game/browser/session.hpp"
#include "game/game.hpp"
#include "game/nu/browserhandler.hpp"
#include "game/nu/gamestate.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/webpage.hpp"
#include "game/test/webserver.hpp"
#include "game/turn.hpp"
#include "util/profiledirectory.hpp"

using afl::base::Ref;
using afl::io::Directory;
using afl::io::FileSystem;
using afl::io::InternalFileSystem;
using afl::net::InternalNetworkStack;
using afl::string::NullTranslator;
using afl::sys::InternalEnvironment;
using afl::sys::Log;
using game::HostVersion;
using game::Root;
using game::Game;
using game::browser::Account;
using game::browser::Session;
using game::nu::BrowserHandler;
using game::nu::GameState;
using game::nu::TurnLoader;
using game::spec::ShipList;
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
        fs.createDirectory("/spec");
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
        Ref<Directory> specDir;
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
              specDir(fs.openDirectory("/spec")),
              handler(session.browser(), webServer.manager(), specDir),
              acct(Account::create())
            {
                acct->setType("nu");
                acct->setUser("id");
                acct->setHost("example.com");
                acct->setName("Account Name");
                acct->setEncoded("api_key", "the_key", true);
            }
    };

    void addListResponse(Environment& env)
    {
        static const WebPage::Response LIST_RESPONSE[] = {
            { 0, 0, "apikey:the_key", 0,
              "{\"success\":true,\"games\":[{\"game\":{\"id\":99},\"player\":{\"id\":7}},{\"game\":{\"id\":98}}]}" },
        };
        env.webServer.addNewPage("api.example.com:443", "/account/mygames", new WebPage(LIST_RESPONSE));
    }
}

// Test basics
AFL_TEST("game.nu.TurnLoader:basics", a)
{
    Environment env;
    addListResponse(env);
    Ref<GameState> st = *new GameState(env.handler, env.acct, 99, 0);
    TurnLoader testee(st, env.profile, env.specDir);

    // getPlayerStatus
    {
        String_t extra;
        a.check("01. player 7", testee.getPlayerStatus(7, extra, env.tx).contains(TurnLoader::Available));
        a.check("02. extra 7", !extra.empty());
    }
    {
        String_t extra;
        a.check("11. player 3", !testee.getPlayerStatus(3, extra, env.tx).contains(TurnLoader::Available));
        a.check("12. extra 3", extra.empty());
    }

    // getProperty
    a.checkEqual("21. local",  testee.getProperty(TurnLoader::LocalFileFormatProperty), "Nu");
    a.checkEqual("22. remote", testee.getProperty(TurnLoader::RemoteFileFormatProperty), "Nu");
    a.checkEqual("23. root",   testee.getProperty(TurnLoader::RootDirectoryProperty), "/spec");
}

// Test loading a turn
AFL_TEST("game.nu.TurnLoader:turn", a)
{
    Environment env;
    addListResponse(env);

    static const WebPage::Response TURN_RESPONSE[] = {
        { 0, 0, "apikey:the_key", 0,
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

    Ref<GameState> st = *new GameState(env.handler, env.acct, 99, 0);
    TurnLoader testee(st, env.profile, env.specDir);

    // Game environment
    game::Session session(env.tx, env.fs);
    session.setGame(new Game());
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new ShipList());

    // getHistoryStatus
    TurnLoader::HistoryStatus hist[10];
    testee.getHistoryStatus(7, 85, hist, *session.getRoot());
    a.checkEqual("01. hist", hist[0], TurnLoader::WeaklyPositive);  // 85
    a.checkEqual("02. hist", hist[1], TurnLoader::WeaklyPositive);
    a.checkEqual("03. hist", hist[2], TurnLoader::WeaklyPositive);  // 87
    a.checkEqual("04. hist", hist[3], TurnLoader::WeaklyPositive);
    a.checkEqual("05. hist", hist[4], TurnLoader::WeaklyPositive);  // 89
    a.checkEqual("06. hist", hist[5], TurnLoader::Negative);        // Current turn is NEGATIVE because it cannot be retrieved as history
    a.checkEqual("07. hist", hist[6], TurnLoader::Negative);        // 91
    a.checkEqual("08. hist", hist[7], TurnLoader::Negative);

    // loadCurrentTurn
    bool loaded = false;
    testee.loadCurrentTurn(session.getGame()->currentTurn(), *session.getGame(), 7, *session.getRoot(), session, game::makeResultTask(loaded))
        ->call();
    a.check("11. loaded", loaded);
    a.checkEqual("12. turn", session.getGame()->currentTurn().getTurnNumber(), 90);
}

// Test loading a turn, error case.
// Turn loading fails if result does not match schema (parse error).
AFL_TEST("game.nu.TurnLoader:turn:error", a)
{
    Environment env;
    addListResponse(env);

    static const WebPage::Response TURN_RESPONSE[] = {
        { 0, 0, "apikey:the_key", 0,
          "{"
          "  \"success\": true,"
          "  \"rst\": {"
          "    \"settings\": {"
          "      \"hostcompleted\": \"4/12/2012 9:04:45 PM\""
          "    },"
          "    \"game\": {"
          "      \"turn\": \"BOOM\""
          "    }"
          "  }"
          "}" },
    };
    env.webServer.addNewPage("api.example.com:443", "/game/loadturn", new WebPage(TURN_RESPONSE));

    Ref<GameState> st = *new GameState(env.handler, env.acct, 99, 0);
    TurnLoader testee(st, env.profile, env.specDir);

    // Game environment
    game::Session session(env.tx, env.fs);
    session.setGame(new Game());
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new ShipList());

    // getHistoryStatus
    TurnLoader::HistoryStatus hist[10];
    testee.getHistoryStatus(7, 85, hist, *session.getRoot());
    a.checkEqual("01. hist", hist[0], TurnLoader::Negative);
    a.checkEqual("02. hist", hist[1], TurnLoader::Negative);
    a.checkEqual("03. hist", hist[2], TurnLoader::Negative);
    a.checkEqual("04. hist", hist[3], TurnLoader::Negative);
    a.checkEqual("05. hist", hist[4], TurnLoader::Negative);
    a.checkEqual("06. hist", hist[5], TurnLoader::Negative);
    a.checkEqual("07. hist", hist[6], TurnLoader::Negative);
    a.checkEqual("08. hist", hist[7], TurnLoader::Negative);

    // loadCurrentTurn: will fail because turn number cannot be parsed
    bool loaded = false;
    testee.loadCurrentTurn(session.getGame()->currentTurn(), *session.getGame(), 7, *session.getRoot(), session, game::makeResultTask(loaded))
        ->call();
    a.check("01. loaded", !loaded);
}

// Test loading a turn, error case.
// Turn loading fails if server does not report success=true.
AFL_TEST("game.nu.TurnLoader:turn:unsuccessful", a)
{
    Environment env;
    addListResponse(env);

    static const WebPage::Response TURN_RESPONSE[] = {
        { 0, 0, "apikey:the_key", 0, "{\"success\": false}" },
    };
    env.webServer.addNewPage("api.example.com:443", "/game/loadturn", new WebPage(TURN_RESPONSE));

    Ref<GameState> st = *new GameState(env.handler, env.acct, 99, 0);
    TurnLoader testee(st, env.profile, env.specDir);

    // Game environment
    game::Session session(env.tx, env.fs);
    session.setGame(new Game());
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new ShipList());

    // getHistoryStatus
    TurnLoader::HistoryStatus hist[10];
    testee.getHistoryStatus(7, 85, hist, *session.getRoot());
    a.checkEqual("01. hist", hist[0], TurnLoader::Negative);
    a.checkEqual("02. hist", hist[1], TurnLoader::Negative);
    a.checkEqual("03. hist", hist[2], TurnLoader::Negative);
    a.checkEqual("04. hist", hist[3], TurnLoader::Negative);
    a.checkEqual("05. hist", hist[4], TurnLoader::Negative);
    a.checkEqual("06. hist", hist[5], TurnLoader::Negative);
    a.checkEqual("07. hist", hist[6], TurnLoader::Negative);
    a.checkEqual("08. hist", hist[7], TurnLoader::Negative);

    // loadCurrentTurn
    bool loaded = false;
    testee.loadCurrentTurn(session.getGame()->currentTurn(), *session.getGame(), 7, *session.getRoot(), session, game::makeResultTask(loaded))
        ->call();
    a.check("01. loaded", !loaded);
}

// Test getPlayerStatus, error case: bad player.
AFL_TEST("game.nu.TurnLoader:getPlayerStatus:bad-player", a)
{
    Environment env;

    static const WebPage::Response LIST_RESPONSE[] = {
        { 0, 0, "apikey:the_key", 0, "{\"success\":true,\"games\":[{\"game\":{\"id\":99},\"player\":{\"id\":\"X\"}}]}" },
    };
    env.webServer.addNewPage("api.example.com:443", "/account/mygames", new WebPage(LIST_RESPONSE));

    Ref<GameState> st = *new GameState(env.handler, env.acct, 99, 0);
    TurnLoader testee(st, env.profile, env.specDir);

    // getPlayerStatus
    String_t extra;
    a.check("01. player 7", testee.getPlayerStatus(7, extra, env.tx).empty());
    a.check("02. extra 7", extra.empty());
}

// Test getPlayerStatus, error case: bad turn status.
AFL_TEST("game.nu.TurnLoader:getPlayerStatus:bad-status", a)
{
    Environment env;

    static const WebPage::Response LIST_RESPONSE[] = {
        { 0, 0, "apikey:the_key", 0, "{\"success\":true,\"games\":[{\"game\":{\"id\":99},\"player\":{\"id\":7,\"turnstatus\":\"X\"}}]}" },
    };
    env.webServer.addNewPage("api.example.com:443", "/account/mygames", new WebPage(LIST_RESPONSE));

    Ref<GameState> st = *new GameState(env.handler, env.acct, 99, 0);
    TurnLoader testee(st, env.profile, env.specDir);

    // getPlayerStatus
    String_t extra;
    a.check("01. player 7", testee.getPlayerStatus(7, extra, env.tx).empty());
    a.check("02. extra 7", extra.empty());
}
