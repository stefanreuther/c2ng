/**
  *  \file server/common/sessionprotocolhandler.hpp
  *  \brief Template class server::common::SessionProtocolHandler
  */
#ifndef C2NG_SERVER_COMMON_SESSIONPROTOCOLHANDLER_HPP
#define C2NG_SERVER_COMMON_SESSIONPROTOCOLHANDLER_HPP

#include "afl/net/protocolhandler.hpp"

namespace server { namespace common {

    /** Generic ProtocolHandler.
        Use this as the ProtocolHandler for a service with per-connection state.
        This contains the per-connection state.
        It contains a RESP protocol handler to feed the given CommandHandler type.
        \param Root            Service root object type
        \param Session         Service session type. Must be default-constructible.
        \param UserProtocolHandler Service ProtocolHandler type. Must be constructible from CommandHandler.
        \param CommandHandler  Service CommandHandler type. Must be constructible from (Root,Session).
        \see SessionProtocolHandlerFactory */
    template<typename Root, typename Session, typename UserProtocolHandler, typename CommandHandler>
    class SessionProtocolHandler : public afl::net::ProtocolHandler {
     public:
        /** Default constructor.
            \param root Service root */
        explicit SessionProtocolHandler(Root& root)
            : m_session(),
              m_commandHandler(root, m_session),
              m_protocolHandler(m_commandHandler)
            { }

        // ProtocolHandler:
        virtual void getOperation(Operation& op)
            { m_protocolHandler.getOperation(op); }
        virtual void advanceTime(afl::sys::Timeout_t msecs)
            { m_protocolHandler.advanceTime(msecs); }
        virtual void handleData(afl::base::ConstBytes_t bytes)
            { m_protocolHandler.handleData(bytes); }
        virtual void handleSendTimeout(afl::base::ConstBytes_t unsentBytes)
            { m_protocolHandler.handleSendTimeout(unsentBytes); }
        virtual void handleConnectionClose()
            { m_protocolHandler.handleConnectionClose(); }

     private:
        Session m_session;
        CommandHandler m_commandHandler;
        UserProtocolHandler m_protocolHandler;
    };

} }

#endif
