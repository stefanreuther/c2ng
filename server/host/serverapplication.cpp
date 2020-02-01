/**
  *  \file server/host/serverapplication.cpp
  *  \brief Class server::host::ServerApplication
  */

#include "server/host/serverapplication.hpp"
#include "afl/except/commandlineexception.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/net/protocolhandler.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/resp/protocolhandler.hpp"
#include "afl/net/server.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "server/common/sessionprotocolhandler.hpp"
#include "server/common/sessionprotocolhandlerfactory.hpp"
#include "server/host/commandhandler.hpp"
#include "server/host/cron.hpp"
#include "server/host/cronimpl.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "server/host/talkadapter.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "server/interface/sessionrouterclient.hpp"
#include "server/interface/talkforumclient.hpp"
#include "server/ports.hpp"
#include "util/processrunner.hpp"
#include "util/string.hpp"
#include "version.hpp"

using afl::async::InterruptOperation;

namespace {
    const char LOG_NAME[] = "host";
}

server::host::ServerApplication::ServerApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net, afl::async::Interrupt& intr)
    : Application(LOG_NAME, env, fs, net),
      m_listenAddress(DEFAULT_ADDRESS, HOST_PORT),
      m_dbAddress(DEFAULT_ADDRESS, DB_PORT),
      m_userFileAddress(DEFAULT_ADDRESS, FILE_PORT),
      m_talkAddress(DEFAULT_ADDRESS, TALK_PORT),
      m_mailAddress(DEFAULT_ADDRESS, MAILOUT_PORT),
      m_routerAddress(DEFAULT_ADDRESS, ROUTER_PORT),
      m_config(),
      m_interrupt(intr)
{
    m_config.binDirectory = env.getInstallationDirectoryName();

    try {
        afl::base::Ref<afl::io::DirectoryEntry> entry = fs.openDirectory(m_config.binDirectory)->getDirectoryEntryByName("bin");
        if (entry->getFileType() == afl::io::DirectoryEntry::tDirectory) {
            m_config.binDirectory = entry->getPathName();
        }
    }
    catch (...)
    {
        // Ignore errors. These mean 'bin' does not exist.
    }
}

server::host::ServerApplication::~ServerApplication()
{ }

bool
server::host::ServerApplication::handleCommandLineOption(const String_t& option, afl::sys::CommandLineParser& /*parser*/)
{
    if (option == "nocron") {
        m_config.useCron = false;
        return true;
    } else {
        return false;
    }
}

