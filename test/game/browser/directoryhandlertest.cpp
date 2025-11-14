/**
  *  \file test/game/browser/directoryhandlertest.cpp
  *  \brief Test for game::browser::DirectoryHandler
  */

#include "game/browser/directoryhandler.hpp"

#include "afl/io/directory.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/browser/accountmanager.hpp"
#include "game/browser/browser.hpp"
#include "game/browser/optionalusercallback.hpp"
#include "game/test/files.hpp"
#include "util/profiledirectory.hpp"

using afl::base::Ref;
using afl::container::PtrVector;
using afl::io::Directory;
using afl::io::FileSystem;
using afl::io::InternalDirectory;
using afl::io::InternalFileSystem;
using afl::string::NullTranslator;
using afl::sys::InternalEnvironment;
using afl::sys::Log;
using game::browser::Account;
using game::browser::AccountManager;
using game::browser::Browser;
using game::browser::DirectoryHandler;
using game::browser::Folder;
using game::browser::LoadGameRootTask_t;
using game::browser::OptionalUserCallback;
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

    Ref<Directory> createGameDirectory()
    {
        Ref<Directory> dir = InternalDirectory::create("dir");
        dir->openFile("player7.rst", FileSystem::Create)
            ->fullWrite(game::test::getResultFile30());
        dir->openFile("race.nm", FileSystem::Create)
            ->fullWrite(game::test::getDefaultRaceNames());
        return dir;
    }

    struct LoadTask {
        afl::base::Ptr<game::Root> result;

        void keep(afl::base::Ptr<game::Root> p)
            { result = p; }
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
        Ref<InternalDirectory> dir;

        BrowserEnvironment()
            : env(), fs(), profile(prepareEnvironment(env), prepareFileSystem(fs)),
              tx(), log(), accounts(profile, tx, log),
              browser(fs, tx, log, accounts, profile, callback),
              dir(InternalDirectory::create("spec"))
            { }
    };
}

// handleFolderName, success case: directory exists
AFL_TEST("game.browser.DirectoryHandler:handleFolderName:success", a)
{
    BrowserEnvironment env;
    DirectoryHandler testee(env.browser, env.dir, env.profile);

    env.fs.createDirectory("/foo");
    env.fs.createDirectory("/foo/bar");
    env.fs.createDirectory("/foo/bar/baz");

    PtrVector<Folder> result;
    bool ok = testee.handleFolderName("/foo/bar/baz", result);

    a.check("01. ok", ok);
    a.checkEqual("02. size", result.size(), 5U);

    // Index 0: 'My Computer'
    // Index 1: 'Root'
    a.checkEqual("11. foo", result[2]->getName(), "foo");
    a.checkEqual("12. foo", result[3]->getName(), "bar");
    a.checkEqual("13. foo", result[4]->getName(), "baz");
}

// handleFolderName, error case: directory does not exist
AFL_TEST("game.browser.DirectoryHandler:handleFolderName:missing", a)
{
    BrowserEnvironment env;
    DirectoryHandler testee(env.browser, env.dir, env.profile);

    PtrVector<Folder> result;
    bool ok = testee.handleFolderName("/foo/bar/baz", result);

    a.check("01. ok", !ok);
    a.checkEqual("02. size", result.size(), 0U);
}

// createAccountFolder
AFL_TEST("game.browser.DirectoryHandler:createAccountFolder", a)
{
    BrowserEnvironment env;
    DirectoryHandler testee(env.browser, env.dir, env.profile);

    Ref<Account> acc = Account::create();
    acc->setUser("u");
    acc->setHost("h");
    acc->setType("t");

    std::auto_ptr<Folder> result(testee.createAccountFolder(acc));
    a.checkNull("01. result", result.get());
}

// loadGameRootMaybe, empty configuration, no file: task is created, but produces no result.
AFL_TEST("game.browser.DirectoryHandler:loadGameRootMaybe:none", a)
{
    BrowserEnvironment env;
    DirectoryHandler testee(env.browser, env.dir, env.profile);

    // Empty game directory
    Ref<Directory> dir = InternalDirectory::create("dir");

    // Empty user configuration
    Ref<UserConfiguration> config = UserConfiguration::create();

    LoadTask loader;
    std::auto_ptr<LoadGameRootTask_t> inTask(LoadGameRootTask_t::makeBound(&loader, &LoadTask::keep));
    std::auto_ptr<game::Task_t> outTask(testee.loadGameRootMaybe(dir, *config, inTask));

    // Task created
    a.checkNull("01. inTask", inTask.get());
    a.checkNonNull("02. outTask", outTask.get());

    // No result
    outTask->call();
    a.checkNull("11. loader", loader.result.get());
}

// loadGameRootMaybe, empty configuration, populated game directory: task is created and produces result.
AFL_TEST("game.browser.DirectoryHandler:loadGameRootMaybe:empty", a)
{
    BrowserEnvironment env;
    DirectoryHandler testee(env.browser, env.dir, env.profile);

    // Game directory
    Ref<Directory> dir = createGameDirectory();

    // Empty user configuration
    Ref<UserConfiguration> config = UserConfiguration::create();

    LoadTask loader;
    std::auto_ptr<LoadGameRootTask_t> inTask(LoadGameRootTask_t::makeBound(&loader, &LoadTask::keep));
    std::auto_ptr<game::Task_t> outTask(testee.loadGameRootMaybe(dir, *config, inTask));

    // Task created
    a.checkNull("01. inTask", inTask.get());
    a.checkNonNull("02. outTask", outTask.get());

    // Result
    outTask->call();
    a.checkNonNull("11. loader", loader.result.get());
}

// loadGameRootMaybe, explicitly configured as local: task is created and produces result
AFL_TEST("game.browser.DirectoryHandler:loadGameRootMaybe:local", a)
{
    BrowserEnvironment env;
    DirectoryHandler testee(env.browser, env.dir, env.profile);

    // Game directory
    Ref<Directory> dir = createGameDirectory();

    // User configuration
    Ref<UserConfiguration> config = UserConfiguration::create();
    (*config)[UserConfiguration::Game_Type].set("local");

    LoadTask loader;
    std::auto_ptr<LoadGameRootTask_t> inTask(LoadGameRootTask_t::makeBound(&loader, &LoadTask::keep));
    std::auto_ptr<game::Task_t> outTask(testee.loadGameRootMaybe(dir, *config, inTask));

    // Task created
    a.checkNull("01. inTask", inTask.get());
    a.checkNonNull("02. outTask", outTask.get());

    // Result
    outTask->call();
    a.checkNonNull("11. loader", loader.result.get());
}

// loadGameRootMaybe, configured as remote game. No task is created; remote DirectoryHandler should pick this up.
AFL_TEST("game.browser.DirectoryHandler:loadGameRootMaybe:remote", a)
{
    BrowserEnvironment env;
    DirectoryHandler testee(env.browser, env.dir, env.profile);

    // Game directory
    Ref<Directory> dir = createGameDirectory();

    // User configuration
    Ref<UserConfiguration> config = UserConfiguration::create();
    (*config)[UserConfiguration::Game_Type].set("remote");

    LoadTask loader;
    std::auto_ptr<LoadGameRootTask_t> inTask(LoadGameRootTask_t::makeBound(&loader, &LoadTask::keep));
    std::auto_ptr<game::Task_t> outTask(testee.loadGameRootMaybe(dir, *config, inTask));

    // No task created
    a.checkNonNull("01. inTask", inTask.get());
    a.checkNull("02. outTask", outTask.get());
}
