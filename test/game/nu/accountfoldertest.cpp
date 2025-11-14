/**
  *  \file test/game/nu/accountfoldertest.cpp
  *  \brief Test for game::nu::AccountFolder
  */

#include "game/nu/accountfolder.hpp"

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
#include "game/config/userconfiguration.hpp"
#include "game/nu/browserhandler.hpp"
#include "game/test/webpage.hpp"
#include "game/test/webserver.hpp"
#include "util/profiledirectory.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::container::PtrVector;
using afl::io::InternalDirectory;
using afl::io::InternalFileSystem;
using afl::net::InternalNetworkStack;
using afl::string::NullTranslator;
using afl::sys::InternalEnvironment;
using afl::sys::Log;
using game::Root;
using game::browser::Account;
using game::browser::Folder;
using game::browser::LoadContentTask_t;
using game::browser::Session;
using game::browser::UnsupportedAccountFolder;
using game::config::UserConfiguration;
using game::nu::AccountFolder;
using game::nu::BrowserHandler;
using game::test::WebPage;
using game::test::WebServer;
using util::ProfileDirectory;

namespace {
    class RootReceiver {
     public:
        RootReceiver(afl::test::Assert a)
            : m_beenHere(false),
              m_assert(a)
            { }

        void take(Ptr<Root> r)
            {
                m_assert.checkNull("take: Root", r.get());
                m_beenHere = true;
            }

        bool beenHere() const
            { return m_beenHere; }

     private:
        bool m_beenHere;
        afl::test::Assert m_assert;
    };

    class ContentReceiver {
     public:
        ContentReceiver()
            : m_result()
            { }

        void take(PtrVector<Folder>& result)
            { m_result.swap(result); }

        PtrVector<Folder>& result()
            { return m_result; }

     private:
        PtrVector<Folder> m_result;
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
                acct->setEncoded("api_key", "secret", true);
            }
    };
}

// Test basic/simple operations
AFL_TEST("game.nu.AccountFolder:basics", a)
{
    Environment env;
    AccountFolder testee(env.handler, env.acct);

    Ref<UserConfiguration> uc = UserConfiguration::create();

    // Accessors
    a.check         ("01. canEnter",               testee.canEnter());
    a.checkEqual    ("02. getName",                testee.getName(), "Account Name");
    a.checkDifferent("03. getText",                testee.getDescription().getText(), "");
    a.check         ("04. setLocalDirectoryName", !testee.setLocalDirectoryName("foo"));
    a.check         ("05. loadConfiguration",     !testee.loadConfiguration(*uc));
    a.checkEqual    ("06. getKind",                testee.getKind(), Folder::kAccount);

    // loadGameRoot
    RootReceiver recv(a("RootReceiver"));
    std::auto_ptr<game::Task_t> t = testee.loadGameRoot(*uc, std::auto_ptr<game::browser::LoadGameRootTask_t>(game::browser::LoadGameRootTask_t::makeBound(&recv, &RootReceiver::take)));
    a.checkNonNull("11. get", t.get());
    t->call();
    a.check("12. beenHere", recv.beenHere());

    // Dummies
    AFL_CHECK_SUCCEEDS(a("21. saveConfiguration"), testee.saveConfiguration(*uc));

    a.check("31. isSame", testee.isSame(testee));

    AccountFolder other(env.handler, Account::create());
    a.check("32. isSame", !testee.isSame(other));

    UnsupportedAccountFolder other2(env.tx, env.acct);
    a.check("33. isSame", !testee.isSame(other2));
}

// Test content retrieval
AFL_TEST("game.nu.AccountFolder:content", a)
{
    Environment env;

    // Web server side
    static const WebPage::Response LIST_RESPONSE[] = {
        { 0, 0, "apikey:secret", 0,
          "{\"games\": ["
          "{\"game\":{\"id\":11111,\"name\":\"First Game\",\"description\":\"First description\",\"slots\":11},\"player\":{\"id\":7,\"raceid\":7,\"username\":\"me\"}},"
          "{\"game\":{\"id\":22222,\"name\":\"Second Game\",\"description\":\"Second description\",\"slots\":2},\"player\": {\"raceid\":7,\"id\":1,\"username\":\"me\"}}"
          "],\"created\": [],\"success\": true}"
        },
    };
    env.webServer.addNewPage("api.example.com:443", "/account/mygames", new WebPage(LIST_RESPONSE));

    // Query
    AccountFolder testee(env.handler, env.acct);
    ContentReceiver receiver;
    testee.loadContent(std::auto_ptr<LoadContentTask_t>(LoadContentTask_t::makeBound(&receiver, &ContentReceiver::take)))
        ->call();
    a.checkEqual("21. size", receiver.result().size(), 2U);
    a.checkEqual("22. first", receiver.result()[0]->getName(), "First Game (11111)");
    a.checkEqual("23. second", receiver.result()[1]->getName(), "Second Game (22222)");
}
