/**
  *  \file game/proxy/searchproxy.hpp
  *  \brief Class game::proxy::SearchProxy
  */
#ifndef C2NG_GAME_PROXY_SEARCHPROXY_HPP
#define C2NG_GAME_PROXY_SEARCHPROXY_HPP

#include "afl/base/signal.hpp"
#include "afl/string/string.hpp"
#include "game/ref/list.hpp"
#include "game/searchquery.hpp"
#include "game/session.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Asynchronous, two-way proxy for resolving search queries.
        Submit a search query using search().
        The response arrives asynchronously on sig_success or sig_error. */
    class SearchProxy {
     public:
        class Responder;

        /** Constructor.
            \param gameSender Game sender
            \param reply RequestDispatcher to receive replies */
        SearchProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply);

        /** Retrieve last saved query.
            Use this to retrieve the query from a UI process.
            \param ind WaitIndicator for UI synchronisation
            \return saved search query
            \see savedQuery() */
        SearchQuery getSavedQuery(WaitIndicator& ind);

        /** Submit a search query.
            The search query is executed asynchronously on the game thread.
            You can (but don't have to) block UI.
            \param q Query
            \param saveQuery true to save the query; see savedQuery() */
        void search(const SearchQuery& q, bool saveQuery);

        /** Signal: successful search result.
            \param list Search result */
        afl::base::Signal<void(const game::ref::List&)> sig_success;

        /** Signal: query failed to parse or execute.
            \param msg Error message */
        afl::base::Signal<void(String_t)> sig_error;

        /** Access a session's saved query.
            Each search() operation with saveQuery=true will store the query in the session.
            Use this call to access it.
            \param session the session
            \return saved query */
        static SearchQuery& savedQuery(Session& session);

     private:
        util::RequestReceiver<SearchProxy> m_reply;
        util::RequestSender<Session> m_gameSender;
    };

} }

#endif
