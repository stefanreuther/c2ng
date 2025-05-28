/**
  *  \file test/game/browser/usercallbacktest.cpp
  *  \brief Test for game::browser::UserCallback
  */

#include "game/browser/usercallback.hpp"

#include "afl/test/testrunner.hpp"

using game::browser::UserCallback;

/** Interface test. */
AFL_TEST_NOARG("game.browser.UserCallback")
{
    // Can create callback
    class Tester : public UserCallback {
     public:
        virtual void askPassword(const PasswordRequest& /*req*/)
            { }
    };
    Tester t;

    // Can call request
    t.askPassword(UserCallback::PasswordRequest());

    // Can call response
    t.sig_passwordResult.raise(UserCallback::PasswordResponse());
}

