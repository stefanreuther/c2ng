/**
  *  \file server/host/hostslot.hpp
  */
#ifndef C2NG_SERVER_HOST_HOSTSLOT_HPP
#define C2NG_SERVER_HOST_HOSTSLOT_HPP

#include "server/interface/hostslot.hpp"

namespace server { namespace host {

    class Root;
    class Session;

    class HostSlot : public server::interface::HostSlot {
     public:
        HostSlot(Session& session, Root& root);

        virtual void add(int32_t gameId, afl::base::Memory<const int32_t> slotNrs);
        virtual void remove(int32_t gameId, afl::base::Memory<const int32_t> slotNrs);
        virtual void getAll(int32_t gameId, afl::data::IntegerList_t& result);

     private:
        Session& m_session;
        Root& m_root;
    };

} }

#endif
