/**
  *  \file server/interface/hostslot.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTSLOT_HPP
#define C2NG_SERVER_INTERFACE_HOSTSLOT_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/types.hpp"
#include "afl/data/integerlist.hpp"

namespace server { namespace interface {

    /** Host file slot interface.
        This interface allows to modify slots of a game. */
    class HostSlot : public afl::base::Deletable {
     public:
        virtual void add(int32_t gameId, afl::base::Memory<const int32_t> slotNrs) = 0;

        virtual void remove(int32_t gameId, afl::base::Memory<const int32_t> slotNrs) = 0;

        virtual void getAll(int32_t gameId, afl::data::IntegerList_t& result) = 0;

        // TODO: more commands?
    };

} }

#endif
