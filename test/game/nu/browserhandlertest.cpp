/**
  *  \file test/game/nu/browserhandlertest.cpp
  *  \brief Test for game::nu::BrowserHandler
  */

#include "game/nu/browserhandler.hpp"

#include "afl/container/ptrvector.hpp"
#include "afl/data/access.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/net/headertable.hpp"
#include "afl/net/internalnetworkstack.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/browser/account.hpp"
#include "game/browser/folder.hpp"
#include "game/browser/session.hpp"
#include "game/task.hpp"
#include "game/test/webpage.hpp"
#include "game/test/webserver.hpp"
#include "game/turnloader.hpp"
#include "util/io.hpp"
#include "util/profiledirectory.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::container::PtrVector;
using afl::data::Access;
using afl::data::Value;
using afl::io::FileSystem;
using afl::io::InternalDirectory;
using afl::io::InternalFileSystem;
using afl::net::HeaderTable;
using afl::net::InternalNetworkStack;
using afl::string::NullTranslator;
using afl::sys::InternalEnvironment;
using afl::sys::Log;
using game::Root;
using game::Task_t;
using game::TurnLoader;
using game::browser::Account;
using game::browser::Folder;
using game::browser::LoadGameRootTask_t;
using game::browser::Session;
using game::browser::UserCallback;
using game::config::UserConfiguration;
using game::nu::BrowserHandler;
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

    // Create a task that stores 'true' in the given flag, to track that it was called
    std::auto_ptr<game::Task_t> makeTrackerTask(bool& flag)
    {
        return game::makeConfirmationTask(true, game::makeResultTask(flag));
    }

    // Implementation of LoadGameRootTask_t
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
}

AFL_TEST("game.nu.BrowserHandler", a)
{
    Environment env;
    a.checkEqual("01. translator", &env.handler.translator(), &env.tx);
    a.checkEqual("02. log",        &env.handler.log(),        &env.log);
    a.checkEqual("03. browser",    &env.handler.browser(),    &env.session.browser());
    a.checkEqual("04. specDir",    &*env.handler.getDefaultSpecificationDirectory(), &*env.specDir);

    PtrVector<Folder> result;
    a.check("11. handleFolderName", !env.handler.handleFolderName("/x", result));
}

/*
 *  createAccountFolder
 */

AFL_TEST("game.nu.BrowserHandler:createAccountFolder:success", a)
{
    Environment env;
    Ref<Account> acct = Account::create();
    acct->setHost("planets.nu");
    acct->setUser("u");
    acct->setType("nu");

    std::auto_ptr<Folder> p(env.handler.createAccountFolder(acct));
    a.checkNonNull("result", p.get());
}

AFL_TEST("game.nu.BrowserHandler:createAccountFolder:failure", a)
{
    Environment env;
    Ref<Account> acct = Account::create();
    acct->setHost("planets.nu");
    acct->setUser("u");
    acct->setType("other");

    std::auto_ptr<Folder> p(env.handler.createAccountFolder(acct));
    a.checkNull("result", p.get());
}

/*
 *  callServer (low-level primitive)
 */

// Normal case
AFL_TEST("game.nu.BrowserHandler:callServer", a)
{
    Environment env;

    static const WebPage::Response RESP[] = {
        { 0, 0, "arg:value", 0, "{\"result\":42}" },
    };

    env.webServer.addNewPage("api.planets.nu:443", "/api/test", new WebPage(RESP));

    Ref<Account> acct = Account::create();
    acct->setHost("planets.nu");
    acct->setUser("u");
    acct->setType("nu");

    HeaderTable args;
    args.set("arg", "value");

    std::auto_ptr<Value> result(env.handler.callServer(acct, "/api/test", args));
    a.checkEqual("result", Access(result)("result").toInteger(), 42);
}

// Normal case, but explicit URL given
AFL_TEST("game.nu.BrowserHandler:callServer:explicit-uri", a)
{
    Environment env;

    static const WebPage::Response RESP[] = {
        { 0, 0, 0, 0, "{\"result\":42}" },
    };

    env.webServer.addNewPage("apihost.com:80", "/v3/api/test", new WebPage(RESP));

    Ref<Account> acct = Account::create();
    acct->setHost("example.com");
    acct->setUser("u");
    acct->setType("nu");
    acct->set("url", "http://apihost.com/v3/api", true);

    HeaderTable args;

    std::auto_ptr<Value> result(env.handler.callServer(acct, "/test", args));
    a.checkEqual("result", Access(result)("result").toInteger(), 42);
}

