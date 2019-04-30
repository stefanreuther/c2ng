/**
  *  \file server/interface/hosthistoryclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTHISTORYCLIENT_HPP
#define C2NG_SERVER_INTERFACE_HOSTHISTORYCLIENT_HPP

#include "server/interface/hosthistory.hpp"
#include "afl/net/commandhandler.hpp"
#include "afl/data/access.hpp"

namespace server { namespace interface {

    class HostHistoryClient : public HostHistory {
     public:
        explicit HostHistoryClient(afl::net::CommandHandler& commandHandler);

        virtual void getEvents(const EventFilter& filter, afl::container::PtrVector<Event>& result);
        virtual void getTurns(int32_t gameId, const TurnFilter& filter, afl::container::PtrVector<Turn>& result);

     private:
        afl::net::CommandHandler& m_commandHandler;

        static void unpackEvent(Event& out, afl::data::Access a);
        static void unpackTurn(Turn& out, afl::data::Access a);
    };

} }

#endif
