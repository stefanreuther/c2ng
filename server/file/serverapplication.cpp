/**
  *  \file server/file/serverapplication.cpp
  *  \brief Class server::file::ServerApplication
  */

#include "server/file/serverapplication.hpp"
#include "afl/except/commandlineexception.hpp"
#include "afl/net/resp/protocolhandler.hpp"
#include "afl/net/server.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/thread.hpp"
#include "server/common/sessionprotocolhandlerfactory.hpp"
#include "server/file/commandhandler.hpp"
#include "server/file/directoryhandler.hpp"
#include "server/file/directoryhandlerfactory.hpp"
#include "server/file/directoryitem.hpp"
#include "server/file/root.hpp"
#include "server/file/session.hpp"
#include "server/ports.hpp"
#include "version.hpp"
#include "util/string.hpp"

using afl::async::InterruptOperation;

namespace {
    const char LOG_NAME[] = "file";

    /** Proxy DirectoryHandler.
        DirectoryHandler's created by DirectoryHandlerFactory are owned by that,
        but DirectoryItem wants a DirectoryHandler it owns, so we need to proxy that.
        Unfortunately this class needs be implemented verbosely out-of-line;
        otherwise, gcc-4.9 bloats it when optimizing. */
    class ProxyDirectoryHandler : public server::file::DirectoryHandler {
     public:
        ProxyDirectoryHandler(server::file::DirectoryHandler& impl)
            : m_impl(impl)
            { }
        virtual String_t getName();
        virtual afl::base::Ref<afl::io::FileMapping> getFile(const Info& info);
        virtual afl::base::Ref<afl::io::FileMapping> getFileByName(String_t name);
        virtual Info createFile(String_t name, afl::base::ConstBytes_t content);
        virtual void removeFile(String_t name);
        virtual afl::base::Optional<Info> copyFile(ReadOnlyDirectoryHandler& source, const Info& sourceInfo, String_t name);
        virtual void readContent(Callback& callback);
        virtual DirectoryHandler* getDirectory(const Info& info);
        virtual Info createDirectory(String_t name);
        virtual void removeDirectory(String_t name);
     private:
        server::file::DirectoryHandler& m_impl;
    };
}

String_t
ProxyDirectoryHandler::getName()
{
    return m_impl.getName();
}

afl::base::Ref<afl::io::FileMapping>
ProxyDirectoryHandler::getFile(const Info& info)
{
    return m_impl.getFile(info);
}

afl::base::Ref<afl::io::FileMapping>
ProxyDirectoryHandler::getFileByName(String_t name)
{
    return m_impl.getFileByName(name);
}

server::file::DirectoryHandler::Info
ProxyDirectoryHandler::createFile(String_t name, afl::base::ConstBytes_t content)
{
    return m_impl.createFile(name, content);
}

void
ProxyDirectoryHandler::removeFile(String_t name)
{
    m_impl.removeFile(name);
}

afl::base::Optional<server::file::DirectoryHandler::Info>
ProxyDirectoryHandler::copyFile(ReadOnlyDirectoryHandler& source, const Info& sourceInfo, String_t name)
{
    return m_impl.copyFile(source, sourceInfo, name);
}

void
ProxyDirectoryHandler::readContent(Callback& callback)
{
    m_impl.readContent(callback);
}

server::file:: DirectoryHandler*
ProxyDirectoryHandler::getDirectory(const Info& info)
{
    return m_impl.getDirectory(info);
}

server::file::DirectoryHandler::Info
ProxyDirectoryHandler::createDirectory(String_t name)
{
    return m_impl.createDirectory(name);
}

void
ProxyDirectoryHandler::removeDirectory(String_t name)
{
    m_impl.removeDirectory(name);
}

/************************* ServerApplication *************************/

server::file::ServerApplication::ServerApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net, afl::async::Interrupt& intr)
    : Application(LOG_NAME, env, fs, net),
      m_listenAddress(DEFAULT_ADDRESS, FILE_PORT),
      m_instanceName("FILE"),
      m_rootDirectory("."),
      m_maxFileSize(10UL*1024*1024),
      m_interrupt(intr),
      m_gcEnabled(true)
{ }

