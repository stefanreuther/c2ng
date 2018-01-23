/**
  *  \file server/console/connectioncontextfactory.hpp
  */
#ifndef C2NG_SERVER_CONSOLE_CONNECTIONCONTEXTFACTORY_HPP
#define C2NG_SERVER_CONSOLE_CONNECTIONCONTEXTFACTORY_HPP

#include <memory>
#include "server/console/contextfactory.hpp"
#include "afl/net/name.hpp"
#include "afl/net/resp/client.hpp"
#include "afl/net/networkstack.hpp"

namespace server { namespace console {

    class ConnectionContextFactory : public ContextFactory {
     public:
        ConnectionContextFactory(String_t name, uint16_t defaultPort, afl::net::NetworkStack& stack);
        ~ConnectionContextFactory();

        virtual String_t getCommandName();
        virtual Context* create();
        virtual bool handleConfiguration(const String_t& key, const String_t& value);

     private:
        class Impl;

        String_t m_name;
        afl::net::Name m_address;
        afl::net::NetworkStack& m_networkStack;
        std::auto_ptr<afl::net::resp::Client> m_client;
    };

} }

#endif
