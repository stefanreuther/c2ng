/**
  *  \file game/proxy/expressionlistproxy.hpp
  *  \brief Class game::proxy::ExpressionListProxy
  */
#ifndef C2NG_GAME_PROXY_EXPRESSIONLISTPROXY_HPP
#define C2NG_GAME_PROXY_EXPRESSIONLISTPROXY_HPP

#include "game/config/expressionlists.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Proxy to access expression lists (LRU, predefined).
        Bidirectional, synchronous access to an area of a game::config::ExpressionLists object.

        As of 20210302, this is the minimum possible implementation that will probably serve us.
        (Once could imagine making it totally asynchronous to avoid the wait when opening a LRU list.) */
    class ExpressionListProxy {
     public:
        /** Constructor.
            \param gameSender Game sender
            \param area       Area to access */
        ExpressionListProxy(util::RequestSender<Session> gameSender, game::config::ExpressionLists::Area area);

        /** Get list.
            \param [in]  ind WaitIndicator
            \param [out] out Result
            \see game::config::ExpressionLists::pack() */
        void getList(WaitIndicator& ind, game::config::ExpressionLists::Items_t& out);

        /** Add new least-recently-used item.
            Parameters describe the item, see util::ExpressionList::Item for details.
            \param flags Flags (including brackets)
            \param expr  Expression (used as name and value)
            \see game::config::ExpressionLists::pushRecent() */
        void pushRecent(String_t flags, String_t expr);

     private:
        util::RequestSender<Session> m_gameSender;
        game::config::ExpressionLists::Area m_area;
    };

} }

#endif
