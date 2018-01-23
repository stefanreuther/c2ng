/**
  *  \file server/mailout/session.hpp
  *  \brief Class server::mailout::Session
  */
#ifndef C2NG_SERVER_MAILOUT_SESSION_HPP
#define C2NG_SERVER_MAILOUT_SESSION_HPP

#include <memory>
#include "server/mailout/message.hpp"

namespace server { namespace mailout {

    /** Per-session state for mailout.
        mailout has no user context, so the only state it has is the message under construction. */
    struct Session {
        /** Current message. Can be null if no message is being constructed. */
        std::auto_ptr<Message> currentMessage;
    };

} }

#endif
