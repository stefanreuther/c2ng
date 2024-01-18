/**
  *  \file test/game/v3/passwordcheckertest.cpp
  *  \brief Test for game::v3::PasswordChecker
  */

#include "game/v3/passwordchecker.hpp"

#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "game/authcache.hpp"
#include "game/browser/usercallback.hpp"
#include "game/turn.hpp"
#include "game/v3/genextra.hpp"
#include "game/v3/genfile.hpp"

using game::AuthCache;
using game::browser::UserCallback;
using game::v3::GenExtra;
using game::v3::PasswordChecker;

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
AFL_TEST("game.v3.PasswordChecker:no-password", a)
{
    // Environment
    game::Turn t;
    AuthCache ac;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    UserCallbackMock cb(a);
    GenExtra::create(t).create(PLAYER_NR).setPassword("NOPASSWORD");

    // Operate
    PasswordChecker testee(t, &cb, log, tx);
    bool flag = false;
    testee.checkPassword(PLAYER_NR, ac, game::makeResultTask(flag));

    // Result is immediately available
    a.check("01. flag", flag);
}

/** Test use with no callback.
    If there is no UserCallback, the request succeeds immediately even with a password present. */
AFL_TEST("game.v3.PasswordChecker:check-disabled", a)
{
    // Environment
    game::Turn t;
    AuthCache ac;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    GenExtra::create(t).create(PLAYER_NR).setPassword("pass");

    // Operate
    PasswordChecker testee(t, 0, log, tx);
    bool flag = false;
    testee.checkPassword(PLAYER_NR, ac, game::makeResultTask(flag));

    // Result is immediately available
    a.check("01. flag", flag);
}

/** Test turn with password, success case.
    If there is a result password, the request succeeds when the correct password is provided. */
AFL_TEST("game.v3.PasswordChecker:ask:success", a)
{
    // Environment
    game::Turn t;
    AuthCache ac;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    UserCallbackMock cb(a);
    GenExtra::create(t).create(PLAYER_NR).setPassword("pass");

    // Operate
    PasswordChecker testee(t, &cb, log, tx);
    bool flag = false;
    cb.expectCall("askPassword('player 9's turn',0)");
    testee.checkPassword(PLAYER_NR, ac, game::makeResultTask(flag));
    cb.checkFinish();

    // Provide password; result becomes available
    cb.sig_passwordResult.raise(makeResponse("pass", false));
    a.check("01. flag", flag);
}

/** Test turn with password, failure case.
    If there is a result password, the request failw when the wrong password is provided. */
AFL_TEST("game.v3.PasswordChecker:ask:failure", a)
{
    // Environment
    game::Turn t;
    AuthCache ac;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    UserCallbackMock cb(a);
    GenExtra::create(t).create(PLAYER_NR).setPassword("pass");

    // Operate
    PasswordChecker testee(t, &cb, log, tx);
    bool flag = true;
    cb.expectCall("askPassword('player 9's turn',0)");
    testee.checkPassword(PLAYER_NR, ac, game::makeResultTask(flag));
    cb.checkFinish();

    // Provide password; result becomes available
    cb.sig_passwordResult.raise(makeResponse("notpass", false));
    a.check("01. flag", !flag);
}

/** Test turn with password, cancel.
    If there is a result password, the request fails when password entry is canceled. */
AFL_TEST("game.v3.PasswordChecker:ask:cancel", a)
{
    // Environment
    game::Turn t;
    AuthCache ac;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    UserCallbackMock cb(a);
    GenExtra::create(t).create(PLAYER_NR).setPassword("pass");

    // Operate
    PasswordChecker testee(t, &cb, log, tx);
    bool flag = true;
    cb.expectCall("askPassword('player 9's turn',0)");
    testee.checkPassword(PLAYER_NR, ac, game::makeResultTask(flag));
    cb.checkFinish();

    // Cancel; result becomes available
    cb.sig_passwordResult.raise(makeResponse("pass", true));
    a.check("01. flag", !flag);
}

/** Test turn with password, cached.
    If the correct password is cached, no question is asked. */
AFL_TEST("game.v3.PasswordChecker:cached", a)
{
    // Environment
    game::Turn t;
    AuthCache ac;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    UserCallbackMock cb(a);
    GenExtra::create(t).create(PLAYER_NR).setPassword("pass");

    // Add cached password
    std::auto_ptr<AuthCache::Item> p(new AuthCache::Item());
    p->password = String_t("pass");
    ac.addNew(p.release());

    // Operate
    PasswordChecker testee(t, &cb, log, tx);
    bool flag = false;
    testee.checkPassword(PLAYER_NR, ac, game::makeResultTask(flag));

    // Result is immediately available
    a.check("01. flag", flag);
}

/** Test turn with password, wrong password cached.
    If the wrong password is cached, user interaction happens anyway. */
AFL_TEST("game.v3.PasswordChecker:wrong-cached", a)
{
    // Environment
    game::Turn t;
    AuthCache ac;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    UserCallbackMock cb(a);
    GenExtra::create(t).create(PLAYER_NR).setPassword("pass");

    // Add wrong cached password
    std::auto_ptr<AuthCache::Item> p(new AuthCache::Item());
    p->password = String_t("wrongpass");
    ac.addNew(p.release());

    // Operate
    PasswordChecker testee(t, &cb, log, tx);
    bool flag = false;
    cb.expectCall("askPassword('player 9's turn',0)");
    testee.checkPassword(PLAYER_NR, ac, game::makeResultTask(flag));
    cb.checkFinish();

    // Provide password; result becomes available
    cb.sig_passwordResult.raise(makeResponse("pass", false));
    a.check("01. flag", flag);
}
