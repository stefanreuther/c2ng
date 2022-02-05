/**
  *  \file client/usercallback.hpp
  *  \brief Class client::UserCallback
  */
#ifndef C2NG_CLIENT_USERCALLBACK_HPP
#define C2NG_CLIENT_USERCALLBACK_HPP

#include "client/si/userside.hpp"
#include "game/browser/usercallback.hpp"

namespace client {

    /** Implementation of game::browser::UserCallback using UI.
        This implements the callbacks using real dialogs.

        This class does not implement the UI/game thread transition. */
    class UserCallback : public game::browser::UserCallback {
     public:
        UserCallback(client::si::UserSide& us);
        ~UserCallback();

        virtual void askPassword(const PasswordRequest& req);

     private:
        client::si::UserSide& m_userSide;
    };

}

#endif
