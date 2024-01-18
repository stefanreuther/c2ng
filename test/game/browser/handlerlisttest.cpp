/**
  *  \file test/game/browser/handlerlisttest.cpp
  *  \brief Test for game::browser::HandlerList
  */

#include "game/browser/handlerlist.hpp"

#include "afl/io/internaldirectory.hpp"
#include "afl/test/testrunner.hpp"
#include "game/browser/account.hpp"
#include "game/browser/folder.hpp"

namespace {
    void dummy(afl::base::Ptr<game::Root>)
    { }
}

/** Ultra-simple test. */
AFL_TEST("game.browser.HandlerList", a)
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
    a.check("01. handleFolderName", !testee.handleFolderName("foo", result));
    a.check("02. createAccountFolder", testee.createAccountFolder(acc) == 0);
    a.check("03. loadGameRootMaybe", testee.loadGameRootMaybe(dir, uc, then).get() == 0);
    a.check("04. then", then.get() != 0);

    testee.addNewHandler(new Tester());
    a.check("11. handleFolderName", !testee.handleFolderName("foo", result));
    a.check("12. createAccountFolder", testee.createAccountFolder(acc) == 0);
    a.check("13. loadGameRootMaybe", testee.loadGameRootMaybe(dir, uc, then).get() == 0);
    a.check("14. then", then.get() != 0);
}
