/**
  *  \file server/host/hostslot.hpp
  *  \brief Class server::host::HostSlot
  */
#ifndef C2NG_SERVER_HOST_HOSTSLOT_HPP
#define C2NG_SERVER_HOST_HOSTSLOT_HPP

#include "server/interface/hostslot.hpp"

namespace server { namespace host {

    class Root;
    class Session;

    /** Implementation of the HostSlot interface.
        This interface allows adding/removing slots from games being set up. */
    class HostSlot : public server::interface::HostSlot {
     public:
        /** Constructor.
            @param session  Session (for access checking)
            @param root     Service root */
        HostSlot(const Session& session, Root& root);

        // Interface methods:
        virtual void add(int32_t gameId, afl::base::Memory<const int32_t> slotNrs);
        virtual void remove(int32_t gameId, afl::base::Memory<const int32_t> slotNrs);
        virtual void getAll(int32_t gameId, afl::data::IntegerList_t& result);

     private:
        const Session& m_session;
        Root& m_root;
    };

} }

#endif
