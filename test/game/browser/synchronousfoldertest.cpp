/**
  *  \file test/game/browser/synchronousfoldertest.cpp
  *  \brief Test for game::browser::SynchronousFolder
  */

#include "game/browser/synchronousfolder.hpp"

#include "afl/test/testrunner.hpp"

using game::browser::Folder;
using game::browser::LoadContentTask_t;

/** Interface test. */
AFL_TEST("game.browser.SynchronousFolder", a)
{
    static const size_t N = 17;
    typedef afl::container::PtrVector<Folder> Result_t;

    class TestFolder : public game::browser::SynchronousFolder {
     public:
        // Synchronous implementation:
        virtual void loadContent(afl::container::PtrVector<Folder>& result)
            {
                for (size_t i = 0; i < N; ++i) {
                    result.pushBackNew(new TestFolder());
                }
            }

        // All the other methods:
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

    // Implementation of LoadContentTask_t
    class Response : public LoadContentTask_t {
     public:
        Response(Result_t& savedResult)
            : m_savedResult(savedResult)
            { }
        void call(Result_t& result)
            { result.swap(m_savedResult); }
     private:
        Result_t& m_savedResult;
    };

    // Test
    Result_t savedResult;
    TestFolder testee;
    Folder& f = testee;

    std::auto_ptr<game::Task_t> t(f.loadContent(std::auto_ptr<LoadContentTask_t>(new Response(savedResult))));
    a.checkNonNull("01. task", t.get());

    t->call();
    a.checkEqual("11. size", savedResult.size(), N);
}