// Error case: Host not reachable/connect failed
AFL_TEST("game.nu.BrowserHandler:callServer:error:bad-host", a)
{
    Environment env;

    Ref<Account> acct = Account::create();
    acct->setHost("example.com");
    acct->setUser("u");
    acct->setType("nu");

    HeaderTable args;

    std::auto_ptr<Value> result(env.handler.callServer(acct, "/test", args));
    a.checkNull("result", result.get());
}

// Error case: bad URL
AFL_TEST("game.nu.BrowserHandler:callServer:error:bad-uri", a)
{
    Environment env;

    Ref<Account> acct = Account::create();
    acct->setHost("example.com");
    acct->setUser("u");
    acct->setType("nu");
    acct->set("url", "http://:@", true);

    HeaderTable args;

    std::auto_ptr<Value> result(env.handler.callServer(acct, "/test", args));
    a.checkNull("result", result.get());
}

// Error case: HTTP error (server generates 404 because we use an undefined endpoint)
AFL_TEST("game.nu.BrowserHandler:callServer:error:bad-path", a)
{
    Environment env;

    env.webServer.addNewPage("api.example.com:443", "/bad/path", new WebPage(afl::base::Nothing));

    Ref<Account> acct = Account::create();
    acct->setHost("example.com");
    acct->setUser("u");
    acct->setType("nu");

    HeaderTable args;

    std::auto_ptr<Value> result(env.handler.callServer(acct, "/test", args));
    a.checkNull("result", result.get());
}

// Error case: Server sends bad data (not JSON)
AFL_TEST("game.nu.BrowserHandler:callServer:bad-data", a)
{
    Environment env;

    static const WebPage::Response RESP[] = {
        { 0, 0, 0, 0, "Go away" },
    };

    env.webServer.addNewPage("api.example.com:443", "/api/test", new WebPage(RESP));

    Ref<Account> acct = Account::create();
    acct->setHost("example.com");
    acct->setUser("u");
    acct->setType("nu");

    HeaderTable args;

    std::auto_ptr<Value> result(env.handler.callServer(acct, "/api/test", args));
    a.checkNull("result", result.get());
}

/*
 *  login
 */

// Login flow, success case
AFL_TEST("game.nu.BrowserHandler:login", a)
{
    Environment env;

    // User callback
    class Callback : public UserCallback {
        virtual void askPassword(const PasswordRequest& /*req*/)
            {
                PasswordResponse resp;
                resp.password = "secret";
                resp.canceled = false;
                sig_passwordResult.raise(resp);
            }
    };
    Callback cb;
    env.session.callback().setInstance(&cb);

    // Web server side
    static const WebPage::Response LOGIN_RESPONSE[] = {
        { 0, 0, "username:user_id|password:secret", 0, "{\"success\":1,\"apikey\":\"cookie\"}" },
    };
    env.webServer.addNewPage("api.planets.nu:443", "/account/login", new WebPage(LOGIN_RESPONSE));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("nu");
    acct->setUser("user_id");
    acct->setHost("planets.nu");

    // Task tracker
    bool called = false;
    std::auto_ptr<Task_t> task = env.handler.login(acct, makeTrackerTask(called));
    a.checkNonNull("01. task", task.get());
    task->call();
    a.check("02. called", called);

    // Verify result
    a.checkEqual("11. token", acct->getEncoded("api_key").orElse("?"), "cookie");
}

// Login flow, already logged in
AFL_TEST("game.nu.BrowserHandler:login:already-logged-in", a)
{
    Environment env;

    // Web server side (will not be called)
    env.webServer.addNewPage("api.planets.nu:443", "/account/login", new WebPage(afl::base::Nothing));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("nu");
    acct->setUser("user_id");
    acct->setHost("planets.nu");
    acct->setEncoded("api_key", "secret", false);

    // Task tracker
    bool called = false;
    std::auto_ptr<Task_t> task = env.handler.login(acct, makeTrackerTask(called));
    a.checkNonNull("01. task", task.get());
    task->call();
    a.check("02. called", called);

    // Verify result
    a.checkEqual("11. token", acct->getEncoded("api_key").orElse(""), "secret");
}

