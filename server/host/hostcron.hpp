/**
  *  \file server/host/hostcron.hpp
  *  \brief Class server::host::HostCron
  */
#ifndef C2NG_SERVER_HOST_HOSTCRON_HPP
#define C2NG_SERVER_HOST_HOSTCRON_HPP

#include "server/interface/hostcron.hpp"

namespace server { namespace host {

    class Root;
    class Session;

    /** Implementation of HostCron interface. */
    class HostCron : public server::interface::HostCron {
     public:
        /** Constructor.
            \param session Session
            \param root    Service root */
        HostCron(Session& session, Root& root);

        // HostCron:
        virtual Event getGameEvent(int32_t gameId);
        virtual void listGameEvents(afl::base::Optional<int32_t> limit, std::vector<Event>& result);
        virtual bool kickstartGame(int32_t gameId);

     private:
        Session& m_session;
        Root& m_root;
    };

} }

#endif
