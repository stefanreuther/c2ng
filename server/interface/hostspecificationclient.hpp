/**
  *  \file server/interface/hostspecificationclient.hpp
  *  \brief Class server::interface::HostSpecificationClient
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTSPECIFICATIONCLIENT_HPP
#define C2NG_SERVER_INTERFACE_HOSTSPECIFICATIONCLIENT_HPP

#include "server/interface/hostspecification.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    /** Client for host specification access.
        Uses a CommandHandler to send commands to a server, and receives the results. */
    class HostSpecificationClient : public HostSpecification {
     public:
        /** Constructor.
            @param commandHandler CommandHandler to use */
        explicit HostSpecificationClient(afl::net::CommandHandler& commandHandler);

        // HostSpecification:
        virtual server::Value_t* getShiplistData(String_t shiplistId, Format format, const afl::data::StringList_t& keys);
        virtual server::Value_t* getGameData(int32_t gameId, Format format, const afl::data::StringList_t& keys);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
