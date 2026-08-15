/**
  *  \file server/console/exportcontextfactory.cpp
  */

#include "server/console/exportcontextfactory.hpp"
#include "server/console/context.hpp"
#include "server/ports.hpp"

#include "server/console/dbexporter.hpp"
#include "afl/sys/longcommandlineparser.hpp"
#include "server/types.hpp"
#include "server/console/terminal.hpp"
#include "server/console/parser.hpp"

/*
 *  Command Handler
 */

class server::console::ExportContextFactory::Impl : public Context {
 public:
    Impl(ClientPool& pool, ClientPool::Index_t index)
        : m_pool(pool), m_index(index)
        { }
    virtual bool call(const String_t& cmd, interpreter::Arguments args, Parser& parser, std::auto_ptr<afl::data::Value>& result)
        {
            if (cmd == "db") {
                exportDatabase(parser.terminal(), m_pool.get(m_index), args);
                result.reset();
                return true;
            } else {
                return false;
            }
        }

    virtual String_t getName()
        { return "export"; }

 private:
    ClientPool& m_pool;
    ClientPool::Index_t m_index;
};

/*
 *  Entry point
 */

server::console::ExportContextFactory::ExportContextFactory(ClientPool& pool, ClientPool::Index_t index)
    : m_pool(pool),
      m_index(index)
{ }

server::console::ExportContextFactory::~ExportContextFactory()
{ }

String_t
server::console::ExportContextFactory::getCommandName()
{
    return "export";
}

server::console::Context*
server::console::ExportContextFactory::create()
{
    return new Impl(m_pool, m_index);
}

bool
server::console::ExportContextFactory::handleConfiguration(const String_t& /*key*/, const String_t& /*value*/)
{
    return false;
}
