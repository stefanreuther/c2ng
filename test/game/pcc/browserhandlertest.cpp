/**
  *  \file test/game/pcc/browserhandlertest.cpp
  *  \brief Test for game::pcc::BrowserHandler
  */

#include "game/pcc/browserhandler.hpp"

#include "afl/container/ptrvector.hpp"
#include "afl/data/access.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/net/headertable.hpp"
#include "afl/net/http/simpledownloadlistener.hpp"
#include "afl/net/internalnetworkstack.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/browser/account.hpp"
#include "game/browser/folder.hpp"
#include "game/browser/session.hpp"
#include "game/hostversion.hpp"
#include "game/task.hpp"
#include "game/test/files.hpp"
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
using afl::net::http::SimpleDownloadListener;
using afl::string::NullTranslator;
using afl::sys::InternalEnvironment;
using afl::sys::Log;
using game::HostVersion;
using game::Root;
using game::Task_t;
using game::TurnLoader;
using game::browser::Account;
using game::browser::Folder;
using game::browser::Session;
using game::browser::UserCallback;
using game::config::UserConfiguration;
using game::pcc::BrowserHandler;
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
              handler(session.browser(), webServer.manager(), specDir, profile)
            { }
    };

    // Create a task that stores 'true' in the given flag, to track that it was called
    std::auto_ptr<game::Task_t> makeTrackerTask(bool& flag)
    {
        return game::makeConfirmationTask(true, game::makeResultTask(flag));
    }
}

/*
 *  General
 */

AFL_TEST("game.pcc.BrowserHandler:basics", a)
{
    Environment env;
    a.checkEqual("01. translator", &env.handler.translator(), &env.tx);
    a.checkEqual("02. log",        &env.handler.log(),        &env.log);
    a.checkEqual("03. callback",   &env.handler.callback(),   &env.session.callback());

    PtrVector<Folder> result;
    a.check("11. handleFolderName", !env.handler.handleFolderName("/x", result));

    // TODO: virtual std::auto_ptr<Task_t> loadGameRootMaybe(afl::base::Ref<afl::io::Directory> dir, const game::config::UserConfiguration& config, std::auto_ptr<game::browser::LoadGameRootTask_t>& then);
}


/*
 *  createAccountFolder
 */

AFL_TEST("game.pcc.BrowserHandler:createAccountFolder:success", a)
{
    Environment env;
    Ref<Account> acct = Account::create();
    acct->setHost("planetscentral.com");
    acct->setUser("u");
    acct->setType("pcc");

    std::auto_ptr<Folder> p(env.handler.createAccountFolder(acct));
    a.checkNonNull("result", p.get());
}

AFL_TEST("game.pcc.BrowserHandler:createAccountFolder:failure", a)
{
    Environment env;
    Ref<Account> acct = Account::create();
    acct->setHost("planetscentral.com");
    acct->setUser("u");
    acct->setType("other");

    std::auto_ptr<Folder> p(env.handler.createAccountFolder(acct));
    a.checkNull("result", p.get());
}

/*
 *  callServer (low-level primitive)
 */

// Normal case
AFL_TEST("game.pcc.BrowserHandler:callServer", a)
{
    Environment env;

    static const WebPage::Response RESP[] = {
        { 0, 0, "arg:value", 0, "{\"result\":42}" },
    };

    env.webServer.addNewPage("example.com:443", "/api/test.cgi", new WebPage(RESP));

    Ref<Account> acct = Account::create();
    acct->setHost("example.com");
    acct->setUser("u");
    acct->setType("pcc");

    HeaderTable args;
    args.set("arg", "value");

    std::auto_ptr<Value> result(env.handler.callServer(acct, "test", args));
    a.checkEqual("result", Access(result)("result").toInteger(), 42);
}