void
server::host::ServerApplication::serverMain()
{
    // Runners [create these before starting other stuff].
    // The main requirement is to create these before creating threads that do unpredictable things with file descriptors.
    // Creating them further down below (in particular, creating the hostRunner only if cron is actually enabled),
    // is now feasible as all afl components properly set FD_CLOEXEC. However, this complicates matters, and having the
    // extra process around and wasting a few kilobytes of memory for a non-production usecase just isn't worth it.
    util::ProcessRunner checkturnRunner;
    util::ProcessRunner hostRunner;

    // Set up work directory
    setupWorkDirectory();

    // Connect to other services.
    // See also Root::configureReconnect()
    afl::base::Deleter del;
    afl::net::CommandHandler& db(createClient(m_dbAddress, del, true));
    afl::net::CommandHandler& hostFile(createClient(m_config.hostFileAddress, del, false));
    afl::net::CommandHandler& userFile(createClient(m_userFileAddress, del, false));
    afl::net::CommandHandler& mail(createClient(m_mailAddress, del, true));

    // Set up root (global data)
    server::interface::MailQueueClient mailClient(mail);
    Root root(db, hostFile, userFile, mailClient, checkturnRunner, fileSystem(), m_config);
    root.log().addListener(log());

    // Set up talk if desired
    if (!m_talkAddress.getName().empty()) {
        // We are only using stateless commands with the forum, so just use auto-reconnect.
        afl::net::CommandHandler& talk(createClient(m_talkAddress, del, true));
        server::interface::TalkForum& forumClient(del.addNew(new server::interface::TalkForumClient(talk)));
        root.setForum(&del.addNew(new TalkAdapter(forumClient)));
    }

    // Set up router if desired
    if (!m_routerAddress.getName().empty()) {
        root.setRouter(&del.addNew(new server::interface::SessionRouterClient(networkStack(), m_routerAddress)));
    }

    // Set up cron if desired
    std::auto_ptr<Cron> pCron;
    if (m_config.useCron) {
        pCron.reset(new CronImpl(root, hostRunner));
        if (m_config.initialSuspend > 0) {
            pCron->suspendScheduler(root.getTime() + m_config.initialSuspend);
        }
        log().write(afl::sys::LogListener::Info, LOG_NAME, "Scheduler enabled");
    } else {
        log().write(afl::sys::LogListener::Info, LOG_NAME, "Scheduler disabled");
    }

    // Protocol Handler
    server::common::SessionProtocolHandlerFactory<Root, Session, afl::net::resp::ProtocolHandler, CommandHandler> factory(root);

    // Server
    afl::net::Server server(networkStack().listen(m_listenAddress, 10), factory);
    log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("Listening on %s", m_listenAddress.toString()));

    // Server thread
    afl::sys::Thread serverThread("host.server", server);
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
server::host::ServerApplication::handleConfiguration(const String_t& key, const String_t& value)
{
    // ex planetscentral/host/host.cc:processConfig(string_t key, string_t value)
    if (key == "HOST.HOST") {
        /* @q Host.Host:Str (Config)
           Listen address. */
        m_listenAddress.setName(value);
        return true;
    } else if (key == "HOST.PORT") {
        /* @q Host.Port:Int (Config)
           Port number. */
        m_listenAddress.setService(value);
        return true;
    } else if (key == "HOST.TIMESCALE") {
        /* @q Host.TimeScale:Int (Config)
           Unix-time-to-{@type Time}-conversion.
           By default, this value is 60, making a time step of 1 equal to a minute.
           For testing, this value can be lowered to make the system run faster
           (i.e. at 1, a daily game runs every 24 minutes, not 24 hours). */
        int n;
        if (afl::string::strToInteger(value, n) && n > 0) {
            m_config.timeScale = n;
        } else {
            throw afl::except::CommandLineException(afl::string::Format("Invalid value for '%s'", key));
        }
        return true;
    } else if (key == "HOST.USERSSEETEMPORARYTURNS") {
        /* @q Host.UsersSeeTemporaryTurns:Bool (Config)
           If enabled, users see each others temporary turns.
           If disabled, users only see their own temporary status (original behaviour).
           c2ng/c2host-server only.
           @since PCC2 2.40.4 */
        if (!util::parseBooleanValue(value, m_config.usersSeeTemporaryTurns)) {
            throw afl::except::CommandLineException(afl::string::Format("Invalid value for '%s'", key));
        }
        return true;
    } else if (key == "HOST.KICKAFTERMISSED") {
        /* @q Host.KickAfterMissed:Int (Config)
           If nonzero, number of missed turns after which a player is removed from the game.
           @since PCC2 2.40.5 */
        int n;
        if (afl::string::strToInteger(value, n) && n >= 0) {
            m_config.numMissedTurnsForKick = n;
        } else {
            throw afl::except::CommandLineException(afl::string::Format("Invalid value for '%s'", key));
        }
        return true;
    } else if (key == "HOST.WORKDIR") {
        /* @q Host.WorkDir:Str (Config)
           Working directory.
           Temporary files are created below this path.
           @change This option is new in c2host-ng. */
        m_config.workDirectory = value;
        return true;
    } else if (key == "HOST.BACKUPS") {
        /* @q Host.Backups:Str (Config)
           How to deal with backups.
           - keep: (default) just keep the tarballs created by the host scripts
           - unpack: unpack the tarballs. This allows the host filer to make use of deduplication (CA backend)
           c2ng/c2host-server only. */
        if (value == "keep") {
            m_config.unpackBackups = false;
        } else if (value == "unpack") {
            m_config.unpackBackups = true;
        } else {
            throw afl::except::CommandLineException(afl::string::Format("Invalid value for '%s'", key));
        }
        return true;
    } else if (key == "HOST.THREADS") {
        /* @q Host.Threads:Int (Config)
           Ignored in c2ng/c2host-server for compatibility reasons.
           Number of threads (=maximum number of parallel connections). */
        return true;
    } else if (key == "HOST.INITIALSUSPEND") {
        /* @q Host.InitialSuspend:Int (Config)
           Suspend scheduler for the given relative time after startup.
           No games will run until that time has passed.

           The intention is to give users (and the mail fetcher) time to submit turns after a server outage, before running hosts.

           @since PCC2 2.40.6 */
        Time_t n;
        if (afl::string::strToInteger(value, n) && n >= 0) {
            m_config.initialSuspend = n;
        } else {
            throw afl::except::CommandLineException(afl::string::Format("Invalid value for '%s'", key));
        }
        return true;
    } else if (key == "BINDIR") {
        /* @q BinDir:Str (Config)
           Pointer to directory containing binary files.
           Passed to subprocesses as <tt>bindir</tt> in <tt>c2host.ini</tt>.

           @since PCC2 2.40.6 */
        m_config.binDirectory = value;
        return true;
    } else if (key == "HOSTFILE.BASEDIR") {
        // Defined by Hostfile.
        // c2host-classic requires access to basedir; c2host-ng does not.
        // Just ignore it and report it supported.
        return true;
    } else if (key == "REDIS.HOST") {
        m_dbAddress.setName(value);
        return true;
    } else if (key == "REDIS.PORT") {
        m_dbAddress.setService(value);
        return true;
    } else if (key == "HOSTFILE.HOST") {
        m_config.hostFileAddress.setName(value);
        return true;
    } else if (key == "HOSTFILE.PORT") {
        m_config.hostFileAddress.setService(value);
        return true;
    } else if (key == "FILE.HOST") {
        m_userFileAddress.setName(value);
        return true;
    } else if (key == "FILE.PORT") {
        m_userFileAddress.setService(value);
        return true;
    } else if (key == "MAILOUT.HOST") {
        m_mailAddress.setName(value);
        return true;
    } else if (key == "MAILOUT.PORT") {
        m_mailAddress.setService(value);
        return true;
    } else if (key == "TALK.HOST") {
        m_talkAddress.setName(value);
        return true;
    } else if (key == "TALK.PORT") {
        m_talkAddress.setService(value);
        return true;
    } else if (key == "ROUTER.HOST") {
        m_routerAddress.setName(value);
        return true;
    } else if (key == "ROUTER.PORT") {
        m_routerAddress.setService(value);
        return true;
    } else {
        return false;
    }
}

