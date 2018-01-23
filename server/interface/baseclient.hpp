/**
  *  \file server/interface/baseclient.hpp
  *  \brief Class server::interface::BaseClient
  */
#ifndef C2NG_SERVER_INTERFACE_BASECLIENT_HPP
#define C2NG_SERVER_INTERFACE_BASECLIENT_HPP

#include "server/interface/base.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    /** Client for base operations.
        Uses a CommandHandler to send commands to a server. */
    class BaseClient : public Base {
     public:
        /** Constructor.
            \param commandHandler Server connection. Lifetime must exceed that of the BaseClient. */
        explicit BaseClient(afl::net::CommandHandler& commandHandler);

        /** Destructor. */
        ~BaseClient();

        // Base:
        virtual String_t ping();
        virtual void setUserContext(String_t user);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