// Login flow, failure case: wrong password (server responds with failure)
AFL_TEST("game.nu.BrowserHandler:login:error:wrong-password", a)
{
    Environment env;

    // User callback
    class Callback : public UserCallback {
        virtual void askPassword(const PasswordRequest& /*req*/)
            {
                PasswordResponse resp;
                resp.password = "secret";
                resp.canceled = false;
                sig_passwordResult.raise(resp);
            }
    };
    Callback cb;
    env.session.callback().setInstance(&cb);

    // Web server side
    static const WebPage::Response LOGIN_RESPONSE[] = {
        { 0, 0, "username:user_id|password:secret", 0, "{\"success\":0}" },
    };
    env.webServer.addNewPage("api.planets.nu:443", "/account/login", new WebPage(LOGIN_RESPONSE));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("nu");
    acct->setUser("user_id");
    acct->setHost("planets.nu");

    // Task tracker
    bool called = false;
    std::auto_ptr<Task_t> task = env.handler.login(acct, makeTrackerTask(called));
    a.checkNonNull("01. task", task.get());
    task->call();
    a.check("02. called", called);

    // Verify result
    a.check("11. token", !acct->getEncoded("api_key").isValid());
}

// Login flow, cancel
AFL_TEST("game.nu.BrowserHandler:login:error:cancel", a)
{
    Environment env;

    // User callback
    class Callback : public UserCallback {
        virtual void askPassword(const PasswordRequest& /*req*/)
            {
                PasswordResponse resp;
                resp.password = "secret";
                resp.canceled = true;
                sig_passwordResult.raise(resp);
            }
    };
    Callback cb;
    env.session.callback().setInstance(&cb);

    // Web server side
    // (Should not be called)
    static const WebPage::Response LOGIN_RESPONSE[] = {
        { 0, 0, "username:user_id|password:secret", 0, "{\"success\":1,\"apikey\":\"cookie\"}" },
    };
    env.webServer.addNewPage("api.planets.nu:443", "/account/login", new WebPage(LOGIN_RESPONSE));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("nu");
    acct->setUser("user_id");
    acct->setHost("planets.nu");

    // Task tracker
    bool called = false;
    std::auto_ptr<Task_t> task = env.handler.login(acct, makeTrackerTask(called));
    a.checkNonNull("01. task", task.get());
    task->call();
    a.check("02. called", called);

    // Verify result
    a.check("11. token", !acct->getEncoded("api_key").isValid());
}

// Login flow, server error case
AFL_TEST("game.nu.BrowserHandler:login:error:server-error", a)
{
    Environment env;

    // User callback
    class Callback : public UserCallback {
        virtual void askPassword(const PasswordRequest& /*req*/)
            {
                PasswordResponse resp;
                resp.password = "secret";
                resp.canceled = false;
                sig_passwordResult.raise(resp);
            }
    };
    Callback cb;
    env.session.callback().setInstance(&cb);

    // Web server side; API endpoint will generate 404
    env.webServer.addNewPage("api.planets.nu:443", "/whatever", new WebPage(afl::base::Nothing));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("nu");
    acct->setUser("user_id");
    acct->setHost("planets.nu");

    // Task tracker
    bool called = false;
    std::auto_ptr<Task_t> task = env.handler.login(acct, makeTrackerTask(called));
    a.checkNonNull("01. task", task.get());
    task->call();
    a.check("02. called", called);

    // Verify result
    a.check("11. token", !acct->getEncoded("api_key").isValid());
}

/*
 *  getGameListPreAuthenticated
 */