void
server::host::ServerApplication::setupWorkDirectory()
{
    afl::io::FileSystem& fs = fileSystem();

    // If no work directory has been given, determine one
    if (m_config.workDirectory.empty()) {
        // Fetch a sensible base directory name
        String_t base = environment().getEnvironmentVariable("TMP");
        if (base.empty()) {
            base = environment().getEnvironmentVariable("TEMP");
        }
        if (base.empty()) {
            base = "/tmp";
        }

        // Open it
        afl::base::Ref<afl::io::Directory> baseDir = fs.openDirectory(base);

        // Try to create a work directory
        String_t stem = afl::string::Format("c2host%d", afl::sys::Time::getTickCounter());
        String_t candidate = stem;
        int n = 0;
        while (1) {
            try {
                afl::base::Ref<afl::io::DirectoryEntry> entry = baseDir->getDirectoryEntryByName(candidate);
                entry->createAsDirectory();
                m_config.workDirectory = entry->getPathName();
                break;
            }
            catch (...) {
            }
            ++n;
            if (n > 1000) {
                throw afl::except::FileProblemException(fs.makePathName(base, candidate), "Unable to create a working directory");
            }
            candidate = afl::string::Format("%s_%d", stem, n);
        }
    }

    // Validate it
    m_config.workDirectory = fs.getAbsolutePathName(m_config.workDirectory);
    fs.openDirectory(m_config.workDirectory)->getDirectoryEntries();

    log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format("Using work directory %s", m_config.workDirectory));
}

String_t
server::host::ServerApplication::getApplicationName() const
{
    return afl::string::Format("PCC2 Host Server v%s - (c) 2017-2020 Stefan Reuther", PCC2_VERSION);
}

String_t
server::host::ServerApplication::getCommandLineOptionHelp() const
{
    return "--nocron\tDisable scheduler\n";
}
