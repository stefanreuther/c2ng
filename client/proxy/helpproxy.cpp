/**
  *  \file client/proxy/helpproxy.cpp
  */

#include "client/proxy/helpproxy.hpp"
#include "client/help.hpp"

client::proxy::HelpProxy::HelpProxy(util::RequestSender<game::Session> gameSender)
    : m_gameSender(gameSender)
{ }

void
client::proxy::HelpProxy::loadHelpPage(Downlink& link, afl::io::xml::Nodes_t& result, String_t pageName)
{
    class Query : public util::Request<game::Session> {
     public:
        Query(afl::io::xml::Nodes_t& result, String_t pageName)
            : m_result(result), m_pageName(pageName)
            { }
        virtual void handle(game::Session& session)
            { client::loadHelpPage(session, m_result, m_pageName); }
     private:
        afl::io::xml::Nodes_t& m_result;
        String_t m_pageName;
    };

    result.clear();

    Query q(result, pageName);
    link.call(m_gameSender, q);
}
