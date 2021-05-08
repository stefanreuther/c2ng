/**
  *  \file server/format/serverapplication.cpp
  *  \brief Class server::format::ServerApplication
  */

#include "server/format/serverapplication.hpp"
#include "afl/net/commandhandler.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/resp/protocolhandler.hpp"
#include "afl/net/server.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/thread.hpp"
#include "server/format/format.hpp"
#include "server/interface/formatserver.hpp"
#include "server/ports.hpp"
#include "version.hpp"

using afl::async::InterruptOperation;

namespace {
    const char LOG_NAME[] = "format";

    class ProtocolHandlerFactory : public afl::net::ProtocolHandlerFactory {
     public:
        ProtocolHandlerFactory(afl::net::CommandHandler& ch)
            : m_commandHandler(ch)
            { }
        virtual afl::net::ProtocolHandler* create()
            { return new afl::net::resp::ProtocolHandler(m_commandHandler); }
     private:
        afl::net::CommandHandler& m_commandHandler;
    };
}

server::format::ServerApplication::ServerApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net, afl::async::Interrupt& intr)
    : Application(LOG_NAME, env, fs, net),
      m_listenAddress(DEFAULT_ADDRESS, FORMAT_PORT),
      m_interrupt(intr)
{ }

server::format::ServerApplication::~ServerApplication()
{ }

void
server::format::ServerApplication::serverMain()
{
    // Server implementation (stateless)
    Format fmt;

    // Command handler (stateless)
    server::interface::FormatServer fs(fmt);

    // Protocol Handler factory
    ProtocolHandlerFactory factory(fs);

    // Server
    afl::net::Server server(networkStack().listen(m_listenAddress, 10), factory);
    log().write(afl::sys::LogListener::Info, "format", afl::string::Format("Listening on %s", m_listenAddress.toString()));

    // Server thread
    afl::sys::Thread serverThread("format.server", server);
    serverThread.start();

    // Wait for termination request
    afl::async::Controller ctl;
    m_interrupt.wait(ctl, InterruptOperation::Kinds_t() + InterruptOperation::Break + InterruptOperation::Terminate);

    // Stop
    log().write(afl::sys::LogListener::Info, LOG_NAME, "Received stop signal, shutting down.");
    server.stop();
    serverThread.join();
}

bool
server::format::ServerApplication::handleConfiguration(const String_t& key, const String_t& value)
{
    // ex planetscentral/format/format.cc:processConfig
    if (key == "FORMAT.HOST") {
        /* @q Format.Host:Str (Config)
           Listen address for the Format instance. */
        m_listenAddress.setName(value);
        return true;
    } else if (key == "FORMAT.PORT") {
        /* @q Format.Port:Int (Config)
           Port number for the Format instance. */
        m_listenAddress.setService(value);
        return true;
    } else if (key == "FORMAT.THREADS") {
        /* @q Format.Threads:Int (Config)
           Ignored in c2ng/c2format-server for compatibility reasons.
           Number of threads (=maximum number of parallel connections). */
        return true;
    } else {
        return false;
    }
}

bool
server::format::ServerApplication::handleCommandLineOption(const String_t& /*option*/, afl::sys::CommandLineParser& /*parser*/)
{
    return false;
}

String_t
server::format::ServerApplication::getApplicationName() const
{
    return afl::string::Format("PCC2 Format Server v%s - (c) 2017-2021 Stefan Reuther", PCC2_VERSION);
}

String_t
server::format::ServerApplication::getCommandLineOptionHelp() const
{
    return String_t();
}