// Normal case, but explicit URL given
AFL_TEST("game.pcc.BrowserHandler:callServer:explicit-uri", a)
{
    Environment env;

    static const WebPage::Response RESP[] = {
        { 0, 0, 0, 0, "{\"result\":42}" },
    };

    env.webServer.addNewPage("apihost.com:80", "/v3/api/test.cgi", new WebPage(RESP));

    Ref<Account> acct = Account::create();
    acct->setHost("example.com");
    acct->setUser("u");
    acct->setType("pcc");
    acct->set("url", "http://apihost.com/v3/api", true);

    HeaderTable args;

    std::auto_ptr<Value> result(env.handler.callServer(acct, "test", args));
    a.checkEqual("result", Access(result)("result").toInteger(), 42);
}

// Error case: Host not reachable/connect failed
AFL_TEST("game.pcc.BrowserHandler:callServer:error:bad-host", a)
{
    Environment env;

    Ref<Account> acct = Account::create();
    acct->setHost("example.com");
    acct->setUser("u");
    acct->setType("pcc");

    HeaderTable args;

    std::auto_ptr<Value> result(env.handler.callServer(acct, "test", args));
    a.checkNull("result", result.get());
}

// Error case: bad URL
AFL_TEST("game.pcc.BrowserHandler:callServer:error:bad-uri", a)
{
    Environment env;

    Ref<Account> acct = Account::create();
    acct->setHost("example.com");
    acct->setUser("u");
    acct->setType("pcc");
    acct->set("url", "http://:@", true);

    HeaderTable args;

    std::auto_ptr<Value> result(env.handler.callServer(acct, "test", args));
    a.checkNull("result", result.get());
}

// Error case: HTTP error (server generates 404 because we use an undefined endpoint)
AFL_TEST("game.pcc.BrowserHandler:callServer:error:bad-path", a)
{
    Environment env;

    env.webServer.addNewPage("example.com:443", "/bad/path", new WebPage(afl::base::Nothing));

    Ref<Account> acct = Account::create();
    acct->setHost("example.com");
    acct->setUser("u");
    acct->setType("pcc");

    HeaderTable args;

    std::auto_ptr<Value> result(env.handler.callServer(acct, "test", args));
    a.checkNull("result", result.get());
}

// Error case: Server sends bad data (not JSON)
AFL_TEST("game.pcc.BrowserHandler:callServer:bad-data", a)
{
    Environment env;

    static const WebPage::Response RESP[] = {
        { 0, 0, 0, 0, "Go away" },
    };

    env.webServer.addNewPage("example.com:443", "/api/test.cgi", new WebPage(RESP));

    Ref<Account> acct = Account::create();
    acct->setHost("example.com");
    acct->setUser("u");
    acct->setType("pcc");

    HeaderTable args;

    std::auto_ptr<Value> result(env.handler.callServer(acct, "test", args));
    a.checkNull("result", result.get());
}

/*
 *  callServerWithFile
 */

// Normal case
AFL_TEST("game.pcc.BrowserHandler:callServerWithFile", a)
{
    Environment env;

    static const WebPage::Response RESP[] = {
        // FIXME: for now, PageRequest cannot handle form uplodads (multipart/form-data),
        // so we cannot verify the content of the message.
        { "POST", 0, 0, 0, "{\"result\":42}" },
    };

    env.webServer.addNewPage("example.com:443", "/api/test.cgi", new WebPage(RESP));

    Ref<Account> acct = Account::create();
    acct->setHost("example.com");
    acct->setUser("u");
    acct->setType("pcc");

    HeaderTable args;
    args.set("direct_arg", "direct_value");

    std::auto_ptr<Value> result(env.handler.callServerWithFile(acct, "test", args, "file_param", "filename.txt", afl::string::toBytes("file_content")));
    a.checkEqual("result", Access(result)("result").toInteger(), 42);
}

// Bad URL case
AFL_TEST("game.pcc.BrowserHandler:callServerWithFile:error:bad-url", a)
{
    Environment env;

    static const WebPage::Response RESP[] = {
        // FIXME: for now, PageRequest cannot handle form uplodads (multipart/form-data),
        // so we cannot verify the content of the message.
        { "POST", 0, 0, 0, "{\"result\":42}" },
    };

    env.webServer.addNewPage("example.com:443", "/api/test.cgi", new WebPage(RESP));

    Ref<Account> acct = Account::create();
    acct->setHost("example.com");
    acct->setUser("u");
    acct->setType("pcc");
    acct->set("url", "http://:@", true);

    HeaderTable args;

    std::auto_ptr<Value> result(env.handler.callServerWithFile(acct, "test", args, "file_param", "filename.txt", afl::string::toBytes("file_content")));
    a.checkNull("result", result.get());
}

