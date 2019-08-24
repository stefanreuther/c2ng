/**
  *  \file server/interface/hostslot.hpp
  *  \brief Interface server::interface::HostSlot
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTSLOT_HPP
#define C2NG_SERVER_INTERFACE_HOSTSLOT_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/types.hpp"
#include "afl/data/integerlist.hpp"

namespace server { namespace interface {

    /** Host slot interface.
        This interface allows to modify slots of a game. */
    class HostSlot : public afl::base::Deletable {
     public:
        /** Add slots to a game.
            \param gameId Game Id
            \param slotNrs Slots to add (1..11) */
        virtual void add(int32_t gameId, afl::base::Memory<const int32_t> slotNrs) = 0;

        /** Remove slots from a game.
            \param gameId Game Id
            \param slotNrs Slots to remove (1..11) */
        virtual void remove(int32_t gameId, afl::base::Memory<const int32_t> slotNrs) = 0;

        /** Lists slots of a game.
            \param gameId [in] Game Id
            \param result [out] Slot numbers (1..11) */
        virtual void getAll(int32_t gameId, afl::data::IntegerList_t& result) = 0;
    };

} }

#endif
