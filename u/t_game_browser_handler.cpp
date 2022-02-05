/**
  *  \file u/t_game_browser_handler.cpp
  *  \brief Test for game::browser::Handler
  */

#include "game/browser/handler.hpp"

#include "t_game_browser.hpp"

/** Interface test. */
void
TestGameBrowserHandler::testIt()
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
    Tester t;
}

