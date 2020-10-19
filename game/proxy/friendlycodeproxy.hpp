/**
  *  \file game/proxy/friendlycodeproxy.hpp
  *  \brief Class game::proxy::FriendlyCodeProxy
  */
#ifndef C2NG_GAME_PROXY_FRIENDLYCODEPROXY_HPP
#define C2NG_GAME_PROXY_FRIENDLYCODEPROXY_HPP

#include "util/requestsender.hpp"
#include "game/session.hpp"
#include "game/proxy/waitindicator.hpp"

namespace game { namespace proxy {

    /** Proxy for friendly-code access.

        Synchronous, bidirectional:
        - generateRandomCode */
    class FriendlyCodeProxy {
     public:
        /** Constructor.
            \param gameSender Sender */
        explicit FriendlyCodeProxy(util::RequestSender<Session> gameSender);

        /** Generate random friendly code.
            \param link WaitIndicator
            \return friendly code; empty if preconditions not fulfilled
            \see game::spec::FriendlyCodeList::generateRandomCode */
        String_t generateRandomCode(WaitIndicator& link);

     private:
        util::RequestSender<Session> m_gameSender;
    };

} }

#endif
