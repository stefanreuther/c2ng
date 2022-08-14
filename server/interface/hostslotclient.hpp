/**
  *  \file server/interface/hostslotclient.hpp
  *  \brief Class server::interface::HostSlotClient
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTSLOTCLIENT_HPP
#define C2NG_SERVER_INTERFACE_HOSTSLOTCLIENT_HPP

#include "afl/net/commandhandler.hpp"
#include "server/interface/hostslot.hpp"

namespace server { namespace interface {

    /** Host file slot interface.
        This interface allows to modify slots of a game. */
    class HostSlotClient : public HostSlot {
     public:
        /** Constructor.
            @param commandHandler Server connection. Lifetime must exceed that of the HostSlotClient. */
        explicit HostSlotClient(afl::net::CommandHandler& commandHandler);

        // HostSlot:
        virtual void add(int32_t gameId, afl::base::Memory<const int32_t> slotNrs);
        virtual void remove(int32_t gameId, afl::base::Memory<const int32_t> slotNrs);
        virtual void getAll(int32_t gameId, afl::data::IntegerList_t& result);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