/*
 *  login
 */

// Login flow, success case
AFL_TEST("game.pcc.BrowserHandler:login", a)
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
    static const WebPage::Response USER_RESPONSE[] = {
        { 0, 0, "api_user:user_id|api_password:secret|action:whoami", 0, "{\"result\":1,\"api_token\":\"cookie\",\"username\":\"user_id\"}" },
    };
    env.webServer.addNewPage("planetscentral.com:443", "/api/user.cgi", new WebPage(USER_RESPONSE));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("pcc");
    acct->setUser("user_id");
    acct->setHost("planetscentral.com");

    // Task tracker
    bool called = false;
    std::auto_ptr<Task_t> task = env.handler.login(acct, makeTrackerTask(called));
    a.checkNonNull("01. task", task.get());
    task->call();
    a.check("02. called", called);

    // Verify result
    a.checkEqual("11. token", acct->getEncoded("api_token").orElse(""), "cookie");
    a.checkEqual("12. token", acct->getEncoded("api_user").orElse(""), "user_id");
}

// Login flow, already logged in
AFL_TEST("game.pcc.BrowserHandler:login:already-logged-in", a)
{
    Environment env;

    // Web server side (will not be called)
    env.webServer.addNewPage("planetscentral.com:443", "/api/user.cgi", new WebPage(afl::base::Nothing));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("pcc");
    acct->setUser("user_id");
    acct->setHost("planetscentral.com");
    acct->setEncoded("api_token", "cookie", false);
    acct->setEncoded("api_user", "user_id", false);

    // Task tracker
    bool called = false;
    std::auto_ptr<Task_t> task = env.handler.login(acct, makeTrackerTask(called));
    a.checkNonNull("01. task", task.get());
    task->call();
    a.check("02. called", called);

    // Verify result
    a.checkEqual("11. token", acct->getEncoded("api_token").orElse(""), "cookie");
    a.checkEqual("12. token", acct->getEncoded("api_user").orElse(""), "user_id");
}

// Login flow, failure case: wrong password (server responds with failure)
AFL_TEST("game.pcc.BrowserHandler:login:error:wrong-password", a)
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
    static const WebPage::Response USER_RESPONSE[] = {
        { 0, 0, "api_user:user_id|api_password:secret|action:whoami", 0, "{\"result\":0}" },
    };
    env.webServer.addNewPage("planetscentral.com:443", "/api/user.cgi", new WebPage(USER_RESPONSE));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("pcc");
    acct->setUser("user_id");
    acct->setHost("planetscentral.com");

    // Task tracker
    bool called = false;
    std::auto_ptr<Task_t> task = env.handler.login(acct, makeTrackerTask(called));
    a.checkNonNull("01. task", task.get());
    task->call();
    a.check("02. called", called);

    // Verify result
    a.check("11. token", !acct->getEncoded("api_token").isValid());
}

// Login flow, cancel
AFL_TEST("game.pcc.BrowserHandler:login:error:cancel", a)
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
    static const WebPage::Response USER_RESPONSE[] = {
        { 0, 0, "api_user:user_id|api_password:secret|action:whoami", 0, "{\"result\":1,\"api_token\":\"cookie\",\"username\":\"user_id\"}" },
    };
    env.webServer.addNewPage("planetscentral.com:443", "/api/user.cgi", new WebPage(USER_RESPONSE));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("pcc");
    acct->setUser("user_id");
    acct->setHost("planetscentral.com");

    // Task tracker
    bool called = false;
    std::auto_ptr<Task_t> task = env.handler.login(acct, makeTrackerTask(called));
    a.checkNonNull("01. task", task.get());
    task->call();
    a.check("02. called", called);

    // Verify result
    a.check("11. token", !acct->getEncoded("api_token").isValid());
}

