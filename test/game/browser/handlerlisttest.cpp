/**
  *  \file test/game/browser/handlerlisttest.cpp
  *  \brief Test for game::browser::HandlerList
  */

#include "game/browser/handlerlist.hpp"

#include "afl/io/internaldirectory.hpp"
#include "afl/test/testrunner.hpp"
#include "game/browser/account.hpp"
#include "game/browser/folder.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::container::PtrVector;
using afl::io::Directory;
using afl::io::InternalDirectory;
using game::Task_t;
using game::browser::Account;
using game::browser::Folder;
using game::browser::Handler;
using game::browser::HandlerList;
using game::browser::LoadGameRootTask_t;
using game::config::UserConfiguration;

namespace {
    void dummy(Ptr<game::Root>)
    { }
}

/** Ultra-simple test. */
AFL_TEST("game.browser.HandlerList:simple", a)
{
    class Tester : public Handler {
     public:
        virtual bool handleFolderName(String_t /*name*/, PtrVector<Folder>& /*result*/)
            { return false; }
        virtual Folder* createAccountFolder(const Ref<Account>& /*acc*/)
            { return 0; }
        virtual std::auto_ptr<Task_t> loadGameRootMaybe(Ref<Directory> /*dir*/, const UserConfiguration& /*config*/, std::auto_ptr<LoadGameRootTask_t>& /*then*/)
            { return std::auto_ptr<Task_t>(); }
    };

    HandlerList testee;
    Ref<Account> acc = Account::create();
    PtrVector<Folder> result;
    Ref<const UserConfiguration> uc = UserConfiguration::create();
    Ref<Directory> dir(InternalDirectory::create("test"));
    std::auto_ptr<LoadGameRootTask_t> then = std::auto_ptr<LoadGameRootTask_t>(LoadGameRootTask_t::makeStatic(dummy));
    a.check("01. handleFolderName", !testee.handleFolderName("foo", result));
    a.checkNull("02. createAccountFolder", testee.createAccountFolder(acc));
    a.checkNull("03. loadGameRootMaybe", testee.loadGameRootMaybe(dir, *uc, then).get());
    a.checkNonNull("04. then", then.get());

    testee.addNewHandler(new Tester());
    a.check("11. handleFolderName", !testee.handleFolderName("foo", result));
    a.checkNull("12. createAccountFolder", testee.createAccountFolder(acc));
    a.checkNull("13. loadGameRootMaybe", testee.loadGameRootMaybe(dir, *uc, then).get());
    a.checkNonNull("14. then", then.get());
}

/** Test the success cases. */
AFL_TEST("game.browser.HandlerList:success", a)
{
    class DummyFolder : public Folder {
     public:
        virtual std::auto_ptr<Task_t> loadContent(std::auto_ptr<game::browser::LoadContentTask_t> /*then*/)
            { return std::auto_ptr<Task_t>(); }
        virtual bool loadConfiguration(UserConfiguration&)
            { return false; }
        virtual void saveConfiguration(const UserConfiguration&)
            { }
        virtual bool setLocalDirectoryName(String_t)
            { return false; }
        virtual std::auto_ptr<Task_t> loadGameRoot(const UserConfiguration& /*config*/, std::auto_ptr<LoadGameRootTask_t> /*then*/)
            { return std::auto_ptr<Task_t>(); }
        virtual String_t getName() const
            { return String_t(); }
        virtual util::rich::Text getDescription() const
            { return ""; }
        virtual bool isSame(const Folder&) const
            { return true; }
        virtual bool canEnter() const
            { return false; }
        virtual Kind getKind() const
            { return kFavorite; }
    };

    class DummyTask : public Task_t {
     public:
        DummyTask(std::auto_ptr<LoadGameRootTask_t>& t)
            : m_task(t)
            { }
        void call()
            { m_task->call(0); }
     private:
        std::auto_ptr<LoadGameRootTask_t> m_task;
    };

    class Tester : public Handler {
     public:
        virtual bool handleFolderName(String_t name, PtrVector<Folder>& result)
            {
                if (name == "test-url") {
                    for (int i = 0; i < 10; ++i) {
                        result.pushBackNew(new DummyFolder());
                    }
                    return true;
                } else {
                    return false;
                }
            }
        virtual Folder* createAccountFolder(const Ref<Account>& acc)
            {
                if (acc->getType() == "test-type") {
                    return new DummyFolder();
                } else {
                    return 0;
                }
            }
        virtual std::auto_ptr<Task_t> loadGameRootMaybe(Ref<Directory> /*dir*/, const UserConfiguration& config, std::auto_ptr<LoadGameRootTask_t>& then)
            {
                if (config.getGameType() == "test-type") {
                    return std::auto_ptr<Task_t>(new DummyTask(then));
                } else {
                    return std::auto_ptr<Task_t>();
                }
            }
    };

    class FlagTask : public LoadGameRootTask_t {
     public:
        FlagTask(bool& flag)
            : m_flag(flag)
            { }
        void call(Ptr<game::Root>)
            { m_flag = true; }
     private:
        bool& m_flag;
    };

    HandlerList testee;
    testee.addNewHandler(new Tester());

    // Successful handleFolderName()
    PtrVector<Folder> result;
    a.check("01. handleFolderName", testee.handleFolderName("test-url", result));
    a.checkEqual("02. result", result.size(), 10U);

    // Successful createAccountFolder
    Ref<Account> acc = Account::create();
    acc->setType("test-type");
    std::auto_ptr<Folder> folder(testee.createAccountFolder(acc));
    a.checkNonNull("11. createAccountFolder", folder.get());

    // Successful loadGameRootMaybe
    Ref<Directory> dir(InternalDirectory::create("test"));
    Ref<UserConfiguration> uc = UserConfiguration::create();
    (*uc)[UserConfiguration::Game_Type].set("test-type");
    bool flag = false;
    std::auto_ptr<LoadGameRootTask_t> then(new FlagTask(flag));
    std::auto_ptr<Task_t> task(testee.loadGameRootMaybe(dir, *uc, then));
    a.checkNonNull("21. loadGameRootMaybe", task.get());
    a.checkNull("22. then", then.get());

    task->call();
    a.check("22. call", flag);
}
