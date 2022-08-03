/**
  *  \file u/t_game_v3_passwordchecker.cpp
  *  \brief Test for game::v3::PasswordChecker
  */

#include "game/v3/passwordchecker.hpp"

#include "t_game_v3.hpp"
#include "game/turn.hpp"
#include "game/browser/usercallback.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/v3/genextra.hpp"
#include "game/v3/genfile.hpp"

using game::v3::GenExtra;
using game::v3::PasswordChecker;
using game::browser::UserCallback;

namespace {
    const int PLAYER_NR = 9;

    class UserCallbackMock : public UserCallback, public afl::test::CallReceiver {
     public:
        UserCallbackMock(afl::test::Assert a)
            : UserCallback(), CallReceiver(a)
            { }

        virtual void askPassword(const PasswordRequest& req)
            { checkCall(afl::string::Format("askPassword('%s',%d)", req.accountName, int(req.hasFailed))); }
    };

    UserCallback::PasswordResponse makeResponse(const String_t& password, bool canceled)
    {
        UserCallback::PasswordResponse resp;
        resp.password = password;
        resp.canceled = canceled;
        return resp;
    }
}

/** Test turn with no password.
    If there is no result password, the request succeeds immediately. */
void
TestGameV3PasswordChecker::testNoPassword()
{
    // Environment
    game::Turn t;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    UserCallbackMock cb("testNoPassword");
    GenExtra::create(t).create(PLAYER_NR).setPassword("NOPASSWORD");

    // Operate
    PasswordChecker testee(t, &cb, log, tx);
    bool flag = false;
    testee.checkPassword(PLAYER_NR, game::makeResultTask(flag));

    // Result is immediately available
    TS_ASSERT(flag);
}

/** Test use with no callback.
    If there is no UserCallback, the request succeeds immediately even with a password present. */
void
TestGameV3PasswordChecker::testCheckDisabled()
{
    // Environment
    game::Turn t;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    GenExtra::create(t).create(PLAYER_NR).setPassword("pass");

    // Operate
    PasswordChecker testee(t, 0, log, tx);
    bool flag = false;
    testee.checkPassword(PLAYER_NR, game::makeResultTask(flag));

    // Result is immediately available
    TS_ASSERT(flag);
}

/** Test turn with password, success case.
    If there is a result password, the request succeeds when the correct password is provided. */
void
TestGameV3PasswordChecker::testAskSuccess()
{
    // Environment
    game::Turn t;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    UserCallbackMock cb("testAskSuccess");
    GenExtra::create(t).create(PLAYER_NR).setPassword("pass");

    // Operate
    PasswordChecker testee(t, &cb, log, tx);
    bool flag = false;
    cb.expectCall("askPassword('player 9's turn',0)");
    testee.checkPassword(PLAYER_NR, game::makeResultTask(flag));
    cb.checkFinish();

    // Provide password; result becomes available
    cb.sig_passwordResult.raise(makeResponse("pass", false));
    TS_ASSERT(flag);
}

/** Test turn with password, failure case.
    If there is a result password, the request failw when the wrong password is provided. */
void
TestGameV3PasswordChecker::testAskFailure()
{
    // Environment
    game::Turn t;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    UserCallbackMock cb("testAskSuccess");
    GenExtra::create(t).create(PLAYER_NR).setPassword("pass");

    // Operate
    PasswordChecker testee(t, &cb, log, tx);
    bool flag = true;
    cb.expectCall("askPassword('player 9's turn',0)");
    testee.checkPassword(PLAYER_NR, game::makeResultTask(flag));
    cb.checkFinish();

    // Provide password; result becomes available
    cb.sig_passwordResult.raise(makeResponse("notpass", false));
    TS_ASSERT(!flag);
}

/** Test turn with password, cancel.
    If there is a result password, the request fails when password entry is canceled. */
void
TestGameV3PasswordChecker::testAskCancel()
{
    // Environment
    game::Turn t;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    UserCallbackMock cb("testAskSuccess");
    GenExtra::create(t).create(PLAYER_NR).setPassword("pass");

    // Operate
    PasswordChecker testee(t, &cb, log, tx);
    bool flag = true;
    cb.expectCall("askPassword('player 9's turn',0)");
    testee.checkPassword(PLAYER_NR, game::makeResultTask(flag));
    cb.checkFinish();

    // Cancel; result becomes available
    cb.sig_passwordResult.raise(makeResponse("pass", true));
    TS_ASSERT(!flag);
}

