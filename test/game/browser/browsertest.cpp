/**
  *  \file test/game/browser/browsertest.cpp
  *  \brief Test for game::browser::Browser
  */

#include "game/browser/browser.hpp"

#include "afl/container/ptrvector.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/browser/accountmanager.hpp"
#include "game/browser/browser.hpp"
#include "game/browser/directoryhandler.hpp"
#include "game/browser/optionalusercallback.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/hostversion.hpp"
#include "game/test/files.hpp"
#include "game/test/root.hpp"
#include "game/turnloader.hpp"
#include "util/profiledirectory.hpp"

using afl::base::Ref;
using afl::container::PtrVector;
using afl::io::FileSystem;
using afl::io::InternalDirectory;
using afl::io::InternalFileSystem;
using afl::io::Stream;
using afl::string::Format;
using afl::string::NullTranslator;
using afl::sys::InternalEnvironment;
using afl::sys::Log;
using game::HostVersion;
using game::Root;
using game::TurnLoader;
using game::browser::Account;
using game::browser::AccountManager;
using game::browser::Browser;
using game::browser::DirectoryHandler;
using game::browser::Folder;
using game::browser::LoadGameRootTask_t;
using game::browser::OptionalUserCallback;
using game::config::ConfigurationOption;
using game::config::HostConfiguration;
using game::config::UserConfiguration;
using util::ProfileDirectory;

namespace {
    /*
     *  Tasks
     */

    struct LoadTask {
        bool called;
        afl::base::Ptr<game::Root> result;

        LoadTask()
            : called(), result()
            { }
        void keep(afl::base::Ptr<game::Root> p)
            { result = p; called = true; }
    };

    // Create a task that stores 'true' in the given flag, to track that it was called
    std::auto_ptr<game::Task_t> makeTrackerTask(bool& flag)
    {
        return game::makeConfirmationTask(true, game::makeResultTask(flag));
    }

    /*
     *  Environment
     */

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

    /*
     *  Simulated remote game
     */

    Ref<Account> makeAccount()
    {
        Ref<Account> acc = Account::create();
        acc->setUser("u");
        acc->setType("t");
        acc->setHost("h");
        return acc;
    }

    std::auto_ptr<game::Task_t> makeRoot(std::auto_ptr<LoadGameRootTask_t>& then)
    {
        class Task : public game::Task_t {
         public:
            Task(std::auto_ptr<LoadGameRootTask_t>& then)
                : m_then(then)
                { }
            void call()
                {
                    afl::base::Ref<Root> root = game::test::makeRoot(HostVersion());
                    root->hostConfiguration()[HostConfiguration::GameName].set("TestRemote");
                    m_then->call(root.asPtr());
                }
         private:
            std::auto_ptr<LoadGameRootTask_t> m_then;
        };
        return std::auto_ptr<game::Task_t>(new Task(then));
    }

    class TestRemoteFolder : public game::browser::SynchronousFolder {
     public:
        TestRemoteFolder(String_t& local, String_t name)
            : m_local(local), m_name(name)
            { }
        virtual void loadContent(afl::container::PtrVector<Folder>& /*result*/)
            { }
        virtual bool loadConfiguration(UserConfiguration& /*config*/)
            { return true; }
        virtual void saveConfiguration(const UserConfiguration& /*config*/)
            { }
        virtual bool setLocalDirectoryName(String_t directoryName)
            { m_local = directoryName; return true; }
        virtual std::auto_ptr<game::Task_t> loadGameRoot(const UserConfiguration& /*config*/, std::auto_ptr<LoadGameRootTask_t> then)
            { return makeRoot(then); }
        virtual String_t getName() const
            { return m_name; }
        virtual util::rich::Text getDescription() const
            { return "desc"; }
        virtual bool isSame(const Folder& other) const
            { return dynamic_cast<const TestRemoteFolder*>(&other) != 0; }
        virtual bool canEnter() const
            { return false; }
        virtual Kind getKind() const
            { return kGame; }
     private:
        String_t& m_local;
        String_t m_name;
    };

