/**
  *  \file test/game/pcc/turnloadertest.cpp
  *  \brief Test for game::pcc::TurnLoader
  */

#include "game/pcc/turnloader.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
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
#include "game/pcc/servertransport.hpp"
#include "game/test/webserver.hpp"
#include "util/profiledirectory.hpp"

using afl::base::Ref;
using afl::charset::Charset;
using afl::charset::CodepageCharset;
using afl::io::InternalDirectory;
using afl::io::InternalFileSystem;
using afl::net::InternalNetworkStack;
using afl::string::NullTranslator;
using afl::sys::InternalEnvironment;
using afl::sys::Log;
using game::PlayerSet_t;
using game::browser::Account;
using game::browser::Session;
using game::pcc::BrowserHandler;
using game::pcc::ServerTransport;
using game::pcc::TurnLoader;
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

// Test some basics.
// The environment for this class is rather complex.
// Therefore, some more tests are in "game.pcc.GameFolder:loadGameRoot"
// (and eventually, we'll leave this class with lower unit-test coverage.)
AFL_TEST("game.pcc.TurnLoader", a)
{
    Environment env;
    Ref<ServerTransport> transport = *new ServerTransport(env.handler, env.acct, "u/id/dir", 0);

    Ref<TurnLoader> testee = *new TurnLoader(transport,
                                             env.specDir,
                                             std::auto_ptr<Charset>(new CodepageCharset(afl::charset::g_codepage437)),
                                             env.log,
                                             PlayerSet_t(7),
                                             env.profile);

    // getPlayerStatus
    {
        String_t tmp;
        TurnLoader::PlayerStatusSet_t result = testee->getPlayerStatus(7, tmp, env.tx);
        a.check("01. available", result.contains(TurnLoader::Available));
        a.check("02. text", !tmp.empty());
    }
    {
        String_t tmp;
        TurnLoader::PlayerStatusSet_t result = testee->getPlayerStatus(1, tmp, env.tx);
        a.check("11. available", !result.contains(TurnLoader::Available));
        a.check("12. text", tmp.empty());
    }

    // getProperty
    a.checkDifferent("21. local", testee->getProperty(TurnLoader::LocalFileFormatProperty), "");
    a.checkDifferent("22. remote", testee->getProperty(TurnLoader::RemoteFileFormatProperty), "");
    a.checkEqual("23. root", testee->getProperty(TurnLoader::RootDirectoryProperty), env.specDir->getDirectoryName());
}