// Login flow, server error case
AFL_TEST("game.pcc.BrowserHandler:login:error:server-error", a)
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
    env.webServer.addNewPage("planetscentral.com:443", "/whatever", new WebPage(afl::base::Nothing));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("pcc");
    acct->setUser("user_id");
    acct->setHost("planetscentral.com");

    // Task tracker
    bool called = false;
    std::auto_ptr<Task_t> task = env.handler.login(acct, makeTrackerTask(called));
    a.checkNonNull("01. task", task.get());
    task->call();
    a.check("02. called", called);

    // Verify result
    a.check("11. token", !acct->getEncoded("api_token").isValid());
}

/*
 *  getGameListPreAuthenticated
 */

AFL_TEST("game.pcc.BrowserHandler:getGameListPreAuthenticated", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response FILE_RESPONSE[] = {
        { 0, 0, "api_token:cookie|dir:u/user_id|action:lsgame", 0,
          "{\"reply\":["
          "{\"conflict\":[],\"finished\":0,\"game\":0,\"hosttime\":0,\"hostversion\":\"PHost 4.1e\","
          "\"missing\":[\"pconfig.src\"],\"name\":\"\",\"path\":\"u/user_id/one\",\"races\":{\"7\":\"The Crystal Confederation\"}},"
          "{\"conflict\":[],\"finished\":0,\"game\":0,\"hosttime\":0,\"hostversion\":\"PHost 3.4l\",\"missing\":"
          "[\"race.nm\",\"beamspec.dat\",\"engspec.dat\",\"hullspec.dat\",\"pconfig.src\",\"planet.nm\","
          "\"torpspec.dat\",\"truehull.dat\",\"xyplan.dat\"],\"name\":\"\",\"path\":\"u/user_id/two\",\"races\":"
          "{\"9\":\"The Robotic Imperium\"}}],\"result\":1}" },
        { 0, 0, "api_token:other|dir:u/other|action:lsgame", 0,
          "{\"reply\":[{\"conflict\":[],\"finished\":1,\"game\":1,\"hosttime\":"
          "0,\"hostversion\":\"PHost 4.1e\",\"missing\":[],\"name\":\"First Steps\",\"path\":\"u/other/games/1-first-steps\","
          "\"races\":{\"7\":\"The Crystal Confederation\"}}],\"result\":1}" },
    };
    env.webServer.addNewPage("planetscentral.com:443", "/api/file.cgi", new WebPage(FILE_RESPONSE));

    // Accounts
    Ref<Account> acct1 = Account::create();
    acct1->setType("pcc");
    acct1->setUser("user_id");
    acct1->setHost("planetscentral.com");
    acct1->setEncoded("api_token", "cookie", false);
    acct1->setEncoded("api_user", "user_id", false);

    Ref<Account> acct2 = Account::create();
    acct2->setType("pcc");
    acct2->setUser("user_id");
    acct2->setHost("planetscentral.com");
    acct2->setEncoded("api_token", "other", false);
    acct2->setEncoded("api_user", "other", false);

    // Fetch
    Access list1 = env.handler.getGameListPreAuthenticated(acct1);
    a.checkEqual("01. content", list1("reply").getArraySize(), 2U);
    a.checkEqual("02. content", list1("reply")[0]("path").toString(), "u/user_id/one");
    a.checkEqual("03. content", list1("reply")[1]("path").toString(), "u/user_id/two");

    // Fetch other account
    Access list2 = env.handler.getGameListPreAuthenticated(acct2);
    a.checkEqual("11. content", list2("reply").getArraySize(), 1U);
    a.checkEqual("12. content", list2("reply")[0]("path").toString(), "u/other/games/1-first-steps");

    // Re-fetch
    Access list2a = env.handler.getGameListPreAuthenticated(acct2);
    a.checkEqual("21. content", list2a("reply").getArraySize(), 1U);
    a.checkEqual("22. content", list2a("reply")[0]("path").toString(), "u/other/games/1-first-steps");
}

/*
 *  getDirectoryContentPreAuthenticated
 */

