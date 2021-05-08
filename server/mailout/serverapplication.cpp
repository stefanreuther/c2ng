/**
  *  \file server/mailout/serverapplication.cpp
  *  \brief Class server::mailout::ServerApplication
  */

#include "server/mailout/serverapplication.hpp"
#include "afl/async/controller.hpp"
#include "afl/async/interruptoperation.hpp"
#include "afl/except/commandlineexception.hpp"
#include "afl/net/resp/protocolhandler.hpp"
#include "afl/net/server.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/thread.hpp"
#include "server/common/sessionprotocolhandlerfactory.hpp"
#include "server/mailout/commandhandler.hpp"
#include "server/mailout/root.hpp"
#include "server/mailout/session.hpp"
#include "server/mailout/transmitterimpl.hpp"
#include "server/ports.hpp"
#include "util/string.hpp"
#include "version.hpp"

namespace {
    const char* LOG_NAME = "mailout";
}


server::mailout::ServerApplication::ServerApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net, afl::async::Interrupt& intr)
    : Application(LOG_NAME, env, fs, net),
      m_listenAddress(DEFAULT_ADDRESS, MAILOUT_PORT),
      m_dbAddress(DEFAULT_ADDRESS, DB_PORT),
      m_smtpAddress(DEFAULT_ADDRESS, SMTP_PORT),
      m_smtpConfig("unconfigured", "unconfigured@invalid"),
      m_config(),
      m_templateDirectoryName("."),
      m_interrupt(intr)
{
    // @diff PCC2 attempted to determine the FQDN (hello string, first arg of smtp::Configuration) from gethostbyname().
    // We don't have an abstraction for that yet, and configure it manually anyway.
}

// Destructor.
server::mailout::ServerApplication::~ServerApplication()
{ }

// server::Application
bool
server::mailout::ServerApplication::handleCommandLineOption(const String_t& option, afl::sys::CommandLineParser& /*parser*/)
{
    if (option == "notx") {
        m_config.useTransmitter = false;
        return true;
    } else {
        return false;
    }
}

void
server::mailout::ServerApplication::serverMain()
{
    using afl::async::InterruptOperation;

    // Connect to server
    afl::base::Deleter del;
    afl::net::CommandHandler& db(createClient(m_dbAddress, del, true));

    // Root
    Root root(db, m_config);
    root.log().addListener(log());

    // Transmitter.
    // Making this a separate auto_ptr means it dies first, before the Root.
    // This is required because Transmitter will access the root.
    // At that point, no other component will be alive that attempts to access Root::getTransmitter().
    std::auto_ptr<Transmitter> tx;
    if (m_config.useTransmitter) {
        // Open template directory and verify that it is ok. getDirectoryEntries() will fail if it's not.
        afl::base::Ref<afl::io::Directory> templateDir = fileSystem().openDirectory(m_templateDirectoryName);
        templateDir->getDirectoryEntries();

        // Create transmitter. Need an intermediate upcast, otherwise the compiler sees an ambiguity re Deletable.
        tx.reset(new TransmitterImpl(root, templateDir, networkStack(), m_smtpAddress, m_smtpConfig));
        root.setTransmitter(tx.get());
        log().write(afl::sys::LogListener::Info, LOG_NAME, "Transmitter enabled.");
    } else {
        log().write(afl::sys::LogListener::Info, LOG_NAME, "Transmitter disabled.");
    }

    // Initialize queues
    root.prepareQueues();

    // Protocol Handler
    server::common::SessionProtocolHandlerFactory<Root, Session, afl::net::resp::ProtocolHandler, CommandHandler> factory(root);

    // Server
    afl::net::Server server(networkStack().listen(m_listenAddress, 10), factory);
    log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("Listening on %s", m_listenAddress.toString()));

    // Server thread
    afl::sys::Thread serverThread("mailout.server", server);
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
server::mailout::ServerApplication::handleConfiguration(const String_t& key, const String_t& value)
{
    // ex Smtp::checkConfig, planetscentral/mailout/mailout.cc:processConfig, Transmitter::checkConfig
    if (key == "SMTP.HOST") {
        /* @q SMTP.Host:Str (Config)
           Host name of SMTP server. */
        m_smtpAddress.setName(value);
        return true;
    } else if (key == "SMTP.PORT") {
        /* @q SMTP.Port:Int (Config)
           Port number of SMTP server. */
        m_smtpAddress.setService(value);
        return true;
    } else if (key == "SMTP.FROM") {
        /* @q SMTP.From:Str (Config)
           Mail address to use as originator in SMTP "MAIL FROM". */
        m_smtpConfig.from = value;
        return true;
    } else if (key == "SMTP.FQDN") {
        /* @q SMTP.FQDN:Str (Config)
           Fully-qualified domain name to use as originator in SMTP "HELO". */
        m_smtpConfig.hello = value;
        return true;
    } else if (key == "WWW.KEY") {
        m_config.confirmationKey = value;
        return true;
    } else if (key == "WWW.URL") {
        m_config.baseUrl = value;
        return true;
    } else if (key == "MAILOUT.MAXAGE") {
        /* @q Mailout.MaxAge:Int (Config)
           Maximum age of a message, in minutes.
           A message that could not been sent for this time is dropped. */
        int n;
        if (afl::string::strToInteger(value, n)) {
            m_config.maximumAge = n;
        } else {
            throw afl::except::CommandLineException(afl::string::Format("Invalid number for '%s'", key));
        }
        return true;
    } else if (key == "MAILOUT.HOST") {
        /* @q Mailout.Host:Str (Config)
           Listen address for the Mailout service. */
        m_listenAddress.setName(value);
        return true;
    } else if (key == "MAILOUT.PORT") {
        /* @q Mailout.Port:Int (Config)
           Port number for the Mailout service. */
        m_listenAddress.setService(value);
        return true;
    } else if (key == "MAILOUT.THREADS") {
        /* @q Mailout.Threads:Int (Config)
           Ignored in c2ng/c2mailout-server for compatibility reasons; maximum number of connections is not limited.
           Number of threads (=maximum number of parallel connections). */
        return true;
    } else if (key == "MAILOUT.TEMPLATEDIR") {
        /* @q Mailout.TemplateDir:Str (Config)
           Directory containing template files for outgoing mails. */
        m_templateDirectoryName = value;
        return true;
    } else if (key == "REDIS.HOST") {
        m_dbAddress.setName(value);
        return true;
    } else if (key == "REDIS.PORT") {
        m_dbAddress.setService(value);
        return true;
    } else {
        return false;
    }
}

String_t
server::mailout::ServerApplication::getApplicationName() const
{
    return afl::string::Format("PCC2 Mail Queue Server v%s - (c) 2017-2021 Stefan Reuther", PCC2_VERSION);
}

String_t
server::mailout::ServerApplication::getCommandLineOptionHelp() const
{
    return "--notx\tDisable transmitter\n";
}
