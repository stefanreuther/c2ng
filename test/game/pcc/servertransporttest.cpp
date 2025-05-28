/**
  *  \file test/game/pcc/servertransporttest.cpp
  *  \brief Test for game::pcc::ServerTransport
  */

#include "game/pcc/servertransport.hpp"

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
#include "game/pcc/browserhandler.hpp"
#include "game/test/webpage.hpp"
#include "game/test/webserver.hpp"
#include "util/profiledirectory.hpp"

using afl::base::Ref;
using afl::except::FileProblemException;
using afl::io::InternalDirectory;
using afl::io::InternalFileSystem;
using afl::net::InternalNetworkStack;
using afl::string::NullTranslator;
using afl::sys::InternalEnvironment;
using afl::sys::Log;
using game::browser::Account;
using game::browser::Session;
using game::pcc::BrowserHandler;
using game::pcc::ServerTransport;
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
                acct->setEncoded("api_token", "key", true);
                acct->setEncoded("api_user", "id", true);
            }
    };
}

// Basics
AFL_TEST("game.pcc.ServerTransport", a)
{
    Environment env;
    ServerTransport testee(env.handler, env.acct, "u/id/dir", 0);

    a.checkEqual("01. handler", &testee.handler(), &env.handler);
    a.checkEqual("02. account", &testee.account(), &*env.acct);
    a.checkEqual("03. writable", testee.isWritable(), true);

    a.check("11. fileName", testee.isValidFileName("player1.rst"));
    a.check("12. fileName", testee.isValidFileName("race.nm"));
    a.check("13. fileName", testee.isValidFileName("12-game-name"));
    a.check("14. fileName", testee.isValidFileName("file_name"));

    a.check("21. fileName", !testee.isValidFileName(".player1.rst"));
    a.check("22. fileName", !testee.isValidFileName("PLAYER1.RST"));
    a.check("23. fileName", !testee.isValidFileName("-race.nm"));
    a.check("24. fileName", !testee.isValidFileName("a/b"));
    a.check("25. fileName", !testee.isValidFileName("a:b"));
    a.check("26. fileName", !testee.isValidFileName("a\\b"));
    a.check("27. fileName", !testee.isValidFileName(String_t("a\0b", 3)));
    a.check("28. fileName", !testee.isValidFileName(""));
    a.check("29. fileName", !testee.isValidFileName("file name"));
}

// Read access (getContent, getFile)
AFL_TEST("game.pcc.ServerTransport:read:normal", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response FILE_RESPONSE[] = {
        { 0, 0, "api_token:key|action:ls|dir:u/id/dir", 0,
          "{\"reply\":[{\"id\":\"71b31ba04b78aac743677556bb6fc8f1831a4c26\",\"name\":\"test.txt\",\"size\":13,\"type\":\"file\",\"url\":\"/file.cgi/id/dir/test.txt\"},"
          "{\"name\":\"games\",\"type\":\"dir\",\"visibility\":0}],"
          "\"result\":1}" },
    };
    env.webServer.addNewPage("example.com:443", "/api/file.cgi", new WebPage(FILE_RESPONSE));

    static const WebPage::Response CONTENT_RESPONSE[] = {
        { "GET", 0, "api_token:key", 0, "hello, world!" },
    };
    env.webServer.addNewPage("example.com:443", "/file.cgi/id/dir/test.txt", new WebPage(CONTENT_RESPONSE));

    // Testee
    ServerTransport testee(env.handler, env.acct, "u/id/dir", 0);

    // Get list
    std::vector<util::ServerDirectory::FileInfo> result;
    AFL_CHECK_SUCCEEDS("01. getContent", testee.getContent(result));
    a.checkEqual("02. size", result.size(), 2U);
    a.checkEqual("03. first type", result[0].isFile, true);
    a.checkEqual("04. first name", result[0].name, "test.txt");
    a.checkEqual("05. first size", result[0].size, 13U);
    a.checkEqual("06. second type", result[1].isFile, false);
    a.checkEqual("07. second name", result[1].name, "games");
    a.checkEqual("08. second size", result[1].size, 0U);

    // Get file
    {
        afl::base::GrowableBytes_t content;
        AFL_CHECK_SUCCEEDS("11. getFile", testee.getFile("test.txt", content));
        a.checkEqual("12. content", afl::string::fromBytes(content), "hello, world!");
    }

    // Get non-existant file
    {
        afl::base::GrowableBytes_t content;
        AFL_CHECK_THROWS("21. getFile", testee.getFile("other.txt", content), FileProblemException);
    }
}

// Read access, error case: file deleted on server
AFL_TEST("game.pcc.ServerTransport:read:error:deleted-on-server", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response FILE_RESPONSE[] = {
        { 0, 0, "api_token:key|action:ls|dir:u/id/dir", 0,
          "{\"reply\":[{\"id\":\"71b31ba04b78aac743677556bb6fc8f1831a4c26\",\"name\":\"test.txt\",\"size\":13,\"type\":\"file\",\"url\":\"/file.cgi/id/dir/test.txt\"},"
          "{\"name\":\"games\",\"type\":\"dir\",\"visibility\":0}],"
          "\"result\":1}" },
    };
    env.webServer.addNewPage("example.com:443", "/api/file.cgi", new WebPage(FILE_RESPONSE));
    // No CONTENT_RESPONSE; file has been deleted on server

    // Testee
    ServerTransport testee(env.handler, env.acct, "u/id/dir", 0);

    // Get list
    std::vector<util::ServerDirectory::FileInfo> result;
    AFL_CHECK_SUCCEEDS(a("01. getContent"), testee.getContent(result));

    // Get file
    afl::base::GrowableBytes_t content;
    AFL_CHECK_THROWS(a("11. getFile"), testee.getFile("test.txt", content), FileProblemException);
}

