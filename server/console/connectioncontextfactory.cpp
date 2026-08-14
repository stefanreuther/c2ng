/**
  *  \file server/console/connectioncontextfactory.cpp
  */

#include <stdexcept>
#include "server/console/connectioncontextfactory.hpp"
#include "afl/net/resp/client.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/time.hpp"
#include "server/console/context.hpp"
#include "server/types.hpp"

class server::console::ConnectionContextFactory::Impl : public Context {
 public:
    explicit Impl(const String_t& name, ClientPool& pool, ClientPool::Index_t index);
    virtual bool call(const String_t& cmd, interpreter::Arguments args, Parser& parser, std::auto_ptr<afl::data::Value>& result);
    virtual String_t getName();

 private:
    const String_t m_name;
    ClientPool& m_pool;
    const ClientPool::Index_t m_index;
};

server::console::ConnectionContextFactory::Impl::Impl(const String_t& name, ClientPool& pool, ClientPool::Index_t index)
    : m_name(name),
      m_pool(pool),
      m_index(index)
{ }

bool
server::console::ConnectionContextFactory::Impl::call(const String_t& cmd, interpreter::Arguments args, Parser& /*parser*/, std::auto_ptr<afl::data::Value>& result)
{
    // ex ConnectionContext::processCommand
    if (cmd == "repeat") {
        // Process command repeatedly, for benchmarking
        args.checkArgumentCountAtLeast(2);

        // Get repeat count
        int32_t n;
        if (!afl::string::strToInteger(toString(args.getNext()), n) || n <= 0) {
            throw std::runtime_error("Expecting number");
        }

        // Build command
        afl::data::Segment seg;
        while (args.getNumArgs() > 0) {
            seg.pushBack(args.getNext());
        }

        // Loop
        afl::net::resp::Client& client = m_pool.get(m_index);
        uint32_t startTicks = afl::sys::Time::getTickCounter();
        for (int32_t i = 0; i < n; ++i) {
            client.callVoid(seg);
        }
        uint32_t endTicks = afl::sys::Time::getTickCounter();
        uint32_t elapsed = endTicks - startTicks;

        // Return
        result.reset(makeStringValue(afl::string::Format("%d.%03d seconds (%d ms per iteration)", elapsed / 1000, elapsed % 1000, elapsed / n)));
        return true;
    }

    if (cmd == "reconnect" || cmd == "reset") {
        // Force reconnect
        m_pool.reset(m_index);
        m_pool.get(m_index);
        result.reset(makeStringValue("OK"));
        return true;
    }

    // Process command directly
    afl::data::Segment seg;
    if (cmd == "exec") {
        args.checkArgumentCountAtLeast(1);
    } else {
        seg.pushBackString(cmd);
    }
    while (args.getNumArgs() > 0) {
        seg.pushBack(args.getNext());
    }
    result.reset(m_pool.get(m_index).call(seg));
    return true;
}

String_t
server::console::ConnectionContextFactory::Impl::getName()
{
    return m_name;
}

/************************ ConnectionContextFactory ***********************/

server::console::ConnectionContextFactory::ConnectionContextFactory(String_t name, ClientPool& pool, ClientPool::Index_t index)
    : m_name(name),
      m_pool(pool),
      m_index(index)
{
    // ex ConnectionContext::ConnectionContext
}

server::console::ConnectionContextFactory::~ConnectionContextFactory()
{ }

String_t
server::console::ConnectionContextFactory::getCommandName()
{
    return m_name;
}

server::console::Context*
server::console::ConnectionContextFactory::create()
{
    // Make sure that we connect when the context is entered
    m_pool.get(m_index);

    // Create context handler
    return new Impl(m_name, m_pool, m_index);
}

bool
server::console::ConnectionContextFactory::handleConfiguration(const String_t& /*key*/, const String_t& /*value*/)
{
    return false;
}
