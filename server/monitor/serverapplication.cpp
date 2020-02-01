/**
  *  \file server/monitor/serverapplication.cpp
  *  \brief Class server::monitor::ServerApplication
  */

#include "server/monitor/serverapplication.hpp"
#include "afl/except/commandlineexception.hpp"
#include "afl/net/http/pagedispatcher.hpp"
#include "afl/net/http/protocolhandler.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/server.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/thread.hpp"
#include "afl/sys/time.hpp"
#include "server/monitor/badnessfileobserver.hpp"
#include "server/monitor/loadaverageobserver.hpp"
#include "server/monitor/networkobserver.hpp"
#include "server/monitor/statuspage.hpp"
#include "server/ports.hpp"
#include "util/string.hpp"
#include "version.hpp"

using afl::async::InterruptOperation;
using afl::string::Format;

namespace {
    const char*const LOG_NAME = "monitor";

    /* Default update interval in seconds */
    const int32_t DEFAULT_UPDATE_INTERVAL = 60;

    /* Default save interval in seconds */
    const int32_t DEFAULT_SAVE_INTERVAL = 3600;

    class ProtocolHandlerFactory : public afl::net::ProtocolHandlerFactory {
     public:
        ProtocolHandlerFactory(afl::net::http::Dispatcher& disp)
            : m_dispatcher(disp)
            { }
        virtual afl::net::ProtocolHandler* create()
            { return new afl::net::http::ProtocolHandler(m_dispatcher); }
     private:
        afl::net::http::Dispatcher& m_dispatcher;
    };
}

server::monitor::ServerApplication::ServerApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net, afl::async::Interrupt& intr)
    : Application(LOG_NAME, env, fs, net),
      m_listenAddress(DEFAULT_ADDRESS, MONITOR_PORT),
      m_interrupt(intr),
      m_templateFileName("share/server/monitor/monitor.html"),
      m_statusFileName(),
      m_updateInterval(DEFAULT_UPDATE_INTERVAL),
      m_saveInterval(DEFAULT_SAVE_INTERVAL),
      m_status()
{
    m_status.addNewObserver(new NetworkObserver("Web Server",       "WWW",      NetworkObserver::Web,     clientNetworkStack(), afl::net::Name(DEFAULT_ADDRESS, WWW_PORT)));
    m_status.addNewObserver(new NetworkObserver("PCC2 Web",         "ROUTER",   NetworkObserver::Router,  clientNetworkStack(), afl::net::Name(DEFAULT_ADDRESS, ROUTER_PORT)));
    m_status.addNewObserver(new NetworkObserver("Database",         "REDIS",    NetworkObserver::Redis,   clientNetworkStack(), afl::net::Name(DEFAULT_ADDRESS, DB_PORT)));
    m_status.addNewObserver(new NetworkObserver("User File Server", "FILE",     NetworkObserver::Service, clientNetworkStack(), afl::net::Name(DEFAULT_ADDRESS, FILE_PORT)));
    m_status.addNewObserver(new NetworkObserver("Host File Server", "HOSTFILE", NetworkObserver::Service, clientNetworkStack(), afl::net::Name(DEFAULT_ADDRESS, HOSTFILE_PORT)));
    m_status.addNewObserver(new NetworkObserver("Host Manager",     "HOST",     NetworkObserver::Service, clientNetworkStack(), afl::net::Name(DEFAULT_ADDRESS, HOST_PORT)));
    m_status.addNewObserver(new NetworkObserver("Mail Manager",     "MAILOUT",  NetworkObserver::Service, clientNetworkStack(), afl::net::Name(DEFAULT_ADDRESS, MAILOUT_PORT)));
    m_status.addNewObserver(new NetworkObserver("User Manager",     "USER",     NetworkObserver::Service, clientNetworkStack(), afl::net::Name(DEFAULT_ADDRESS, USER_PORT)));
    m_status.addNewObserver(new NetworkObserver("Forum",            "TALK",     NetworkObserver::Service, clientNetworkStack(), afl::net::Name(DEFAULT_ADDRESS, TALK_PORT)));
    m_status.addNewObserver(new NetworkObserver("Binary File I/O",  "FORMAT",   NetworkObserver::Service, clientNetworkStack(), afl::net::Name(DEFAULT_ADDRESS, FORMAT_PORT)));
    m_status.addNewObserver(new BadnessFileObserver("Mail Fetch", "POP3.ERROR", fileSystem()));
    m_status.addNewObserver(new LoadAverageObserver(fileSystem(), "/proc/loadavg"));
}

server::monitor::ServerApplication::~ServerApplication()
{ }

// server::Application
bool
server::monitor::ServerApplication::handleCommandLineOption(const String_t& /*option*/, afl::sys::CommandLineParser& /*parser*/)
{
    return false;
}

