/**
  *  \file game/browser/optionalusercallback.hpp
  *  \brief Class game::browser::OptionalUserCallback
  */
#ifndef C2NG_GAME_BROWSER_OPTIONALUSERCALLBACK_HPP
#define C2NG_GAME_BROWSER_OPTIONALUSERCALLBACK_HPP

#include "game/browser/usercallback.hpp"
#include "afl/base/signalconnection.hpp"

namespace game { namespace browser {

    /** Optional UserCallback.
        Forwards calls (and responses) to another UserCallback instance,
        or terminates them directly, reporting an unsuccessful/cancelled status. */
    class OptionalUserCallback : public UserCallback {
     public:
        /** Constructor. */
        OptionalUserCallback();

        /** Destructor. */
        ~OptionalUserCallback();

        /** Set instance.
            Future calls will be forwarded to and from that instance.
            @param pInstance Instance. Ownership will remain at caller. Can be null. */
        void setInstance(UserCallback* pInstance);

        // UserCallback:
        virtual void askPassword(const PasswordRequest& req);

     private:
        UserCallback* m_pInstance;
        afl::base::SignalConnection conn_passwordResult;
    };

} }

#endif
