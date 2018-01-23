/**
  *  \file server/talk/session.hpp
  *  \brief Class server::talk::Session
  */
#ifndef C2NG_SERVER_TALK_SESSION_HPP
#define C2NG_SERVER_TALK_SESSION_HPP

#include "afl/string/string.hpp"
#include "server/talk/render/options.hpp"
#include "server/common/session.hpp"

namespace server { namespace talk {

    class Root;

    /** A talk connection's session state.
        Represents per-connection state that is lost when the connection is closed. */
    class Session : public server::common::Session {
     public:
        /** Default constructor. */
        Session();

        /** Destructor. */
        ~Session();

        /** Access render options.
            Render options are per-session state.
            \return options object */
        server::talk::render::Options& renderOptions();

        /** Check permission.
            \param privString Privilege string to check against (see Root::checkUserPermission)
            \param root Service root
            \return true if permission granted for this session */
        bool hasPermission(String_t privString, Root& root) const;

        /** Check permission, throw.
            \param privString Privilege string to check against (see Root::checkUserPermission)
            \param root Service root
            \throw std::exception if permission not granted for htis session */
        void checkPermission(String_t privString, Root& root) const;

     private:
        server::talk::render::Options m_renderOptions;
    };

} }

#endif
