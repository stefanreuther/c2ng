/**
  *  \file game/browser/usercallback.hpp
  *  \brief Base class game::browser::UserCallback
  */
#ifndef C2NG_GAME_BROWSER_USERCALLBACK_HPP
#define C2NG_GAME_BROWSER_USERCALLBACK_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/signal.hpp"
#include "afl/string/string.hpp"

namespace game { namespace browser {

    /** User callback.
        Browser actions can at any time require user interaction (credential input).
        They do so by calling a function of UserCallback.

        The integrator can provide a response using a signal,
        either on the same stack or a new one. */
    class UserCallback : public afl::base::Deletable {
     public:
        /** Password request.
            @see askPassword */
        struct PasswordRequest {
            String_t accountName;    ///< Name of account, e.g. "user @ host".
            bool hasFailed;          ///< true if password authentication has failed before.
            // possible future attributes: allowSave - allow saving password
            PasswordRequest()
                : accountName(), hasFailed()
                { }
        };

        /** Password response.
            @see sig_passwordResult */
        struct PasswordResponse {
            String_t password;       ///< Password provided by user.
            bool canceled;           ///< true if user canceled input.
            // possible future attributes: allowSave - save password permanently
            PasswordResponse()
                : password(), canceled()
                { }
        };

        /** Ask for a password.
            Function must eventually cause sig_passwordResult to be called. */
        virtual void askPassword(const PasswordRequest& req) = 0;

        /** Signal: password entered.
            @see askPassword */
        afl::base::Signal<void(PasswordResponse)> sig_passwordResult;
    };

} }

#endif
