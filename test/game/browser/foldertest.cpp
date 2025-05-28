/**
  *  \file test/game/browser/foldertest.cpp
  *  \brief Test for game::browser::Folder
  */

#include "game/browser/folder.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.browser.Folder:interface")
{
    class Tester : public game::browser::Folder {
     public:
        virtual std::auto_ptr<game::Task_t> loadContent(std::auto_ptr<game::browser::LoadContentTask_t> /*then*/)
            { return std::auto_ptr<game::Task_t>(); }
        virtual bool loadConfiguration(game::config::UserConfiguration&)
            { return false; }
        virtual void saveConfiguration(const game::config::UserConfiguration&)
            { }
        virtual bool setLocalDirectoryName(String_t)
            { return false; }
        virtual std::auto_ptr<game::Task_t> loadGameRoot(const game::config::UserConfiguration& /*config*/, std::auto_ptr<game::browser::LoadGameRootTask_t> /*then*/)
            { return std::auto_ptr<game::Task_t>(); }
        virtual String_t getName() const
            { return String_t(); }
        virtual util::rich::Text getDescription() const
            { return ""; }
        virtual bool isSame(const game::browser::Folder&) const
            { return true; }
        virtual bool canEnter() const
            { return false; }
        virtual Kind getKind() const
            { return kFavorite; }
    };
    Tester t;
}

/** Test defaultLoadGameRoot. */
AFL_TEST("game.browser.Folder:defaultLoadGameRoot", a)
{
    class Task : public game::browser::LoadGameRootTask_t {
     public:
        Task(bool& flag)
            : m_flag(flag)
            { }
        void call(afl::base::Ptr<game::Root>)
            { m_flag = true; }
     private:
        bool& m_flag;
    };

    // Create
    bool flag = false;
    std::auto_ptr<game::Task_t> task(game::browser::Folder::defaultLoadGameRoot(std::auto_ptr<game::browser::LoadGameRootTask_t>(new Task(flag))));

    // Verify
    a.checkNonNull("01. task", task.get());

    a.check("11. before", !flag);
    task->call();
    a.check("12. after", flag);
}
