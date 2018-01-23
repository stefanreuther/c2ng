/**
  *  \file server/host/session.hpp
  *  \brief Class server::host::Session
  */
#ifndef C2NG_SERVER_HOST_SESSION_HPP
#define C2NG_SERVER_HOST_SESSION_HPP

#include "server/common/session.hpp"
#include "server/host/game.hpp"

namespace server { namespace host {

    /** Server session state for Host service. */
    class Session : public server::common::Session {
     public:
        /** Constructor. */
        Session();

        /** Destructor. */
        ~Session();

        /** Check permissions.
            If the desired permission is not available, throws a PERMISSION_DENIED exception.
            \param g Game to check
            \param level Desired permission level */
        void checkPermission(Game& g, Game::PermissionLevel level);
    };

} }

#endif
