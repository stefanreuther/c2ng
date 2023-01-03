/**
  *  \file server/router/serverapplication.cpp
  */

#include "server/router/serverapplication.hpp"
#include "afl/async/controller.hpp"
#include "afl/base/deleter.hpp"
#include "afl/except/commandlineexception.hpp"
#include "afl/net/line/protocolhandler.hpp"
#include "afl/net/protocolhandler.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/server.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/thread.hpp"
#include "server/common/numericalidgenerator.hpp"
#include "server/common/randomidgenerator.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/sessionroutersingleserver.hpp"
#include "server/ports.hpp"
#include "server/router/root.hpp"
#include "server/router/sessionrouter.hpp"
#include "util/string.hpp"
#include "version.hpp"

namespace {
    const char*const LOG_NAME = "router";

    using afl::async::InterruptOperation;

    template<typename Impl, typename Server>
    class ProtocolHandler : public afl::net::ProtocolHandler {
     public:
        ProtocolHandler(Impl& impl)
            : m_server(impl),
              m_protocolHandler(m_server)
            { }

        // ProtocolHandler:
        virtual void getOperation(Operation& op)
            { m_protocolHandler.getOperation(op); }
        virtual void advanceTime(afl::sys::Timeout_t msecs)
            { m_protocolHandler.advanceTime(msecs); }
        virtual void handleData(afl::base::ConstBytes_t bytes)
            { m_protocolHandler.handleData(bytes); }
        virtual void handleSendTimeout(afl::base::ConstBytes_t unsentBytes)
            { m_protocolHandler.handleSendTimeout(unsentBytes); }
        virtual void handleConnectionClose()
            { m_protocolHandler.handleConnectionClose(); }

     private:
        Server m_server;
        afl::net::line::ProtocolHandler m_protocolHandler;
    };

    template<typename Impl, typename Server>
    class StatelessProtocolHandlerFactory : public afl::net::ProtocolHandlerFactory {
     public:
        typedef ProtocolHandler<Impl, Server> ProtocolHandler_t;

        explicit StatelessProtocolHandlerFactory(Impl& impl)
            : m_impl(impl)
            { }

        virtual ProtocolHandler_t* create()
            { return new ProtocolHandler_t(m_impl); }

     private:
        Impl& m_impl;
    };
}

server::router::ServerApplication::ServerApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net, afl::async::Interrupt& intr, util::process::Factory& factory)
    : Application(LOG_NAME, env, fs, net),
      m_listenAddress(DEFAULT_ADDRESS, ROUTER_PORT),
      m_fileAddress(DEFAULT_ADDRESS, FILE_PORT),
      m_interrupt(intr),
      m_factory(factory),
      m_generator(new server::common::NumericalIdGenerator()),
      m_enableFileNotify(true),
      m_config()
{ }

void
server::router::ServerApplication::serverMain()
{
    // Connect to file server if requested
    server::interface::FileBase* pFileBase = 0;
    afl::base::Deleter del;
    if (m_enableFileNotify && m_fileAddress.getName().empty()) {
        log().write(afl::sys::LogListener::Warn, LOG_NAME, "FILE.HOST not set, disabling ROUTER.FILENOTIFY");
        m_enableFileNotify = false;
    }
    if (m_enableFileNotify) {
        afl::net::CommandHandler& hdl = createClient(m_fileAddress, del, true);
        pFileBase = &del.addNew(new server::interface::FileBaseClient(hdl));
    }

    // Set up root (global data)
    Root root(m_factory, *m_generator, m_config, pFileBase);
    root.log().addListener(log());

    // Protocol Handler
    SessionRouter impl(root);
    StatelessProtocolHandlerFactory<SessionRouter, server::interface::SessionRouterSingleServer> factory(impl);

    // Server
    afl::net::Server server(networkStack().listen(m_listenAddress, 10), factory);
    log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("Listening on %s", m_listenAddress.toString()));
    // server.log().addListener(log());

    // Server thread
    afl::sys::Thread serverThread("router.server", server);
    serverThread.start();

    // Wait for termination request
    afl::async::Controller ctl;
    m_interrupt.wait(ctl, InterruptOperation::Kinds_t() + InterruptOperation::Break + InterruptOperation::Terminate);

    // Stop
    log().write(afl::sys::LogListener::Info, LOG_NAME, "Received stop signal, shutting down.");
    root.stopAllSessions();
    server.stop();
    serverThread.join();
    log().write(afl::sys::LogListener::Info, LOG_NAME, "Done.");
}

