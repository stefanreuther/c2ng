/**
  *  \file server/interface/hostslotclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTSLOTCLIENT_HPP
#define C2NG_SERVER_INTERFACE_HOSTSLOTCLIENT_HPP

#include "server/interface/hostslot.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    /** Host file slot interface.
        This interface allows to modify slots of a game. */
    class HostSlotClient : public HostSlot {
     public:
        explicit HostSlotClient(afl::net::CommandHandler& commandHandler);

        virtual void add(int32_t gameId, afl::base::Memory<const int32_t> slotNrs);
        virtual void remove(int32_t gameId, afl::base::Memory<const int32_t> slotNrs);
        virtual void getAll(int32_t gameId, afl::data::IntegerList_t& result);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
