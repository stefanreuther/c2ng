/**
  *  \file u/t_game_browser_unsupportedaccountfolder.cpp
  *  \brief Test for game::browser::UnsupportedAccountFolder
  */

#include "game/browser/unsupportedaccountfolder.hpp"

#include "t_game_browser.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/browser/account.hpp"

namespace {
    class Receiver {
     public:
        Receiver()
            : m_beenHere(false)
            { }

        void take(afl::base::Ptr<game::Root> r)
            {
                TS_ASSERT(r.get() == 0);
                m_beenHere = true;
            }

        bool beenHere() const
            { return m_beenHere; }

     private:
        bool m_beenHere;
    };
}

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

    Receiver recv;
    std::auto_ptr<game::Task_t> t = testee.loadGameRoot(uc, std::auto_ptr<game::browser::LoadGameRootTask_t>(game::browser::LoadGameRootTask_t::makeBound(&recv, &Receiver::take)));
    TS_ASSERT(t.get() != 0);
    t->call();
    TS_ASSERT(recv.beenHere());
}

