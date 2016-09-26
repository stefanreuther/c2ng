/**
  *  \file client/usercallback.hpp
  *  \brief Class client::UserCallback
  */
#ifndef C2NG_CLIENT_USERCALLBACK_HPP
#define C2NG_CLIENT_USERCALLBACK_HPP

#include "game/browser/usercallback.hpp"
#include "ui/root.hpp"
#include "util/requestreceiver.hpp"
#include "game/browser/session.hpp"

namespace client {

    /** Implementation of game::browser::UserCallback using UI.
        This implements the callbacks using real dialogs.
        
        Creating an object of this type will automatically register it with the BrowserSession's UserCallbackProxy, destroying it will unregister it.
        Therefore, if you anticipate background browser callbacks, create a UserCallback object in the UI thread. */
    class UserCallback : public game::browser::UserCallback {
     public:
        UserCallback(ui::Root& root, util::RequestSender<game::browser::Session> sender);
        ~UserCallback();

        virtual bool askInput(String_t title, const std::vector<Element>& question, afl::data::Segment& values);

     private:
        util::RequestReceiver<game::browser::UserCallback> m_receiver;
        ui::Root& m_root;
        util::RequestSender<game::browser::Session> m_sender;
    };

}

#endif