AFL_TEST("game.nu.BrowserHandler:getGameListPreAuthenticated", a)
{
    Environment env;

    // Web server side
    // An actual game list is huge, but BrowserHandler isn't supposed to parse it.
    // Thus, just return a minimum result.
    static const WebPage::Response LIST_RESPONSE[] = {
        { 0, 0, "apikey:first_cookie", 0,
          "{\"success\":true,\"games\":[{\"game\":{\"id\":100}},{\"game\":{\"id\":200}}]}" },
        { 0, 0, "apikey:second_cookie", 0,
          "{\"success\":true,\"games\":[{\"game\":{\"id\":500}}]}" },
    };
    env.webServer.addNewPage("api.planets.nu:443", "/account/mygames", new WebPage(LIST_RESPONSE));

    // Accounts
    Ref<Account> acct1 = Account::create();
    acct1->setType("nu");
    acct1->setUser("one");
    acct1->setHost("planets.nu");
    acct1->setEncoded("api_key", "first_cookie", false);

    Ref<Account> acct2 = Account::create();
    acct2->setType("nu");
    acct2->setUser("two");
    acct2->setHost("planets.nu");
    acct2->setEncoded("api_key", "second_cookie", false);

    // Fetch
    Access list1 = env.handler.getGameListPreAuthenticated(acct1);
    a.checkEqual("01. content", list1("games").getArraySize(), 2U);
    a.checkEqual("02. content", list1("games")[0]("game")("id").toInteger(), 100);
    a.checkEqual("03. content", list1("games")[1]("game")("id").toInteger(), 200);

    // Fetch other account
    Access list2 = env.handler.getGameListPreAuthenticated(acct2);
    a.checkEqual("11. content", list2("games").getArraySize(), 1U);
    a.checkEqual("12. content", list2("games")[0]("game")("id").toInteger(), 500);

    // Re-fetch
    Access list2a = env.handler.getGameListPreAuthenticated(acct2);
    a.checkEqual("21. content", list2a("games").getArraySize(), 1U);
    a.checkEqual("22. content", list2a("games")[0]("game")("id").toInteger(), 500);
}

/*
 *  getAccountInfoPreAuthenticated
 */

AFL_TEST("game.nu.BrowserHandler:getAccountInfoPreAuthenticated", a)
{
    Environment env;

    static const WebPage::Response LOAD_RESPONSE[] = {
        { 0, 0, "apikey:first_cookie", 0,
          "{\"success\":true,\"account\":{\"username\":\"user one\"}}" },
        { 0, 0, "apikey:second_cookie", 0,
          "{\"success\":true,\"account\":{\"username\":\"user two\"}}" },
    };
    env.webServer.addNewPage("api.planets.nu:443", "/account/load", new WebPage(LOAD_RESPONSE));

    // Accounts
    Ref<Account> acct1 = Account::create();
    acct1->setType("nu");
    acct1->setUser("one");
    acct1->setHost("planets.nu");
    acct1->setEncoded("api_key", "first_cookie", false);

    Ref<Account> acct2 = Account::create();
    acct2->setType("nu");
    acct2->setUser("two");
    acct2->setHost("planets.nu");
    acct2->setEncoded("api_key", "second_cookie", false);

    // Fetch
    Access list1 = env.handler.getAccountInfoPreAuthenticated(acct1);
    a.checkEqual("01. content", list1("account")("username").toString(), "user one");

    // Fetch other account
    Access list2 = env.handler.getAccountInfoPreAuthenticated(acct2);
    a.checkEqual("11. content", list2("account")("username").toString(), "user two");

    // Re-fetch
    Access list2a = env.handler.getAccountInfoPreAuthenticated(acct2);
    a.checkEqual("21. content", list2a("account")("username").toString(), "user two");
}

/*
 *  loadGameRootMaybe
 */