void
server::monitor::ServerApplication::serverMain()
{
    // Setup
    m_status.log().addListener(log());

    // Load status
    if (!m_statusFileName.empty()) {
        afl::base::Ptr<afl::io::Stream> file = fileSystem().openFileNT(m_statusFileName, afl::io::FileSystem::OpenRead);
        if (file.get() != 0) {
            m_status.load(*file);
            log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("Status read from \"%s\".", m_statusFileName));
        }
    }

    // Initial update
    m_status.update();

    // Set up HTTP infrastructure
    afl::net::http::PageDispatcher disp;
    disp.addNewPage("/", new StatusPage(m_status, fileSystem(), m_templateFileName));

    // Run it
    ProtocolHandlerFactory factory(disp);
    afl::net::Server server(networkStack().listen(m_listenAddress, 10), factory);
    log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("Listening on %s", m_listenAddress.toString()));
    afl::sys::Thread serverThread("monitor.server", server);
    serverThread.start();

    // Wait for termination request
    afl::async::Controller ctl;
    uint32_t lastSaveTime = afl::sys::Time::getTickCounter();
    while (m_interrupt.wait(ctl, InterruptOperation::Kinds_t() + InterruptOperation::Break + InterruptOperation::Terminate, 1000 * m_updateInterval).empty()) {
        m_status.update();

        uint32_t now = afl::sys::Time::getTickCounter();
        if (static_cast<int32_t>((now - lastSaveTime) / 1000) >= m_saveInterval) {
            doSave();
            lastSaveTime = now;
        }
    }

    // Stop
    log().write(afl::sys::LogListener::Info, LOG_NAME, "Received stop signal, shutting down.");
    server.stop();
    serverThread.join();

    // Save status
    doSave();
}

bool
server::monitor::ServerApplication::handleConfiguration(const String_t& key, const String_t& value)
{
    // ex planetscentral/monitor.cc:processConfig
    // Check all children
    bool childHandled = m_status.handleConfiguration(key, value);

    // Check own configuration - even if a child already accepted it.
    if (key == "MONITOR.HOST") {
        /* @q Monitor.Host:Str (Config)
           Listen address for the status monitor. */
        m_listenAddress.setName(value);
        return true;
    } else if (key == "MONITOR.PORT") {
        /* @q Monitor.Port:Str (Config)
           Port number for the status monitor. */
        m_listenAddress.setService(value);
        return true;
    } else if (key == "MONITOR.TEMPLATE") {
        /* @q Monitor.Template:Str (Config)
           Name of file containing the HTML template for the status monitor. */
        m_templateFileName = value;
        return true;
    } else if (key == "MONITOR.INTERVAL") {
        /* @q Monitor.Interval:Int (Config)
           Interval between two checks, in seconds.
           @change Note that whereas c2monitor-classic updates on user requests and thus this was a minimum interval (=maximum rate),
           c2monitor-ng will permanently poll in the interval given.
           Whereas a typical value for -classic would be 10, typical values for -ng are 60..300. */
        int32_t n;
        if (!afl::string::strToInteger(value, n) || n <= 0 || n > 86400) {
            throw afl::except::CommandLineException(afl::string::Format("Invalid number for '%s'", key));
        }
        m_updateInterval = n;
        return true;
    } else if (key == "MONITOR.SAVEINTERVAL") {
        /* @q Monitor.SaveInterval:Int (Config)
           Interval for saving the history file, in seconds.
           @since PCC2 2.40.5 */
        int32_t n;
        if (!afl::string::strToInteger(value, n) || n <= 0 || n > 40*86400) {
            throw afl::except::CommandLineException(afl::string::Format("Invalid number for '%s'", key));
        }
        m_saveInterval = n;
        return true;
    } else if (key == "MONITOR.HISTORYFILE") {
        /* @q Monitor.HistoryFile:Str (Config)
           Name of history file.
           History is persisted across monitor restarts in this file.
           @since PCC2 2.40.3 */
        m_statusFileName = value;
        return true;
    } else {
        return childHandled;
    }
}

String_t
server::monitor::ServerApplication::getApplicationName() const
{
    return afl::string::Format("PCC2 Monitor Server v%s - (c) 2017-2020 Stefan Reuther", PCC2_VERSION);
}

String_t
server::monitor::ServerApplication::getCommandLineOptionHelp() const
{
    return String_t();
}

void
server::monitor::ServerApplication::doSave()
{
    if (!m_statusFileName.empty()) {
        afl::base::Ref<afl::io::Stream> file = fileSystem().openFile(m_statusFileName, afl::io::FileSystem::Create);
        m_status.save(*file);
        log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("Status saved to \"%s\".", m_statusFileName));
    }
}
