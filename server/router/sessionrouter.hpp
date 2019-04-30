/**
  *  \file server/router/sessionrouter.hpp
  */
#ifndef C2NG_SERVER_ROUTER_SESSIONROUTER_HPP
#define C2NG_SERVER_ROUTER_SESSIONROUTER_HPP

#include "server/interface/sessionrouter.hpp"

namespace server { namespace router {

    class Root;
    class Session;

    class SessionRouter : public server::interface::SessionRouter {
     public:
        SessionRouter(Root& root);

        virtual String_t getStatus();
        virtual String_t getInfo(SessionId_t sessionId);
        virtual String_t talk(SessionId_t sessionId, String_t command);
        virtual void sessionAction(SessionId_t sessionId, Action action);
        virtual void groupAction(String_t key, Action action, afl::data::StringList_t& result);
        virtual SessionId_t create(afl::base::Memory<const String_t> args);
        virtual String_t getConfiguration();

     private:
        Root& m_root;

        void doAction(Session& s, Action action);
    };

} }

#endif
