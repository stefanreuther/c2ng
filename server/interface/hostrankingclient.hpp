/**
  *  \file server/interface/hostrankingclient.hpp
  *  \brief Class server::interface::HostRankingClient
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTRANKINGCLIENT_HPP
#define C2NG_SERVER_INTERFACE_HOSTRANKINGCLIENT_HPP

#include "server/interface/hostranking.hpp"
#include "afl/net/commandhandler.hpp"
#include "afl/data/access.hpp"

namespace server { namespace interface {

    class HostRankingClient : public HostRanking {
     public:
        explicit HostRankingClient(afl::net::CommandHandler& commandHandler);

        virtual Value_t* getUserList(const ListRequest& req);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
