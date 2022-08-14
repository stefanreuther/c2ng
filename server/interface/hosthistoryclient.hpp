/**
  *  \file server/interface/hosthistoryclient.hpp
  *  \brief Class server::interface::HostHistoryClient
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTHISTORYCLIENT_HPP
#define C2NG_SERVER_INTERFACE_HOSTHISTORYCLIENT_HPP

#include "afl/data/access.hpp"
#include "afl/net/commandhandler.hpp"
#include "server/interface/hosthistory.hpp"

namespace server { namespace interface {

    /** Client for host history access.
        Uses a CommandHandler to send commands to a server, and receives the results. */
    class HostHistoryClient : public HostHistory {
     public:
        /** Constructor.
            @param commandHandler Server connection. Lifetime must exceed that of the HostHistoryClient. */
        explicit HostHistoryClient(afl::net::CommandHandler& commandHandler);

        // HostHistory virtuals:
        virtual void getEvents(const EventFilter& filter, afl::container::PtrVector<Event>& result);
        virtual void getTurns(int32_t gameId, const TurnFilter& filter, afl::container::PtrVector<Turn>& result);

     private:
        afl::net::CommandHandler& m_commandHandler;

        static void unpackEvent(Event& out, afl::data::Access a);
        static void unpackTurn(Turn& out, afl::data::Access a);
    };

} }

#endif