// Read access, error case: error in getContent()
AFL_TEST("game.pcc.ServerTransport:read:error:ls-api-error", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response FILE_RESPONSE[] = {
        { 0, 0, "api_token:key|action:ls|dir:u/id/dir", 0, "{\"error\":\"sorry, it's broken\",\"result\":0}" },
    };
    env.webServer.addNewPage("example.com:443", "/api/file.cgi", new WebPage(FILE_RESPONSE));

    // Testee
    ServerTransport testee(env.handler, env.acct, "u/id/dir", 0);

    // Get list
    std::vector<util::ServerDirectory::FileInfo> result;
    AFL_CHECK_THROWS(a, testee.getContent(result), FileProblemException);
}

// putFile, normal case
AFL_TEST("game.pcc.ServerTransport:put", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response FILE_RESPONSE[] = {
        // FIXME: cannot match on file POST operation currently
        { "POST", 0, 0, 0, "{\"result\":1}" },
    };
    env.webServer.addNewPage("example.com:443", "/api/file.cgi", new WebPage(FILE_RESPONSE));

    // Testee
    ServerTransport testee(env.handler, env.acct, "u/id/dir", 0);

    // Do it
    AFL_CHECK_SUCCEEDS(a, testee.putFile("test2.txt", afl::string::toBytes("content...")));
}

// putFile, turn file case
AFL_TEST("game.pcc.ServerTransport:put:turn", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response FILE_RESPONSE[] = {
        // FIXME: cannot match on file POST operation currently
        { "POST", 0, 0, 0, "{\"result\":1,\"output\":\"text\\ntext\",\"allowtemp\":1}" },
    };
    env.webServer.addNewPage("example.com:443", "/api/host.cgi", new WebPage(FILE_RESPONSE));

    // Testee
    ServerTransport testee(env.handler, env.acct, "u/id/dir", 12);

    // Do it
    AFL_CHECK_SUCCEEDS(a("first"), testee.putFile("player3.trn", afl::string::toBytes("content")));

    // Again, with marktemp
    testee.setTemporaryTurn(true);
    AFL_CHECK_SUCCEEDS(a("second"), testee.putFile("player3.trn", afl::string::toBytes("content")));
}

// putFile, error
AFL_TEST("game.pcc.ServerTransport:put:error", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response FILE_RESPONSE[] = {
        // FIXME: cannot match on file POST operation currently
        { "POST", 0, 0, 0, "{\"result\":0,\"error\":\"boom\"}" },
    };
    env.webServer.addNewPage("example.com:443", "/api/file.cgi", new WebPage(FILE_RESPONSE));

    // Testee
    ServerTransport testee(env.handler, env.acct, "u/id/dir", 0);

    // Do it
    AFL_CHECK_THROWS(a, testee.putFile("test2.txt", afl::string::toBytes("content")), FileProblemException);
}

// putFile, turn file error case
AFL_TEST("game.pcc.ServerTransport:put:turn:error", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response FILE_RESPONSE[] = {
        // FIXME: cannot match on file POST operation currently
        { "POST", 0, 0, 0, "{\"result\":0,\"error\":\"boom\"}" },
    };
    env.webServer.addNewPage("example.com:443", "/api/host.cgi", new WebPage(FILE_RESPONSE));

    // Testee
    ServerTransport testee(env.handler, env.acct, "u/id/dir", 12);

    // Do it
    AFL_CHECK_THROWS(a, testee.putFile("player3.trn", afl::string::toBytes("content")), FileProblemException);
}

// eraseFile, normal case
AFL_TEST("game.pcc.ServerTransport:erase", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response FILE_RESPONSE[] = {
        { "POST", 0, "api_token:key|action:rm|file:u/id/dir/test3.txt", 0, "{\"result\":1}" },
    };
    env.webServer.addNewPage("example.com:443", "/api/file.cgi", new WebPage(FILE_RESPONSE));

    // Testee
    ServerTransport testee(env.handler, env.acct, "u/id/dir", 0);

    // Do it
    AFL_CHECK_SUCCEEDS(a, testee.eraseFile("test3.txt"));
}

// eraseFile, error case
AFL_TEST("game.pcc.ServerTransport:erase:error", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response FILE_RESPONSE[] = {
        { "POST", 0, "api_token:key|action:rm|file:u/id/dir/test3.txt", 0, "{\"result\":0}" },
    };
    env.webServer.addNewPage("example.com:443", "/api/file.cgi", new WebPage(FILE_RESPONSE));

    // Testee
    ServerTransport testee(env.handler, env.acct, "u/id/dir", 0);

    // Do it
    AFL_CHECK_THROWS(a, testee.eraseFile("test3.txt"), FileProblemException);
}