    class TestRemoteHandler : public game::browser::Handler {
     public:
        TestRemoteHandler(String_t& local, String_t name)
            : m_local(local), m_name(name)
            { }
        virtual bool handleFolderName(String_t /*name*/, afl::container::PtrVector<Folder>& /*result*/)
            { return false; }
        virtual Folder* createAccountFolder(const afl::base::Ref<Account>& /*acc*/)
            { return new TestRemoteFolder(m_local, m_name); }
        virtual std::auto_ptr<game::Task_t> loadGameRootMaybe(afl::base::Ref<afl::io::Directory> /*dir*/, const UserConfiguration& /*config*/, std::auto_ptr<LoadGameRootTask_t>& then)
            { return makeRoot(then); }
     private:
        String_t& m_local;
        String_t m_name;
    };
}

// Basic accessor test
AFL_TEST("game.browser.Browser:accessors", a)
{
    BrowserEnvironment env;

    a.checkEqual("01. fileSystem", &env.fs, &env.browser.fileSystem());
    a.checkEqual("02. translator", &env.tx, &env.browser.translator());
    a.checkEqual("03. log",        &env.log, &env.browser.log());
    a.checkEqual("04. accounts",   &env.accounts, &env.browser.accounts());
    a.checkEqual("05. callback",   &env.callback, &env.browser.callback());
    a.checkEqual("06. profile",    &env.profile, &env.browser.profile());

    a.checkEqual("11. expand", env.browser.expandGameDirectoryName("/foo"), "/foo");
    a.checkEqual("12. expand", env.browser.expandGameDirectoryName("game:foo"), "/home/games/foo");
}

// Browsing sequence
AFL_TEST("game.browser.Browser:browse-sequence", a)
{
    Ref<InternalDirectory> spec = InternalDirectory::create("spec");
    spec->openFile("race.nm", FileSystem::Create)
        ->fullWrite(game::test::getDefaultRaceNames());
    BrowserEnvironment env;
    env.fs.createDirectory("/sub");
    env.fs.createDirectory("/sub/one");
    env.fs.createDirectory("/sub/two");
    env.fs.createDirectory("/sub/two/more");
    env.browser.addNewHandler(new DirectoryHandler(env.browser, spec, env.profile));

    // We can open this folder
    a.check("01. openFolder", env.browser.openFolder("/sub"));

    // Load its content
    bool loaded = false;
    env.browser.loadContent(makeTrackerTask(loaded))->call();
    a.check("02. loaded", loaded);

    // Verify content
    a.checkEqual("11. name", env.browser.currentFolder().getName(), "sub");
    a.checkEqual("12. path", env.browser.path().size(), 3U);           // Virtual fs root, "/", "/sub"
    a.checkEqual("13. content", env.browser.content().size(), 2U);
    a.checkEqual("14. content", env.browser.content()[0]->getName(), "one");
    a.checkEqual("15. content", env.browser.content()[1]->getName(), "two");

    a.checkNull("16. child", env.browser.getSelectedChild());
    a.check("17. child", !env.browser.getSelectedChildIndex().isValid());

    // Select child
    env.browser.selectChild(1);
    a.checkNonNull("21. child", env.browser.getSelectedChild());
    a.checkEqual("22. child", env.browser.getSelectedChildIndex().orElse(99), 1U);

    // Load child
    bool loaded2 = false;
    env.browser.openChild(1);
    env.browser.loadContent(makeTrackerTask(loaded2))->call();
    a.check("23. loaded", loaded2);

    // Verify content
    a.checkEqual("31. name", env.browser.currentFolder().getName(), "two");
    a.checkEqual("32. path", env.browser.path().size(), 4U);           // Virtual fs root, "/", "/sub", "two"
    a.checkEqual("33. content", env.browser.content().size(), 1U);
    a.checkEqual("34. content", env.browser.content()[0]->getName(), "more");

    a.checkNull("36. child", env.browser.getSelectedChild());
    a.check("37. child", !env.browser.getSelectedChildIndex().isValid());

    // Go back
    bool loaded3 = false;
    env.browser.openParent();
    env.browser.loadContent(makeTrackerTask(loaded3))->call();
    a.check("41. loaded", loaded3);
    a.checkNonNull("42. child", env.browser.getSelectedChild());
    a.checkEqual("43. child", env.browser.getSelectedChildIndex().orElse(99), 1U);
}

