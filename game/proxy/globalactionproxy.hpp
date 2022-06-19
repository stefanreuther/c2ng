/**
  *  \file game/proxy/globalactionproxy.hpp
  *  \brief Class game::proxy::GlobalActionProxy
  */
#ifndef C2NG_GAME_PROXY_GLOBALACTIONPROXY_HPP
#define C2NG_GAME_PROXY_GLOBALACTIONPROXY_HPP

#include "util/requestsender.hpp"
#include "game/session.hpp"
#include "util/treelist.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Proxy for global actions.

        Synchronous, bidirectional:
        - get list of actions

        For now, actually running the actions is NOT part of this proxy.
        This requires UI integration and is therefore implemented on the UI side
        using ScriptTask, client::si::Control::executeTaskWait(). */
    class GlobalActionProxy {
     public:
        /** Constructor.
            @param gameSender Game sender */
        explicit GlobalActionProxy(util::RequestSender<Session> gameSender);

        /** Get list of actions.
            @param [in,out] ind     WaitIndicator for UI synchronisation
            @param [out]    result  Result */
        void getActions(WaitIndicator& ind, util::TreeList& result);

     private:
        util::RequestSender<Session> m_gameSender;
    };

} }

#endif