// Normal case
AFL_TEST("game.nu.BrowserHandler:loadGameRootMaybe", a)
{
    Environment env;

    // Account
    Ref<Account> acct = Account::create();
    acct->setUser("u");
    acct->setType("nu");
    acct->setHost("planets.nu");
    acct->setEncoded("api_key", "the_cookie", true);
    env.session.accountManager().addNewAccount(acct);

    // Config
    Ref<InternalDirectory> gameDir = InternalDirectory::create("game");
    UserConfiguration config;
    config[UserConfiguration::Game_Type].set("nu");
    config[UserConfiguration::Game_User].set("u");
    config[UserConfiguration::Game_Host].set("planets.nu");
    config[UserConfiguration::Game_Id].set("42");

    // Web server
    static const WebPage::Response TURN_RESPONSE[] = {
        { 0, 0, "apikey:the_cookie|gameid:42", 0,
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
    env.webServer.addNewPage("api.planets.nu:443", "/game/loadturn", new WebPage(TURN_RESPONSE));

    static const WebPage::Response LIST_RESPONSE[] = {
        { 0, 0, "apikey:the_cookie", 0,
          "{\"success\":true,\"games\":[{\"game\":{\"id\":42}}]}" },
    };
    env.webServer.addNewPage("api.planets.nu:443", "/account/mygames", new WebPage(LIST_RESPONSE));

    // Do it
    RootReceiver recv;
    std::auto_ptr<LoadGameRootTask_t> in(LoadGameRootTask_t::makeBound(&recv, &RootReceiver::take));
    std::auto_ptr<Task_t> out(env.handler.loadGameRootMaybe(gameDir, config, in));
    a.checkNull("01. in", in.get());
    a.checkNonNull("02. out", out.get());

    out->call();

    // Verify root
    a.checkNonNull("11. root", recv.get().get());
    a.check("12. act", recv.get()->getPossibleActions().contains(Root::aLocalSetup));

    // Verify TurnLoader
    String_t tmp;
    a.checkNonNull("21. root", recv.get()->getTurnLoader().get());
}

// Error case: no account object for this folder
AFL_TEST("game.nu.BrowserHandler:loadGameRootMaybe:error:no-account", a)
{
    Environment env;

    // Config
    Ref<InternalDirectory> gameDir = InternalDirectory::create("game");
    UserConfiguration config;
    config[UserConfiguration::Game_Type].set("nu");
    config[UserConfiguration::Game_User].set("u");
    config[UserConfiguration::Game_Host].set("planets.nu");
    config[UserConfiguration::Game_Id].set("42");

    // Do it
    RootReceiver recv;
    std::auto_ptr<LoadGameRootTask_t> in(LoadGameRootTask_t::makeBound(&recv, &RootReceiver::take));
    std::auto_ptr<Task_t> out(env.handler.loadGameRootMaybe(gameDir, config, in));
    a.checkNonNull("01. in", in.get());
    a.checkNull("02. out", out.get());
}

// Error case: invalid Id
AFL_TEST("game.nu.BrowserHandler:loadGameRootMaybe:error:no-id", a)
{
    Environment env;

    // Account
    Ref<Account> acct = Account::create();
    acct->setUser("u");
    acct->setType("nu");
    acct->setHost("planets.nu");
    acct->setEncoded("api_key", "the_cookie", true);
    env.session.accountManager().addNewAccount(acct);

    // Config
    Ref<InternalDirectory> gameDir = InternalDirectory::create("game");
    UserConfiguration config;
    config[UserConfiguration::Game_Type].set("nu");
    config[UserConfiguration::Game_User].set("u");
    config[UserConfiguration::Game_Host].set("planets.nu");
    config[UserConfiguration::Game_Id].set("xyz");

    // Do it
    RootReceiver recv;
    std::auto_ptr<LoadGameRootTask_t> in(LoadGameRootTask_t::makeBound(&recv, &RootReceiver::take));
    std::auto_ptr<Task_t> out(env.handler.loadGameRootMaybe(gameDir, config, in));
    a.checkNonNull("01. in", in.get());
    a.checkNull("02. out", out.get());
}

// Folder applies to other type of game
AFL_TEST("game.nu.BrowserHandler:loadGameRootMaybe:no-match", a)
{
    Environment env;

    // Account
    Ref<Account> acct = Account::create();
    acct->setUser("u");
    acct->setType("other");
    acct->setHost("planets.nu");
    acct->setEncoded("api_key", "the_cookie", true);
    env.session.accountManager().addNewAccount(acct);

    // Config
    Ref<InternalDirectory> gameDir = InternalDirectory::create("game");
    UserConfiguration config;
    config[UserConfiguration::Game_Type].set("other");
    config[UserConfiguration::Game_User].set("u");
    config[UserConfiguration::Game_Host].set("planets.nu");
    config[UserConfiguration::Game_Id].set("xyz");

    // Do it
    RootReceiver recv;
    std::auto_ptr<LoadGameRootTask_t> in(LoadGameRootTask_t::makeBound(&recv, &RootReceiver::take));
    std::auto_ptr<Task_t> out(env.handler.loadGameRootMaybe(gameDir, config, in));
    a.checkNonNull("01. in", in.get());
    a.checkNull("02. out", out.get());
}