AFL_TEST("game.pcc.BrowserHandler:getDirectoryContentPreAuthenticated", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response FILE_RESPONSE[] = {
        { 0, 0, "api_token:cookie|dir:u/user_id|action:ls", 0,
          "{\"reply\":[{\"id\":\"71b31ba04b78aac743677556bb6fc8f1831a4c26\",\"name\":\"alert.ccb\",\"size\":181,\"type\":\"file\",\"url\":\"/file.cgi/streu/alert.ccb\"},"
          "{\"name\":\"games\",\"type\":\"dir\",\"visibility\":0},"
          "{\"id\":\"65fc702cdb6b82d6a63504f3d549396351b91b87\",\"name\":\"infinite.ccb\",\"size\":181,\"type\":\"file\",\"url\":\"/file.cgi/streu/infinite.ccb\"}],"
          "\"result\":1}"
        }
    };
    env.webServer.addNewPage("planetscentral.com:443", "/api/file.cgi", new WebPage(FILE_RESPONSE));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("pcc");
    acct->setUser("user_id");
    acct->setHost("planetscentral.com");
    acct->setEncoded("api_token", "cookie", false);
    acct->setEncoded("api_user", "user_id", false);

    // Fetch
    std::auto_ptr<Value> list(env.handler.getDirectoryContentPreAuthenticated(acct, "u/user_id"));
    a.checkEqual("01. content", Access(list)("reply").getArraySize(), 3U);
    a.checkEqual("02. content", Access(list)("reply")[0]("name").toString(), "alert.ccb");
    a.checkEqual("03. content", Access(list)("reply")[1]("name").toString(), "games");
    a.checkEqual("04. content", Access(list)("reply")[2]("name").toString(), "infinite.ccb");
}

AFL_TEST("game.pcc.BrowserHandler:getDirectoryContentPreAuthenticated:not-logged-in", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response FILE_RESPONSE[] = {
        { 0, 0, 0, 0, "{\"reply\":[],\"result\":1}" }
    };
    env.webServer.addNewPage("planetscentral.com:443", "/api/file.cgi", new WebPage(FILE_RESPONSE));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("pcc");
    acct->setUser("user_id");
    acct->setHost("planetscentral.com");

    // Fetch
    std::auto_ptr<Value> list(env.handler.getDirectoryContentPreAuthenticated(acct, "u/user_id"));
    a.checkNull("01. result", list.get());
}

/*
 *  getFilePreAuthenticated
 */

AFL_TEST("game.pcc.BrowserHandler:getFilePreAuthenticated", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response FILE_RESPONSE[] = {
        { "GET", 0, "api_token:cookie", 0, "FileContent" }
    };
    env.webServer.addNewPage("planetscentral.com:443", "/file.cgi/u/user_id/file.txt", new WebPage(FILE_RESPONSE));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("pcc");
    acct->setUser("user_id");
    acct->setHost("planetscentral.com");
    acct->setEncoded("api_token", "cookie", false);
    acct->setEncoded("api_user", "user_id", false);

    // Fetch
    SimpleDownloadListener out;
    env.handler.getFilePreAuthenticated(acct, "/file.cgi/u/user_id/file.txt", out);
    SimpleDownloadListener::Status st = out.wait();
    a.checkEqual("01. status", st, SimpleDownloadListener::Succeeded);
    a.checkEqual("02. code", out.getStatusCode(), 200);
    a.checkEqual("03. data", afl::string::fromBytes(out.getResponseData()), "FileContent");
}

AFL_TEST("game.pcc.BrowserHandler:getFilePreAuthenticated:not-logged-in", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response FILE_RESPONSE[] = {
        { "GET", 0, 0, 0, "FileContent" }
    };
    env.webServer.addNewPage("planetscentral.com:443", "/file.cgi/u/user_id/file.txt", new WebPage(FILE_RESPONSE));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("pcc");
    acct->setUser("user_id");
    acct->setHost("planetscentral.com");

    // Fetch
    SimpleDownloadListener out;
    env.handler.getFilePreAuthenticated(acct, "/file.cgi/u/user_id/file.txt", out);
    SimpleDownloadListener::Status st = out.wait();
    a.checkEqual("01. status", st, SimpleDownloadListener::Failed);
}

