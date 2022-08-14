/**
  *  \file server/host/hostranking.hpp
  *  \brief Class server::host::HostRanking
  */
#ifndef C2NG_SERVER_HOST_HOSTRANKING_HPP
#define C2NG_SERVER_HOST_HOSTRANKING_HPP

#include "server/interface/hostranking.hpp"

namespace server { namespace host {

    class Session;
    class Root;

    /** Implementation of HostRanking interface. */
    class HostRanking : public server::interface::HostRanking {
     public:
        /** Constructor.
            \param session Session
            \param root    Service root */
        HostRanking(Session& session, Root& root);

        // Interface methods:
        virtual Value_t* getUserList(const ListRequest& req);

     private:
        const Session& m_session;
        Root& m_root;
    };

} }

#endif
