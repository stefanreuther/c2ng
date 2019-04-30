/**
  *  \file server/host/hosthistory.hpp
  *  \brief Class server::host::HostHistory
  */
#ifndef C2NG_SERVER_HOST_HOSTHISTORY_HPP
#define C2NG_SERVER_HOST_HOSTHISTORY_HPP

#include "server/interface/hosthistory.hpp"

namespace server { namespace host {

    class Session;
    class Root;

    /** Implementation of HostGame interface.
        This interface implements HIST commands. */
    class HostHistory : public server::interface::HostHistory {
     public:
        /** Constructor.
            \param session Session
            \param root    Service root */
        HostHistory(Session& session, Root& root);

        virtual void getEvents(const EventFilter& filter, afl::container::PtrVector<Event>& result);
        virtual void getTurns(int32_t gameId, const TurnFilter& filter, afl::container::PtrVector<Turn>& result);

     private:
        Session& m_session;
        Root& m_root;
    };

} }

#endif
