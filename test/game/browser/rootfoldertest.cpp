/**
  *  \file test/game/browser/rootfoldertest.cpp
  *  \brief Test for game::browser::RootFolder
  */

#include "game/browser/rootfolder.hpp"

#include "afl/container/ptrvector.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/browser/accountmanager.hpp"
#include "game/browser/browser.hpp"
#include "game/browser/optionalusercallback.hpp"
#include "game/config/userconfiguration.hpp"
#include "util/profiledirectory.hpp"

using afl::base::Ref;
using afl::container::PtrVector;
using afl::io::InternalFileSystem;
using afl::string::NullTranslator;
using afl::sys::InternalEnvironment;
using afl::sys::Log;
using game::browser::Account;
using game::browser::AccountManager;
using game::browser::Browser;
using game::browser::Folder;
using game::browser::LoadGameRootTask_t;
using game::browser::OptionalUserCallback;
using game::browser::RootFolder;
using game::config::UserConfiguration;
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

    struct LoadTask {
        bool called;
        afl::base::Ptr<game::Root> result;

        LoadTask()
            : called(), result()
            { }
        void keep(afl::base::Ptr<game::Root> p)
            { result = p; called = true; }
    };

    struct BrowserEnvironment {
        InternalEnvironment env;
        InternalFileSystem fs;
        ProfileDirectory profile;
        NullTranslator tx;
        Log log;
        AccountManager accounts;
        OptionalUserCallback callback;
        Browser browser;

        BrowserEnvironment()
            : env(), fs(), profile(prepareEnvironment(env), prepareFileSystem(fs)),
              tx(), log(), accounts(profile, tx, log),
              browser(fs, tx, log, accounts, profile, callback)
            { }
    };
}

AFL_TEST("game.browser.RootFolder", a)
{
    BrowserEnvironment env;
    Ref<Account> account = Account::create();
    account->setUser("u");
    account->setType("t");
    account->setHost("h");
    env.accounts.addNewAccount(account);
    RootFolder testee(env.browser);

    // Configuration
    Ref<UserConfiguration> config = UserConfiguration::create();
    a.check("01. loadConfiguration", !testee.loadConfiguration(*config));
    a.check("02. setLocalDirectoryName", !testee.setLocalDirectoryName("/"));
    AFL_CHECK_SUCCEEDS(a("03. saveConfiguration"), testee.saveConfiguration(*config));

    // Names
    AFL_CHECK_SUCCEEDS(a("11. getName"), testee.getName());
    AFL_CHECK_SUCCEEDS(a("12. getDescription"), testee.getDescription());

    // Others
    a.check("21. isSame", testee.isSame(testee));
    a.check("22. canEnter", testee.canEnter());
    a.checkEqual("23. getKind", testee.getKind(), Folder::kRoot);

    // Enumerate
    PtrVector<Folder> list;
    testee.loadContent(list);
    a.checkGreaterEqual("31. list", list.size(), 2U); // et least 2: FS root + Account
    for (size_t i = 0; i < list.size(); ++i) {
        a.checkNonNull("32. content", list[i]);
        a.check("33. content", !testee.isSame(*list[i]));
    }

    // loadGameRoot
    LoadTask result;
    std::auto_ptr<LoadGameRootTask_t> inTask(LoadGameRootTask_t::makeBound(&result, &LoadTask::keep));
    std::auto_ptr<game::Task_t> outTask(testee.loadGameRoot(*config, inTask));

    a.checkNull("41. inTask", inTask.get());
    a.checkNonNull("42. outTask", outTask.get());
    outTask->call();
    a.check("43. called", result.called);
    a.checkNull("44. result", result.result.get());
}
