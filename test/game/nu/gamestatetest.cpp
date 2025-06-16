/**
  *  \file test/game/nu/gamestatetest.cpp
  *  \brief Test for game::nu::GameState
  */

#include "game/nu/gamestate.hpp"

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
#include "game/player.hpp"
#include "game/test/webpage.hpp"
#include "game/test/webserver.hpp"
#include "util/profiledirectory.hpp"

/*
 *  Much of this is tested indirectly through the users of this class; just test some corner cases.
 */

using afl::base::Ref;
using afl::io::FileSystem;
using afl::io::InternalDirectory;
using afl::io::InternalFileSystem;
using afl::net::InternalNetworkStack;
using afl::string::NullTranslator;
using afl::sys::InternalEnvironment;
using afl::sys::Log;
using game::Player;
using game::browser::Account;
using game::browser::Folder;
using game::browser::Session;
using game::nu::BrowserHandler;
using game::nu::GameState;
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

// getGameListEntry
AFL_TEST("game.nu.GameState:getGameListEntry", a)
{
    Environment env;

    static const WebPage::Response LIST_RESPONSE[] = {
        { 0, 0, "apikey:the_key", 0,
          "{\"success\":true,\"games\":[{\"game\":{\"id\":100}},{\"game\":{\"id\":200}}]}" },
    };
    env.webServer.addNewPage("api.example.com:443", "/account/mygames", new WebPage(LIST_RESPONSE));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("nu");
    acct->setUser("id");
    acct->setHost("example.com");
    acct->setEncoded("api_key", "the_key", true);

    // Matching key
    a.checkEqual("01. matching hint", GameState(env.handler, acct, 100, 0).loadGameListEntryPreAuthenticated()("game")("id").toInteger(), 100);
    a.checkEqual("02. matching hint", GameState(env.handler, acct, 200, 1).loadGameListEntryPreAuthenticated()("game")("id").toInteger(), 200);

    // Mismatching hint
    a.checkEqual("11. mismatching hint", GameState(env.handler, acct, 100, 1).loadGameListEntryPreAuthenticated()("game")("id").toInteger(), 100);
    a.checkEqual("12. mismatching hint", GameState(env.handler, acct, 100, 99).loadGameListEntryPreAuthenticated()("game")("id").toInteger(), 100);

    // Nonexistant
    a.checkNull("21. missing", GameState(env.handler, acct, 999, 0).loadGameListEntryPreAuthenticated().getValue());
    a.checkNull("21. missing", GameState(env.handler, acct, 999, 99).loadGameListEntryPreAuthenticated().getValue());
}

// setRaceName
AFL_TEST("game.nu.GameState:setRaceName:1", a)
{
    Player pl(1);
    NullTranslator tx;
    a.check("success", GameState::setRaceName(pl, 1));
    a.checkEqual("name", pl.getName(Player::ShortName, tx), "The Feds");
}

AFL_TEST("game.nu.GameState:setRaceName:12", a)
{
    Player pl(1);
    NullTranslator tx;
    a.check("success", GameState::setRaceName(pl, 12));
    a.checkEqual("name", pl.getName(Player::ShortName, tx), "The Horwasp");
}

AFL_TEST("game.nu.GameState:setRaceName:0", a)
{
    Player pl(1);
    NullTranslator tx;
    a.check("success", !GameState::setRaceName(pl, 0));
}

// loadResult, invalidateResult
AFL_TEST("game.nu.GameState:loadResult", a)
{
    Environment env;

    static const WebPage::Response TURN_RESPONSE[] = {
        { 0, 0, "apikey:the_key|gameid:11111", 0,
          "{"
          "  \"success\": true,"
          "  \"rst\": {"
          "    \"settings\": {\"hostcompleted\": \"4/12/2012 9:04:45 PM\"},"
          "    \"game\": {\"turn\": 90}"
          "  }"
          "}" },
    };
    env.webServer.addNewPage("api.example.com:443", "/game/loadturn", new WebPage(TURN_RESPONSE));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("nu");
    acct->setUser("id");
    acct->setHost("example.com");
    acct->setEncoded("api_key", "the_key", true);

    // Load result succeeds
    GameState st(env.handler, acct, 11111, 0);
    a.checkEqual("01. first load", st.loadResultPreAuthenticated()("rst")("game")("turn").toInteger(), 90);
    a.checkEqual("02. second load", st.loadResultPreAuthenticated()("rst")("game")("turn").toInteger(), 90);

    // After invalidation, loaded again
    st.invalidateResult();
    a.checkEqual("11. third load", st.loadResultPreAuthenticated()("rst")("game")("turn").toInteger(), 90);

    // Trigger error after invalidate; will not load
    st.invalidateResult();
    acct->setEncoded("api_key", "wrong_key", true);
    a.checkNull("21. fourth load", st.loadResultPreAuthenticated().getValue());
}
