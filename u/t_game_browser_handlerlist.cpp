/**
  *  \file u/t_game_browser_handlerlist.cpp
  *  \brief Test for game::browser::HandlerList
  */

#include "game/browser/handlerlist.hpp"

#include "t_game_browser.hpp"
#include "game/browser/folder.hpp"
#include "game/browser/account.hpp"
#include "afl/io/internaldirectory.hpp"

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
        virtual afl::base::Ptr<game::Root> loadGameRoot(afl::base::Ptr<afl::io::Directory> /*dir*/)
            { return 0; }
    };

    game::browser::HandlerList testee;
    game::browser::Account acc;
    afl::container::PtrVector<game::browser::Folder> result;
    afl::base::Ptr<afl::io::Directory> dir(afl::io::InternalDirectory::create("test"));
    TS_ASSERT(!testee.handleFolderName("foo", result));
    TS_ASSERT(testee.createAccountFolder(acc) == 0);
    TS_ASSERT(testee.loadGameRoot(dir).get() == 0);

    testee.addNewHandler(new Tester());
    TS_ASSERT(!testee.handleFolderName("foo", result));
    TS_ASSERT(testee.createAccountFolder(acc) == 0);
    TS_ASSERT(testee.loadGameRoot(dir).get() == 0);
}

