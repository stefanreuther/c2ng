/**
  *  \file test/game/browser/optionalusercallbacktest.cpp
  *  \brief Test for game::browser::OptionalUserCallback
  */

#include "game/browser/optionalusercallback.hpp"

#include "afl/test/testrunner.hpp"
#include "afl/test/callreceiver.hpp"

using game::browser::OptionalUserCallback;
using game::browser::UserCallback;
using afl::test::CallReceiver;

namespace {
    class TestCallback : public UserCallback, public CallReceiver {
     public:
        TestCallback(afl::test::Assert a)
            : CallReceiver(a)
            { }
        void askPassword(const PasswordRequest& /*req*/)
            { checkCall("askPassword()"); }
    };

    class TestResponder : public CallReceiver {
     public:
        TestResponder(afl::test::Assert a)
            : CallReceiver(a)
            { }
        void respond()
            { checkCall("respond()"); }
    };
}

AFL_TEST("game.browser.OptionalUserCallback:null", a)
{
    OptionalUserCallback testee;
    TestResponder resp(a);
    testee.sig_passwordResult.add(&resp, &TestResponder::respond);

    // Request produces response
    resp.expectCall("respond()");
    AFL_CHECK_SUCCEEDS(a, testee.askPassword(UserCallback::PasswordRequest()));
    resp.checkFinish();
}

AFL_TEST("game.browser.OptionalUserCallback:connected", a)
{
    OptionalUserCallback testee;
    TestCallback cb(a("TestCallback"));
    testee.setInstance(&cb);

    TestResponder resp(a("TestResponder"));
    testee.sig_passwordResult.add(&resp, &TestResponder::respond);

    // Request forwarded to callback
    cb.expectCall("askPassword()");
    AFL_CHECK_SUCCEEDS(a, testee.askPassword(UserCallback::PasswordRequest()));
    cb.checkFinish();

    // Response forwarded back
    resp.expectCall("respond()");
    cb.sig_passwordResult.raise(UserCallback::PasswordResponse());
    resp.checkFinish();
}