server::file::ServerApplication::~ServerApplication()
{ }

bool
server::file::ServerApplication::handleCommandLineOption(const String_t& option, afl::sys::CommandLineParser& parser)
{
    if (option == "instance") {
        // @change was "-I" in PCC2
        m_instanceName = afl::string::strUCase(parser.getRequiredParameter("instance"));
        return true;
    } else if (option == "nogc") {
        m_gcEnabled = false;
        return true;
    } else {
        return false;
    }
}

void
server::file::ServerApplication::serverMain()
{
    // Set up file access
    afl::io::FileSystem& fs = fileSystem();
    DirectoryHandlerFactory dhFactory(fs, networkStack());
    dhFactory.setGarbageCollection(m_gcEnabled);
    DirectoryItem item("(root)", 0, std::auto_ptr<DirectoryHandler>(new ProxyDirectoryHandler(dhFactory.createDirectoryHandler(m_rootDirectory, log()))));

    afl::base::Ref<afl::io::Directory> defaultSpecDirectory = fs.openDirectory(fs.makePathName(fs.makePathName(environment().getInstallationDirectoryName(), "share"), "specs"));

    // Set up root (global data)
    Root root(item, defaultSpecDirectory);
    root.log().addListener(log());
    root.setMaxFileSize(m_maxFileSize);

    // Protocol Handler
    server::common::SessionProtocolHandlerFactory<Root, Session, afl::net::resp::ProtocolHandler, CommandHandler> factory(root);

    // Server
    afl::net::Server server(networkStack().listen(m_listenAddress, 10), factory);
    log().write(afl::sys::LogListener::Info, "file", afl::string::Format("Listening on %s", m_listenAddress.toString()));

    // Server thread
    afl::sys::Thread serverThread("file.server", server);
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
server::file::ServerApplication::handleConfiguration(const String_t& key, const String_t& value)
{
    // ex planetscentral/file.cc:processConfig
    if (key == m_instanceName + ".HOST") {
        /* @q File.Host:Str (Config), HostFile.Host:Str (Config)
           Listen address for the File/HostFile instance. */
        m_listenAddress.setName(value);
        return true;
    } else if (key == m_instanceName + ".PORT") {
        /* @q File.Port:Int (Config), HostFile.Port:Int (Config)
           Port number for the File/HostFile instance. */
        m_listenAddress.setService(value);
        return true;
    } else if (key == m_instanceName + ".BASEDIR") {
        /* @q File.BaseDir:Str (Config), HostFile.BaseDir:Str (Config)
           Base directory where managed files are.
           Syntax:
           - "PATH": manage a plain directory
           - "[PATH@]ca:SPEC": manage a content-adressable object pool (i.e. a git repository) inside the file space defined by SPEC
           - "int:": operate internally (in RAM); mainly for testing use
           - "c2file://[USER@]HOST:PORT/PATH": talk to another c2file instance (experimental/unsupported) */
        m_rootDirectory = value;
        return true;
    } else if (key == m_instanceName + ".SIZELIMIT") {
        /* @q File.SizeLimit:Int (Config), HostFile.SizeLimit:Int (Config)
           Maximum size of a file in this instance. */
        if (!afl::string::strToInteger(value, m_maxFileSize)) {
            throw afl::except::CommandLineException(afl::string::Format("Invalid number for '%s'", key));
        }
        return true;
    } else if (key == m_instanceName + ".THREADS") {
        /* @q File.Threads:Int (Config), HostFile.Threads:Int (Config)
           Ignored in c2file-ng for compatibility reasons.
           Number of threads (=maximum number of parallel connections) */
        return true;
    } else {
        return false;
    }
}

String_t
server::file::ServerApplication::getApplicationName() const
{
    return afl::string::Format("PCC2 File Server v%s - (c) 2017-2023 Stefan Reuther", PCC2_VERSION);
}

String_t
server::file::ServerApplication::getCommandLineOptionHelp() const
{
    return "--instance=NAME\tInstance name (default: \"FILE\")\n"
        "--nogc\tDisable garbage collection\n";
}