// Browsing sequence including reload operation
AFL_TEST("game.browser.Browser:reload-sequence", a)
{
    Ref<InternalDirectory> spec = InternalDirectory::create("spec");
    spec->openFile("race.nm", FileSystem::Create)
        ->fullWrite(game::test::getDefaultRaceNames());
    BrowserEnvironment env;
    env.fs.createDirectory("/sub");
    env.fs.createDirectory("/sub/x1");
    env.fs.createDirectory("/sub/x2");
    env.fs.createDirectory("/sub/x3");
    env.browser.addNewHandler(new DirectoryHandler(env.browser, spec, env.profile));

    // Open a folder
    a.check("01. openFolder", env.browser.openFolder("/sub"));

    // Load its content
    bool loaded = false;
    env.browser.loadContent(makeTrackerTask(loaded))->call();
    a.check("02. loaded", loaded);
    a.checkEqual("03. content", env.browser.content().size(), 3U);
    env.browser.selectChild(2);

    // Reload
    bool loaded2 = false;
    env.browser.loadContent(makeTrackerTask(loaded2))->call();
    a.check("11. loaded", loaded2);

    // Origin folder still selected
    // (This is the "selected element, but not a previous path element" case.)
    a.checkNonNull("12. child", env.browser.getSelectedChild());
    a.checkEqual("13. child", env.browser.getSelectedChildIndex().orElse(99), 2U);
}

// Test openFolder() using nonexistant folder
AFL_TEST("game.browser.Browser:openFolder:fails", a)
{
    Ref<InternalDirectory> spec = InternalDirectory::create("spec");
    spec->openFile("race.nm", FileSystem::Create)
        ->fullWrite(game::test::getDefaultRaceNames());
    BrowserEnvironment env;
    env.browser.addNewHandler(new DirectoryHandler(env.browser, spec, env.profile));

    // Opening nonexistant folder is correctly rejected
    a.check("01. openFolder", !env.browser.openFolder("/nonexistant"));
}

// Test loadContent() and configuration modification
AFL_TEST("game.browser.Browser:config", a)
{
    Ref<InternalDirectory> spec = InternalDirectory::create("spec");
    spec->openFile("race.nm", FileSystem::Create)
        ->fullWrite(game::test::getDefaultRaceNames());
    BrowserEnvironment env;
    env.fs.createDirectory("/dir");
    env.fs.createDirectory("/dir/sub");
    env.fs.openFile("/dir/sub/player7.rst", FileSystem::Create)
        ->fullWrite(game::test::getResultFile30());
    env.browser.addNewHandler(new DirectoryHandler(env.browser, spec, env.profile));

    // Open directory
    a.check("01. openFolder", env.browser.openFolder("/dir"));

    // Load its content
    bool loaded = false;
    env.browser.loadContent(makeTrackerTask(loaded))->call();
    a.check("02. loaded", loaded);
    a.checkEqual("03. content", env.browser.content().size(), 1U);
    env.browser.selectChild(0);

    // Load root (exercises loadGameRoot)
    bool loaded2 = false;
    env.browser.loadChildRoot(makeTrackerTask(loaded2))->call();
    a.check("11. loaded", loaded2);
    a.checkNonNull("12. root", env.browser.getSelectedRoot().get());
    a.checkNonNull("13. turn", env.browser.getSelectedRoot()->getTurnLoader().get());

    String_t extra;
    a.check("14. status", env.browser.getSelectedRoot()->getTurnLoader()->getPlayerStatus(7, extra, env.tx).contains(TurnLoader::Available));

    // Verify presence of configuration
    a.checkNonNull("21. config", env.browser.getSelectedConfiguration());

    // Update configuration
    UserConfiguration& config = *env.browser.getSelectedConfiguration();
    config[UserConfiguration::ExportShipFields].set("name,owner");
    config[UserConfiguration::ExportShipFields].setSource(ConfigurationOption::Game);

    bool saved = false;
    env.browser.updateConfiguration(makeTrackerTask(saved))->call();
    a.check("31. updated", saved);

    Ref<Stream> in = env.fs.openFile("/dir/sub/pcc2.ini", FileSystem::OpenRead);
    String_t content = afl::string::fromBytes(in->createVirtualMapping()->get());
    a.check("32. contains", content.find("Export.ShipFields = name,owner") != String_t::npos);
}

