/**
  *  \file server/console/connectioncontextfactory.hpp
  */
#ifndef C2NG_SERVER_CONSOLE_CONNECTIONCONTEXTFACTORY_HPP
#define C2NG_SERVER_CONSOLE_CONNECTIONCONTEXTFACTORY_HPP

#include <memory>
#include "server/console/contextfactory.hpp"
#include "server/console/clientpool.hpp"

namespace server { namespace console {

    class ConnectionContextFactory : public ContextFactory {
     public:
        ConnectionContextFactory(String_t name, ClientPool& pool, ClientPool::Index_t index);
        ~ConnectionContextFactory();

        virtual String_t getCommandName();
        virtual Context* create();
        virtual bool handleConfiguration(const String_t& key, const String_t& value);

     private:
        class Impl;

        const String_t m_name;
        ClientPool& m_pool;
        const ClientPool::Index_t m_index;
    };

} }

#endif
