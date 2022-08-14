/**
  *  \file server/interface/hostrankingclient.hpp
  *  \brief Class server::interface::HostRankingClient
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTRANKINGCLIENT_HPP
#define C2NG_SERVER_INTERFACE_HOSTRANKINGCLIENT_HPP

#include "afl/data/access.hpp"
#include "afl/net/commandhandler.hpp"
#include "server/interface/hostranking.hpp"

namespace server { namespace interface {

    /** Client for host ranking list access.
        Uses a CommandHandler to send commands to a server, and receives the results. */
    class HostRankingClient : public HostRanking {
     public:
        /** Constructor.
            @param commandHandler Server connection. Lifetime must exceed that of the HostRankingClient. */
        explicit HostRankingClient(afl::net::CommandHandler& commandHandler);

        // HostRanking:
        virtual Value_t* getUserList(const ListRequest& req);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