// Test handling server directory: setSelectedLocalDirectoryName
AFL_TEST("game.browser.Browser:setSelectedLocalDirectoryName", a)
{
    String_t local;
    BrowserEnvironment env;
    env.accounts.addNewAccount(makeAccount());
    env.browser.addNewHandler(new TestRemoteHandler(local, "remote game name"));

    // Load root
    bool loaded = false;
    env.browser.loadContent(makeTrackerTask(loaded))->call();
    a.check("01. loaded", loaded);
    a.check("02. count", env.browser.content().size() >= 2U);

    // Select last file
    env.browser.selectChild(env.browser.content().size()-1);

    // Configure
    env.browser.setSelectedLocalDirectoryName("/foo");
    a.checkEqual("11. dir", local, "/foo");
}

// Test handling server directory: setSelectedLocalDirectoryAutomatically
AFL_TEST("game.browser.Browser:setSelectedLocalDirectoryAutomatically", a)
{
    String_t local;
    BrowserEnvironment env;
    env.accounts.addNewAccount(makeAccount());
    env.browser.addNewHandler(new TestRemoteHandler(local, "remote game name (42)"));

    // Load root
    bool loaded = false;
    env.browser.loadContent(makeTrackerTask(loaded))->call();
    a.check("01. loaded", loaded);
    a.check("02. count", env.browser.content().size() >= 2U);

    // Select last file
    env.browser.selectChild(env.browser.content().size()-1);

    // Configure repeatedly
    // We do not persist the directory name. Thus, each subsequent call conflicts with the previous one.
    env.browser.setSelectedLocalDirectoryAutomatically();
    a.checkEqual("11. dir", local, "game:remote_game_name");
    AFL_CHECK_SUCCEEDS(a("12. exists"), env.fs.openDirectory("/home/games/remote_game_name"));

    env.browser.setSelectedLocalDirectoryAutomatically();
    a.checkEqual("21. dir", local, "game:remote_game_name_42");
    AFL_CHECK_SUCCEEDS(a("22. exists"), env.fs.openDirectory("/home/games/remote_game_name_42"));

    env.browser.setSelectedLocalDirectoryAutomatically();
    a.checkEqual("31. dir", local, "game:remote_game_name_42_1");
    AFL_CHECK_SUCCEEDS(a("32. exists"), env.fs.openDirectory("/home/games/remote_game_name_42_1"));

    env.browser.setSelectedLocalDirectoryAutomatically();
    a.checkEqual("41. dir", local, "game:remote_game_name_42_2");
    AFL_CHECK_SUCCEEDS(a("42. exists"), env.fs.openDirectory("/home/games/remote_game_name_42_2"));
}

