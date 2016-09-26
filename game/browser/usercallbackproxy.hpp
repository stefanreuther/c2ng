/**
  *  \file game/browser/usercallbackproxy.hpp
  *  \brief Class game::browser::UserCallbackProxy
  */
#ifndef C2NG_GAME_BROWSER_USERCALLBACKPROXY_HPP
#define C2NG_GAME_BROWSER_USERCALLBACKPROXY_HPP

#include "game/browser/usercallback.hpp"
#include "afl/base/ptr.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "util/requestsender.hpp"

namespace game { namespace browser {

    /** Proxy for UserCallback.
        This dispatchs requests that arrive on a UserCallback into another thread for handling.
        The other thread is identified by a RequestSender that can be set and reset as needed.

        If a callback appears while no RequestSender is set, calls to this UserCallback will fail/cancel. */
    class UserCallbackProxy : public UserCallback {
     public:
        /** Constructor.
            \param tx Translator
            \param log Logger */
        UserCallbackProxy(afl::string::Translator& tx, afl::sys::LogListener& log);

        /** Set instance.
            This function must be called from the thread owning the UserCallbackProxy (=the browser thread).
            \param p Instance of a RequestSender<UserCallback> that will allow access to the UserCallback which finally executes the requests. */
        void setInstance(util::RequestSender<UserCallback> p);

        // UserCallback:
        virtual bool askInput(String_t title, const std::vector<Element>& question, afl::data::Segment& values);

     private:
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;
        util::RequestSender<UserCallback> m_sender;
    };

} }

#endif
