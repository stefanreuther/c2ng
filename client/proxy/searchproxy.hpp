/**
  *  \file client/proxy/searchproxy.hpp
  */
#ifndef C2NG_CLIENT_PROXY_SEARCHPROXY_HPP
#define C2NG_CLIENT_PROXY_SEARCHPROXY_HPP

#include "afl/base/signal.hpp"
#include "afl/string/string.hpp"
#include "game/ref/list.hpp"
#include "game/searchquery.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace client { namespace proxy {

    /** Asynchronous, two-way proxy for resolving search queries.
        Submit a search query using search().
        The response arrives asynchronously on sig_success or sig_error. */
    class SearchProxy {
     public:
        class Responder;

        /** Constructor.
            \param root User-interface root
            \param gameSender Game sender */
        SearchProxy(ui::Root& root, util::RequestSender<game::Session> gameSender);

        /** Submit a search query.
            The search query is executed asynchronously on the game thread.
            You can (but don't have to) block UI.
            \param q Query */
        void search(const game::SearchQuery& q);

        /** Signal: successful search result.
            \param list Search result */
        afl::base::Signal<void(const game::ref::List&)> sig_success;

        /** Signal: query failed to parse or execute.
            \param msg Error message */
        afl::base::Signal<void(String_t)> sig_error;

        /** Access a session's saved query.
            Each search() operation will store the query in the session.
            Use this call to access it.
            \param session the session
            \return saved query */
        static game::SearchQuery& savedQuery(game::Session& session);

     private:
        util::RequestReceiver<SearchProxy> m_reply;
        util::RequestSender<game::Session> m_gameSender;
    };

} }

#endif