bool
server::router::ServerApplication::handleConfiguration(const String_t& key, const String_t& value)
{
    // ex router.cc:handleRouterConfig
    if (key == "ROUTER.HOST") {
        /* @q Router.Host:Str (Config)
           Listen address for Router service. */
        m_listenAddress.setName(value);
        return true;
    } else if (key == "ROUTER.PORT") {
        /* @q Router.Port:Int (Config)
           Port number for Router service. */
        m_listenAddress.setService(value);
        return true;
    } else if (key == "ROUTER.SERVER") {
        /* @q Router.Server:Str (Config)
           File name of %c2server (c2play-server) binary. */
        m_config.serverPath = value;
        return true;
    } else if (key == "ROUTER.TIMEOUT") {
        /* @q Router.Timeout:Int (Config)
           Session timeout in seconds.
           A session will be terminated if it has not been accessed within this time. */
        int32_t n;
        if (afl::string::strToInteger(value, n) && n > 0) {
            m_config.normalTimeout = n;
        } else {
            throw afl::except::CommandLineException(afl::string::Format("Invalid value for '%s'", key));
        }
        return true;
    } else if (key == "ROUTER.VIRGINTIMEOUT") {
        /* @q Router.VirginTimeout:Int (Config)
           Session timeout in seconds for virgin (unaccessed) sessions.
           A session will be terminated if it has not been accessed within this time directly after creation.
           This happens when a user starts a session but their browser has trouble with the JavaScript. */
        int32_t n;
        if (afl::string::strToInteger(value, n) && n > 0) {
            m_config.virginTimeout = n;
        } else {
            throw afl::except::CommandLineException(afl::string::Format("Invalid value for '%s'", key));
        }
        return true;
    } else if (key == "ROUTER.MAXSESSIONS") {
        /* @q Router.MaxSessions:Int (Config)
           Maximum number of concurrent sessions. */
        size_t n;
        if (afl::string::strToInteger(value, n) && n > 0) {
            m_config.maxSessions = n;
        } else {
            throw afl::except::CommandLineException(afl::string::Format("Invalid value for '%s'", key));
        }
        return true;
    } else if (key == "ROUTER.NEWSESSIONSWIN") {
        /* @q Router.NewSessionsWin:Str (Config)
           Determines behaviour when two conflicting sessions are started ("-W" and "-R" flags).
           If "y" or "1", new sessions that conflict with old ones cause the old ones to terminate.
           If "n" or "0", the new session will be refused. */
        if (!util::parseBooleanValue(value, m_config.newSessionsWin)) {
            throw afl::except::CommandLineException(afl::string::Format("Invalid value for '%s'", key));
        }
        return true;
    } else if (key == "ROUTER.FILENOTIFY") {
        /* @q Router.FileNotify:Str (Config)
           If "y" or "1", the {SAVE (Router Command)|SAVE} command will notify the {File (Service)|file server}. */
        if (!util::parseBooleanValue(value, m_enableFileNotify)) {
            throw afl::except::CommandLineException(afl::string::Format("Invalid value for '%s'", key));
        }
        return true;
    } else if (key == "ROUTER.SESSIONID") {
        /* @q Router.SessionId:Str (Config)
           Select session Id generation algorithm.
           - "numeric": simple counter (classic, default); needs outside protection against Id guessing.
             Original front-end assumed numerical Ids.
           - "random": hex string; needs no extra protection
           @since PCC2 2.40.6 */
        if (afl::string::strCaseCompare(value, "numeric") == 0) {
            m_generator.reset(new server::common::NumericalIdGenerator());
        } else if (afl::string::strCaseCompare(value, "random") == 0) {
            m_generator.reset(new server::common::RandomIdGenerator(fileSystem()));
        } else {
            throw afl::except::CommandLineException(afl::string::Format("Invalid value for '%s'", key));
        }
        return true;
    } else if (key == "FILE.HOST") {
        m_fileAddress.setName(value);
        return true;
    } else if (key == "FILE.PORT") {
        m_fileAddress.setService(value);
        return true;
    } else {
        return false;
    }
}

bool
server::router::ServerApplication::handleCommandLineOption(const String_t& /*option*/, afl::sys::CommandLineParser& /*parser*/)
{
    return false;
}

String_t
server::router::ServerApplication::getApplicationName() const
{
    return afl::string::Format("PCC2 Router Server v%s - (c) 2019-2023 Stefan Reuther", PCC2_VERSION);
}

String_t
server::router::ServerApplication::getCommandLineOptionHelp() const
{
    return String_t();
}