// Test handling server directory: setSelectedLocalDirectoryAutomatically (variant, slash in game name)
AFL_TEST("game.browser.Browser:setSelectedLocalDirectoryAutomatically:slash", a)
{
    String_t local;
    BrowserEnvironment env;
    env.accounts.addNewAccount(makeAccount());
    env.browser.addNewHandler(new TestRemoteHandler(local, "dir/game name"));

    // Load root
    bool loaded = false;
    env.browser.loadContent(makeTrackerTask(loaded))->call();
    a.check("01. loaded", loaded);
    a.check("02. count", env.browser.content().size() >= 2U);

    // Select last file
    env.browser.selectChild(env.browser.content().size()-1);

    // Configure repeatedly
    // We do not persist the directory name. Thus, each subsequent call conflicts with the previous one.
    env.browser.setSelectedLocalDirectoryAutomatically();
    a.checkEqual("11. dir", local, "game:game_name");
    AFL_CHECK_SUCCEEDS(a("12. exists"), env.fs.openDirectory("/home/games/game_name"));
}

// Test handling server directory: setSelectedLocalDirectoryAutomatically (variant, special characters)
AFL_TEST("game.browser.Browser:setSelectedLocalDirectoryAutomatically:special", a)
{
    String_t local;
    BrowserEnvironment env;
    env.accounts.addNewAccount(makeAccount());
    env.browser.addNewHandler(new TestRemoteHandler(local, "Game \"Nick\" Name"));

    // Load root
    bool loaded = false;
    env.browser.loadContent(makeTrackerTask(loaded))->call();
    a.check("01. loaded", loaded);
    a.check("02. count", env.browser.content().size() >= 2U);

    // Select last file
    env.browser.selectChild(env.browser.content().size()-1);

    // Configure repeatedly
    // We do not persist the directory name. Thus, each subsequent call conflicts with the previous one.
    env.browser.setSelectedLocalDirectoryAutomatically();
    a.checkEqual("11. dir", local, "game:game_nick_name");
    AFL_CHECK_SUCCEEDS(a("12. exists"), env.fs.openDirectory("/home/games/game_nick_name"));
}

// Test operation without a Handler.
// This exercises all the fallback cases where no Handler does anything.
AFL_TEST("game.browser.Browser:no-handler", a)
{
    BrowserEnvironment env;

    // Cannot open folders
    a.check("01. openFolder", !env.browser.openFolder("/"));

    // Cannot load roots (but still consumes task)
    Ref<UserConfiguration> config = UserConfiguration::create();
    LoadTask result;
    std::auto_ptr<LoadGameRootTask_t> inTask(LoadGameRootTask_t::makeBound(&result, &LoadTask::keep));
    std::auto_ptr<game::Task_t> outTask(env.browser.loadGameRoot(InternalDirectory::create("dir"), *config, inTask));
    a.checkNull("11. inTask", inTask.get());
    a.checkNonNull("12. outTask", outTask.get());

    outTask->call();
    a.check("13. called", result.called);
    a.checkNull("14. result", result.result.get());

    // createAccountFolder will create a dummy
    Ref<Account> acc = Account::create();
    acc->setUser("u");
    acc->setHost("h");
    acc->setType("t");
    acc->setName("nn");
    std::auto_ptr<Folder> p(env.browser.createAccountFolder(acc));
    a.checkNonNull("21. account", p.get());
    a.checkEqual("22. name", p->getName(), "nn");
}

AFL_TEST("game.browser.Browser:verifyLocalDirectory:ok", a)
{
    BrowserEnvironment env;
    env.fs.createDirectory("/foo");

    a.checkEqual("result", env.browser.verifyLocalDirectory("/foo"), Browser::Success);
}

AFL_TEST("game.browser.Browser:verifyLocalDirectory:missing", a)
{
    BrowserEnvironment env;

    a.checkEqual("result", env.browser.verifyLocalDirectory("/foo"), Browser::Missing);
}

AFL_TEST("game.browser.Browser:verifyLocalDirectory:not-empty", a)
{
    BrowserEnvironment env;
    env.fs.createDirectory("/foo");
    env.fs.openFile("/foo/bar", FileSystem::Create);

    a.checkEqual("result", env.browser.verifyLocalDirectory("/foo"), Browser::NotEmpty);
}
