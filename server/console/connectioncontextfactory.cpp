/**
  *  \file server/console/connectioncontextfactory.cpp
  */

#include <stdexcept>
#include "server/console/connectioncontextfactory.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/time.hpp"
#include "server/console/context.hpp"
#include "server/types.hpp"
#include "server/ports.hpp"

class server::console::ConnectionContextFactory::Impl : public Context {
 public:
    explicit Impl(String_t name, afl::net::resp::Client& client);
    virtual bool call(const String_t& cmd, interpreter::Arguments args, Parser& parser, std::auto_ptr<afl::data::Value>& result);
    virtual String_t getName();

 private:
    String_t m_name;
    afl::net::resp::Client& m_client;
};

server::console::ConnectionContextFactory::Impl::Impl(String_t name, afl::net::resp::Client& client)
    : m_name(name),
      m_client(client)
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
        //     if (client.maybeReconnect(host, port)) {
        //         std::cout << "(auto-reconnecting to " << host << ":" << port << "...)\n";
        //     }

        // Build command
        afl::data::Segment seg;
        while (args.getNumArgs() > 0) {
            seg.pushBack(args.getNext());
        }

        // Loop
        uint32_t startTicks = afl::sys::Time::getTickCounter();
        for (int32_t i = 0; i < n; ++i) {
            m_client.callVoid(seg);
        }
        uint32_t endTicks = afl::sys::Time::getTickCounter();
        uint32_t elapsed = endTicks - startTicks;

        // Return
        result.reset(makeStringValue(afl::string::Format("%d.%03d seconds (%d ms per iteration)", elapsed / 1000, elapsed % 1000, elapsed / n)));
        return true;
    }

    // FIXME:
    // if (cmd[0] == "reconnect" || cmd[0] == "reset") {
    //     // Reconnect
    //     std::cout << "(reconnecting to " << host << ":" << port << "...)\n";
    //     client.connect(host, port);
    //     return makeStringValue("OK");
    // }

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
    // FIXME: visibly deal with reconnect
    //     if (client.maybeReconnect(host, port)) {
    //         std::cout << "(auto-reconnecting to " << host << ":" << port << "...)\n";
    //     }
    result.reset(m_client.call(seg));
    return true;
}

String_t
server::console::ConnectionContextFactory::Impl::getName()
{
    return m_name;
}

/************************ ConnectionContextFactory ***********************/

server::console::ConnectionContextFactory::ConnectionContextFactory(String_t name, uint16_t defaultPort, afl::net::NetworkStack& stack)
    : m_name(name),
      m_address(DEFAULT_ADDRESS, defaultPort),
      m_networkStack(stack),
      m_client()
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
    if (m_client.get() == 0) {
        // FIXME: std::cout << "(connecting to " << host << ":" << port << "...)\n";
        m_client.reset(new afl::net::resp::Client(m_networkStack, m_address));
    }
    return new Impl(m_name, *m_client);
}

bool
server::console::ConnectionContextFactory::handleConfiguration(const String_t& key, const String_t& value)
{
    // ex ConnectionContext::checkConfig
    if (afl::string::strCaseCompare(key, m_name + ".host") == 0) {
        m_address.setName(value);
        return true;
    } else if (afl::string::strCaseCompare(key, m_name + ".port") == 0) {
        m_address.setService(value);
        return true;
    } else {
        return false;
    }
}
