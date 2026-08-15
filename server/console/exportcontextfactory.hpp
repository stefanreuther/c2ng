/**
  *  \file server/console/exportcontextfactory.hpp
  */
#ifndef C2NG_SERVER_CONSOLE_EXPORTCONTEXTFACTORY_HPP
#define C2NG_SERVER_CONSOLE_EXPORTCONTEXTFACTORY_HPP

#include "server/console/contextfactory.hpp"
#include "server/console/clientpool.hpp"

namespace server { namespace console {

    class ExportContextFactory : public ContextFactory {
     public:
        ExportContextFactory(ClientPool& pool, ClientPool::Index_t index);
        ~ExportContextFactory();

        virtual String_t getCommandName();
        virtual Context* create();
        virtual bool handleConfiguration(const String_t& key, const String_t& value);

     private:
        ClientPool& m_pool;
        ClientPool::Index_t m_index;

        class Impl;
    };

} }

#endif
