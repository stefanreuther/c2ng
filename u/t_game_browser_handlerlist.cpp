/**
  *  \file u/t_game_browser_handlerlist.cpp
  *  \brief Test for game::browser::HandlerList
  */

#include "game/browser/handlerlist.hpp"

#include "t_game_browser.hpp"
#include "game/browser/folder.hpp"
#include "game/browser/account.hpp"
#include "afl/io/internaldirectory.hpp"

namespace {
    void dummy(afl::base::Ptr<game::Root>)
    { }
}

/** Ultra-simple test. */
void
TestGameBrowserHandlerList::testIt()
{
    class Tester : public game::browser::Handler {
     public:
        virtual bool handleFolderName(String_t /*name*/, afl::container::PtrVector<game::browser::Folder>& /*result*/)
            { return false; }
        virtual game::browser::Folder* createAccountFolder(game::browser::Account& /*acc*/)
            { return 0; }
        virtual std::auto_ptr<game::Task_t> loadGameRootMaybe(afl::base::Ref<afl::io::Directory> /*dir*/, const game::config::UserConfiguration& /*config*/, std::auto_ptr<game::browser::LoadGameRootTask_t>& /*then*/)
            { return std::auto_ptr<game::Task_t>(); }
    };

    game::browser::HandlerList testee;
    game::browser::Account acc;
    afl::container::PtrVector<game::browser::Folder> result;
    const game::config::UserConfiguration uc;
    afl::base::Ref<afl::io::Directory> dir(afl::io::InternalDirectory::create("test"));
    std::auto_ptr<game::browser::LoadGameRootTask_t> then = std::auto_ptr<game::browser::LoadGameRootTask_t>(game::browser::LoadGameRootTask_t::makeStatic(dummy));
    TS_ASSERT(!testee.handleFolderName("foo", result));
    TS_ASSERT(testee.createAccountFolder(acc) == 0);
    TS_ASSERT(testee.loadGameRootMaybe(dir, uc, then).get() == 0);
    TS_ASSERT(then.get() != 0);

    testee.addNewHandler(new Tester());
    TS_ASSERT(!testee.handleFolderName("foo", result));
    TS_ASSERT(testee.createAccountFolder(acc) == 0);
    TS_ASSERT(testee.loadGameRootMaybe(dir, uc, then).get() == 0);
    TS_ASSERT(then.get() != 0);
}

