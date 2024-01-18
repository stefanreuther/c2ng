/**
  *  \file test/game/browser/unsupportedaccountfoldertest.cpp
  *  \brief Test for game::browser::UnsupportedAccountFolder
  */

#include "game/browser/unsupportedaccountfolder.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/browser/account.hpp"

namespace {
    class Receiver {
     public:
        Receiver(afl::test::Assert a)
            : m_beenHere(false),
              m_assert(a)
            { }

        void take(afl::base::Ptr<game::Root> r)
            {
                m_assert.checkNull("take: Root", r.get());
                m_beenHere = true;
            }

        bool beenHere() const
            { return m_beenHere; }

     private:
        bool m_beenHere;
        afl::test::Assert m_assert;
    };
}

/** Simple test.
    It's hard to test this class without repeating all the implementation,
    so this mainly tests we can instantiate the class. */
AFL_TEST("game.browser.UnsupportedAccountFolder", a)
{
    afl::string::NullTranslator tx;
    game::browser::Account account;
    game::browser::UnsupportedAccountFolder testee(tx, account);
    const game::config::UserConfiguration uc;

    a.check("01. canEnter", !testee.canEnter());

    Receiver recv(a("Receiver"));
    std::auto_ptr<game::Task_t> t = testee.loadGameRoot(uc, std::auto_ptr<game::browser::LoadGameRootTask_t>(game::browser::LoadGameRootTask_t::makeBound(&recv, &Receiver::take)));
    a.checkNonNull("11. get", t.get());
    t->call();
    a.check("12. beenHere", recv.beenHere());
}
