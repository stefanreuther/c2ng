/**
  *  \file client/proxy/helpproxy.hpp
  */
#ifndef C2NG_CLIENT_PROXY_HELPPROXY_HPP
#define C2NG_CLIENT_PROXY_HELPPROXY_HPP

#include "client/downlink.hpp"
#include "afl/io/xml/node.hpp"
#include "util/requestsender.hpp"
#include "game/session.hpp"

namespace client { namespace proxy {

    class HelpProxy {
     public:
        HelpProxy(util::RequestSender<game::Session> gameSender);

        void loadHelpPage(Downlink& link, afl::io::xml::Nodes_t& result, String_t pageName);

     private:
        util::RequestSender<game::Session> m_gameSender;
    };

} }

#endif
