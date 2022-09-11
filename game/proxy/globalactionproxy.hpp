/**
  *  \file game/proxy/globalactionproxy.hpp
  *  \brief Class game::proxy::GlobalActionProxy
  */
#ifndef C2NG_GAME_PROXY_GLOBALACTIONPROXY_HPP
#define C2NG_GAME_PROXY_GLOBALACTIONPROXY_HPP

#include "game/session.hpp"
#include "interpreter/variablereference.hpp"
#include "util/requestsender.hpp"
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
            @param [out]    result  Result
            @param [in]     ref     Reference to variable containing the global actions */
        void getActions(WaitIndicator& ind, util::TreeList& result, interpreter::VariableReference ref);

     private:
        util::RequestSender<Session> m_gameSender;
    };

} }

#endif