/*
 *  putFilePreAuthenticated
 */

AFL_TEST("game.pcc.BrowserHandler:putFilePreAuthenticated", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response FILE_RESPONSE[] = {
        // FIXME: cannot match on file content right now; see callServerWithFile
        { "POST", 0, 0 /*"api_token:cookie|action:put|file:u/user_id/test.txt|data:FileContent"*/, 0, "{\"result\":1}" }
    };
    env.webServer.addNewPage("planetscentral.com:443", "/api/file.cgi", new WebPage(FILE_RESPONSE));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("pcc");
    acct->setUser("user_id");
    acct->setHost("planetscentral.com");
    acct->setEncoded("api_token", "cookie", false);
    acct->setEncoded("api_user", "user_id", false);

    // Post
    std::auto_ptr<Value> result(env.handler.putFilePreAuthenticated(acct, "u/user_id/test.txt", afl::string::toBytes("FileContent")));
    a.checkEqual("01. result", Access(result)("result").toInteger(), 1);
}

AFL_TEST("game.pcc.BrowserHandler:putFilePreAuthenticated:not-logged-in", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response FILE_RESPONSE[] = {
        { 0, 0, 0, 0, "{\"result\":1}" }
    };
    env.webServer.addNewPage("planetscentral.com:443", "/api/file.cgi", new WebPage(FILE_RESPONSE));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("pcc");
    acct->setUser("user_id");
    acct->setHost("planetscentral.com");

    // Post
    std::auto_ptr<Value> result(env.handler.putFilePreAuthenticated(acct, "u/user_id/test.txt", afl::string::toBytes("FileContent")));
    a.checkNull("01. result", result.get());
}

/*
 *  eraseFilePreAuthenticated
 */

AFL_TEST("game.pcc.BrowserHandler:eraseFilePreAuthenticated", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response FILE_RESPONSE[] = {
        { "POST", 0, "api_token:cookie|action:rm|file:u/user_id/test.txt", 0, "{\"result\":1}" },
    };
    env.webServer.addNewPage("planetscentral.com:443", "/api/file.cgi", new WebPage(FILE_RESPONSE));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("pcc");
    acct->setUser("user_id");
    acct->setHost("planetscentral.com");
    acct->setEncoded("api_token", "cookie", false);
    acct->setEncoded("api_user", "user_id", false);

    // Post
    std::auto_ptr<Value> result(env.handler.eraseFilePreAuthenticated(acct, "u/user_id/test.txt"));
    a.checkEqual("01. result", Access(result)("result").toInteger(), 1);
}

AFL_TEST("game.pcc.BrowserHandler:eraseFilePreAuthenticated:not-logged-in", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response FILE_RESPONSE[] = {
        { 0, 0, 0, 0, "{\"result\":1}" }
    };
    env.webServer.addNewPage("planetscentral.com:443", "/api/file.cgi", new WebPage(FILE_RESPONSE));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("pcc");
    acct->setUser("user_id");
    acct->setHost("planetscentral.com");

    // Post
    std::auto_ptr<Value> result(env.handler.eraseFilePreAuthenticated(acct, "u/user_id/test.txt"));
    a.checkNull("01. result", result.get());
}

/*
 *  uploadTurnPreAuthenticated
 */

AFL_TEST("game.pcc.BrowserHandler:uploadTurnPreAuthenticated", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response HOST_RESPONSE[] = {
        // FIXME: cannot match on file content right now; see callServerWithFile
        { "POST", 0, 0 /*"api_token:cookie|action:trn|gid:42|slot:7|data:FileContent"*/, 0, "{\"result\":1}" }
    };
    env.webServer.addNewPage("planetscentral.com:443", "/api/host.cgi", new WebPage(HOST_RESPONSE));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("pcc");
    acct->setUser("user_id");
    acct->setHost("planetscentral.com");
    acct->setEncoded("api_token", "cookie", false);
    acct->setEncoded("api_user", "user_id", false);

    // Post
    std::auto_ptr<Value> result(env.handler.uploadTurnPreAuthenticated(acct, 42, 7, afl::string::toBytes("FileContent")));
    a.checkEqual("01. result", Access(result)("result").toInteger(), 1);
}

