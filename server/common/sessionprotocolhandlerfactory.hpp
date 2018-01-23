/**
  *  \file server/common/sessionprotocolhandlerfactory.hpp
  *  \brief Template class server::common::SessionProtocolHandlerFactory
  */
#ifndef C2NG_SERVER_COMMON_SESSIONPROTOCOLHANDLERFACTORY_HPP
#define C2NG_SERVER_COMMON_SESSIONPROTOCOLHANDLERFACTORY_HPP

#include "afl/net/protocolhandlerfactory.hpp"
#include "server/common/sessionprotocolhandler.hpp"

namespace server { namespace common {

    /** Generic ProtocolHandlerFactory.
        Use this as the ProtocolHandlerFactory for a service with per-connection state.
        \param Root            Service root object type
        \param Session         Service session type. Must be default-constructible.
        \param UserProtocolHandler Service ProtocolHandler type. Must be constructible from CommandHandler.
        \param CommandHandler  Service CommandHandler type. Must be constructible from (Root,Session).
        \see SessionProtocolHandler */
    template<typename Root, typename Session, typename UserProtocolHandler, typename CommandHandler>
    class SessionProtocolHandlerFactory : public afl::net::ProtocolHandlerFactory {
     public:
        /** Shortcut. */
        typedef SessionProtocolHandler<Root, Session, UserProtocolHandler, CommandHandler> ProtocolHandler_t;

        /** Default constructor.
            \param root Service root */
        explicit SessionProtocolHandlerFactory(Root& root)
            : m_root(root)
            { }

        // ProtocolHandlerFactory:
        virtual ProtocolHandler_t* create()
            { return new ProtocolHandler_t(m_root); }
     private:
        Root& m_root;
    };

} }

#endif
