/**
  *  \file server/host/hostranking.hpp
  */
#ifndef C2NG_SERVER_HOST_HOSTRANKING_HPP
#define C2NG_SERVER_HOST_HOSTRANKING_HPP

#include "server/interface/hostranking.hpp"

namespace server { namespace host {

    class Session;
    class Root;

    class HostRanking : public server::interface::HostRanking {
     public:
        /** Constructor.
            \param session Session
            \param root    Service root */
        HostRanking(Session& session, Root& root);

        virtual Value_t* getUserList(const ListRequest& req);

     private:
        Session& m_session;
        Root& m_root;
    };

} }

#endif
