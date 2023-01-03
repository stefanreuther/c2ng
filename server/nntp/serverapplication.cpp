/**
  *  \file server/nntp/serverapplication.cpp
  *  \brief Class server::nntp::ServerApplication
  */

#include "server/nntp/serverapplication.hpp"
#include "afl/async/controller.hpp"
#include "afl/net/line/protocolhandler.hpp"
#include "afl/net/server.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/thread.hpp"
#include "server/common/sessionprotocolhandlerfactory.hpp"
#include "server/nntp/linehandler.hpp"
#include "server/nntp/root.hpp"
#include "server/nntp/session.hpp"
#include "server/ports.hpp"
#include "util/string.hpp"
#include "version.hpp"

using afl::async::InterruptOperation;

namespace {
    const char LOG_NAME[] = "nntp";
}

server::nntp::ServerApplication::ServerApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net, afl::async::Interrupt& intr)
    : Application(LOG_NAME, env, fs, net),
      m_listenAddress(DEFAULT_ADDRESS, NNTP_PORT),
      m_talkAddress(DEFAULT_ADDRESS, TALK_PORT),
      m_userAddress(DEFAULT_ADDRESS, USER_PORT),
      m_baseUrl(),
      m_interrupt(intr)
{ }

// Destructor.
server::nntp::ServerApplication::~ServerApplication()
{ }

// server::Application
bool
server::nntp::ServerApplication::handleCommandLineOption(const String_t& /*option*/, afl::sys::CommandLineParser& /*parser*/)
{
    return false;
}

void
server::nntp::ServerApplication::serverMain()
{
    // Connect to server.
    // Talk is stateful, so it cannot be auto-reconnect.
    afl::base::Deleter del;
    afl::net::CommandHandler& talk(createClient(m_talkAddress, del, false));
    afl::net::CommandHandler& user(createClient(m_userAddress, del, true));

    // Set up root (global data)
    Root root(talk, user, m_baseUrl);
    root.log().addListener(log());

    // Protocol Handler
    server::common::SessionProtocolHandlerFactory<Root, Session, afl::net::line::ProtocolHandler, LineHandler> factory(root);

    // Server
    // FIXME: -classic would accept a socket from environment:
    //     if (char* s = getenv("C2SOCKET")) {
    //         char* result;
    //         server_socket = strtol(s, &result, 0);
    //         if (result == 0 || *result != '\0' || server_socket < 0) {
    //             fprintf(stderr, "[main] unable to parse provided socket '%s'\n", s);
    //             return 1;
    //         }
    //     }
    afl::net::Server server(networkStack().listen(m_listenAddress, 10), factory);
    log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("Listening on %s", m_listenAddress.toString()));

    // Server thread
    afl::sys::Thread serverThread("nntp.server", server);
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
server::nntp::ServerApplication::handleConfiguration(const String_t& key, const String_t& value)
{
    // ex planetscentral/nntp/nntp.cc:processConfig
    if (key == "NNTP.HOST") {
        m_listenAddress.setName(value);
        return true;
    } else if (key == "NNTP.PORT") {
        m_listenAddress.setService(value);
        return true;
    } else if (key == "USER.HOST") {
        m_userAddress.setName(value);
        return true;
    } else if (key == "USER.PORT") {
        m_userAddress.setService(value);
        return true;
    } else if (key == "TALK.HOST") {
        m_talkAddress.setName(value);
        return true;
    } else if (key == "TALK.PORT") {
        m_talkAddress.setService(value);
        return true;
    } else if (key == "TALK.WWWROOT") {
        m_baseUrl = value;
        return true;
    } else {
        // ignore
        return false;
    }
}

String_t
server::nntp::ServerApplication::getApplicationName() const
{
    return afl::string::Format("PCC2 NNTP Server v%s - (c) 2017-2023 Stefan Reuther", PCC2_VERSION);
}

String_t
server::nntp::ServerApplication::getCommandLineOptionHelp() const
{
    return String_t();
}