AFL_TEST("game.pcc.BrowserHandler:uploadTurnPreAuthenticated:not-logged-in", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response HOST_RESPONSE[] = {
        { 0, 0, 0, 0, "{\"result\":1}" }
    };
    env.webServer.addNewPage("planetscentral.com:443", "/api/host.cgi", new WebPage(HOST_RESPONSE));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("pcc");
    acct->setUser("user_id");
    acct->setHost("planetscentral.com");

    // Post
    std::auto_ptr<Value> result(env.handler.uploadTurnPreAuthenticated(acct, 42, 7, afl::string::toBytes("FileContent")));
    a.checkNull("01. result", result.get());
}

/*
 *  markTurnTemporaryPreAuthenticated
 */

AFL_TEST("game.pcc.BrowserHandler:markTurnTemporaryPreAuthenticated", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response HOST_RESPONSE[] = {
        { "POST", 0, "api_token:cookie|action:trnmarktemp|gid:42|slot:7|istemp:1", 0, "{\"result\":1}" },
    };
    env.webServer.addNewPage("planetscentral.com:443", "/api/host.cgi", new WebPage(HOST_RESPONSE));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("pcc");
    acct->setUser("user_id");
    acct->setHost("planetscentral.com");
    acct->setEncoded("api_token", "cookie", false);
    acct->setEncoded("api_user", "user_id", false);

    // Post
    AFL_CHECK_SUCCEEDS(a, env.handler.markTurnTemporaryPreAuthenticated(acct, 42, 7, 1));
}

AFL_TEST("game.pcc.BrowserHandler:markTurnTemporaryPreAuthenticated:not-logged-in", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response HOST_RESPONSE[] = {
        { 0, 0, 0, 0, "{\"result\":1}" },
    };
    env.webServer.addNewPage("planetscentral.com:443", "/api/host.cgi", new WebPage(HOST_RESPONSE));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("pcc");
    acct->setUser("user_id");
    acct->setHost("planetscentral.com");

    // Post
    AFL_CHECK_SUCCEEDS(a, env.handler.markTurnTemporaryPreAuthenticated(acct, 42, 7, 1));
}

/*
 *  loadRoot
 */

AFL_TEST("game.pcc.BrowserHandler:loadRoot", a)
{
    Environment env;
    env.specDir->openFile("race.nm", FileSystem::Create)
        ->fullWrite(game::test::getDefaultRaceNames());

    // Game list entry
    const char* gameListJSON =
        "{\"conflict\":[],\"finished\":0,\"game\":0,\"hosttime\":0,\"hostversion\":\"PHost 4.1e\","
        "\"missing\":[\"pconfig.src\"],\"name\":\"\",\"path\":\"u/user_id/one\",\"races\":{\"7\":\"The Crystal Confederation\"}}";
    std::auto_ptr<Value> gameListEntry(util::parseJSON(afl::string::toBytes(gameListJSON)));

    // Account
    Ref<Account> acct = Account::create();
    acct->setType("pcc");
    acct->setUser("user_id");
    acct->setHost("planetscentral.com");
    acct->setEncoded("api_token", "cookie", false);
    acct->setEncoded("api_user", "user_id", false);

    // Config
    UserConfiguration config;
    config[UserConfiguration::Game_Type].set("pcc");

    // Do it
    Ptr<Root> result = env.handler.loadRoot(acct, gameListEntry, config);
    a.checkNonNull("01. result", result.get());
    a.checkEqual("02. host type", result->hostVersion().getKind(), HostVersion::PHost);
    a.checkEqual("03. host vers", result->hostVersion().getVersion(), MKVERSION(4, 1, 5));

    String_t tmp;
    a.checkNonNull("11. turnLoader", result->getTurnLoader().get());
    a.check("12. players",  result->getTurnLoader()->getPlayerStatus(7, tmp, env.tx).contains(TurnLoader::Available));
    a.check("13. players", !result->getTurnLoader()->getPlayerStatus(5, tmp, env.tx).contains(TurnLoader::Available));
}
