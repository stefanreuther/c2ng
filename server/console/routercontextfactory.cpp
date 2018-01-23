/**
  *  \file server/console/routercontextfactory.cpp
  */

#include "server/console/routercontextfactory.hpp"
#include "afl/net/line/client.hpp"
#include "afl/net/line/linehandler.hpp"
#include "afl/net/line/linesink.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "server/console/context.hpp"
#include "server/console/parser.hpp"
#include "server/console/terminal.hpp"
#include "server/types.hpp"
#include "afl/net/line/simplequery.hpp"
#include "server/ports.hpp"

namespace {
    bool isSessionCommand(const String_t& session,
                          const String_t& cmd,
                          const interpreter::Arguments& args)
    {
        // ex RouterContext::isSessionCommand
        return !session.empty()
            && (cmd == "help"
                || cmd == "stat"
                || cmd == "get"
                || cmd == "post"
                || cmd == "quit"
                || (cmd == "save" && args.getNumArgs() == 0));
    }
}

class server::console::RouterContextFactory::Impl : public server::console::Context {
 public:
    Impl(RouterContextFactory& parent)
        : m_parent(parent),
          m_sessionName()
        { }

    virtual bool call(const String_t& cmd, interpreter::Arguments args, Parser& parser, std::auto_ptr<afl::data::Value>& result);
    virtual String_t getName();

 private:
    void processCommand(const String_t& cmd, interpreter::Arguments args, std::auto_ptr<afl::data::Value>& result);
    void processSessionCommand(const String_t& session, const String_t& cmd, interpreter::Arguments args, std::auto_ptr<afl::data::Value>& result);

    String_t callRouter(const String_t& cmd);
    
    RouterContextFactory& m_parent;
    String_t m_sessionName;
};

// Call a command.
bool
server::console::RouterContextFactory::Impl::call(const String_t& cmd, interpreter::Arguments args, Parser& parser, std::auto_ptr<afl::data::Value>& result)
{
    // ex RouterContext::processCommand
    if (cmd == "repeat") {
        // Process command repeatedly, for benchmarking
        args.checkArgumentCountAtLeast(2);

        // Get repeat count
        int32_t n = 0;
        if (!afl::string::strToInteger(toString(args.getNext()), n) || n <= 0) {
            throw std::runtime_error("Expecting number");
        }

        // Get command
        String_t verb = toString(args.getNext());

        // Do it
        uint32_t startTicks = afl::sys::Time::getTickCounter();
        for (int32_t i = 0; i < n; ++i) {
            std::auto_ptr<afl::data::Value> p;
            if (isSessionCommand(m_sessionName, verb, args)) {
                processSessionCommand(m_sessionName, verb, args, p);
            } else {
                processCommand(verb, args, p);
            }
        }
        uint32_t endTicks = afl::sys::Time::getTickCounter();
        uint32_t elapsed = endTicks - startTicks;

        // Return
        result.reset(makeStringValue(afl::string::Format("%d.%03d seconds (%d ms per iteration)", elapsed / 1000, elapsed % 1000, elapsed / n)));
        return true;
    } else if (cmd == "s") {
        // Select session
        args.checkArgumentCountAtLeast(1);

        String_t sessionName = toString(args.getNext());
        if (args.getNumArgs() == 0) {
            parser.terminal().printMessage(afl::string::Format("Selected session '%s'.", sessionName));
            m_sessionName = sessionName;
            return true;
        } else {
            processSessionCommand(sessionName, toString(args.getNext()), args, result);
            return true;
        }
    } else if (isSessionCommand(m_sessionName, cmd, args)) {
        // command directed at session
        processSessionCommand(m_sessionName, cmd, args, result);
        return true;
    } else {
        // raw command
        processCommand(cmd, args, result);
        return true;
    }
}

// Get name of this context.
String_t
server::console::RouterContextFactory::Impl::getName()
{
    // ex RouterContext::getPrefix
    if (m_sessionName.empty()) {
        return m_parent.getCommandName();
    } else {
        return afl::string::Format("%s:%s", m_parent.getCommandName(), m_sessionName);
    }
}

void
server::console::RouterContextFactory::Impl::processCommand(const String_t& cmd, interpreter::Arguments args, std::auto_ptr<afl::data::Value>& result)
{
    // ex RouterContext::processCommand
    String_t commandLine = cmd;
    while (args.getNumArgs() > 0) {
        commandLine += ' ';
        commandLine += toString(args.getNext());
    }
    result.reset(makeStringValue(callRouter(commandLine)));
}

void
server::console::RouterContextFactory::Impl::processSessionCommand(const String_t& session, const String_t& cmd, interpreter::Arguments args, std::auto_ptr<afl::data::Value>& result)
{
    // ex RouterContext::processSessionCommand
    String_t commandLine = afl::string::Format("S %s\n%s", session, cmd);
    while (args.getNumArgs() > 0) {
        commandLine += ' ';
        commandLine += toString(args.getNext());
    }
    result.reset(makeStringValue(callRouter(commandLine)));
}


String_t
server::console::RouterContextFactory::Impl::callRouter(const String_t& cmd)
{
    afl::net::line::SimpleQuery c(cmd);
    afl::net::line::Client(m_parent.m_networkStack, m_parent.m_address).call(c);
    return c.getResult();
}


server::console::RouterContextFactory::RouterContextFactory(String_t name, afl::net::NetworkStack& stack)
    : ContextFactory(),
      m_name(name),
      m_address(DEFAULT_ADDRESS, ROUTER_PORT),
      m_networkStack(stack)
{ }
      
server::console::RouterContextFactory::~RouterContextFactory()
{ }

// Get name of command used to invoke this context.
String_t
server::console::RouterContextFactory::getCommandName()
{
    return m_name;
}

// Create the associated context.
server::console::Context*
server::console::RouterContextFactory::create()
{
    return new Impl(*this);
}

// Handle configuration option.
bool
server::console::RouterContextFactory::handleConfiguration(const String_t& key, const String_t& value)
{
    // ex RouterContext::checkConfig
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
