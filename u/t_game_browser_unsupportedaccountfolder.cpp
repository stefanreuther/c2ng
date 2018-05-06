/**
  *  \file u/t_game_browser_unsupportedaccountfolder.cpp
  *  \brief Test for game::browser::UnsupportedAccountFolder
  */

#include "game/browser/unsupportedaccountfolder.hpp"

#include "t_game_browser.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/browser/account.hpp"

/** Simple test.
    It's hard to test this class without repeating all the implementation,
    so this mainly tests we can instantiate the class. */
void
TestGameBrowserUnsupportedAccountFolder::testIt()
{
    afl::string::NullTranslator tx;
    game::browser::Account account;
    game::browser::UnsupportedAccountFolder testee(tx, account);
    const game::config::UserConfiguration uc;

    TS_ASSERT(!testee.canEnter());
    TS_ASSERT(testee.loadGameRoot(uc).get() == 0);
}

