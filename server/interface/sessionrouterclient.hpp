/**
  *  \file server/interface/sessionrouterclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_SESSIONROUTERCLIENT_HPP
#define C2NG_SERVER_INTERFACE_SESSIONROUTERCLIENT_HPP

#include "server/interface/sessionrouter.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/net/name.hpp"
#include "afl/net/line/linehandler.hpp"

namespace server { namespace interface {

    class SessionRouterClient : public SessionRouter {
     public:
        explicit SessionRouterClient(afl::net::NetworkStack& net, afl::net::Name name);
        virtual ~SessionRouterClient();

        virtual String_t getStatus();
        virtual String_t getInfo(SessionId_t sessionId);
        virtual String_t talk(SessionId_t sessionId, String_t command);
        virtual void sessionAction(SessionId_t sessionId, Action action);
        virtual void groupAction(String_t key, Action action, afl::data::StringList_t& result);
        virtual SessionId_t create(afl::base::Memory<const String_t> args);
        virtual String_t getConfiguration();

     private:
        afl::net::NetworkStack& m_networkStack;
        afl::net::Name m_name;

        void call(afl::net::line::LineHandler& hdl);
    };

} }

#endif
