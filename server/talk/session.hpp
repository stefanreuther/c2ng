/**
  *  \file server/talk/session.hpp
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
        Session();
        ~Session();

        bool hasPrivilege(String_t privString, Root& root);
        void checkPrivilege(String_t privString, Root& root);

        server::talk::render::Options& renderOptions();

     private:
        server::talk::render::Options m_renderOptions;
    };

} }

#endif
