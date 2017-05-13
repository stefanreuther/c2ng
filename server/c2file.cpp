/**
  *  \file server/c2file.cpp
  *  \brief c2file server application
  */

#include <memory>
#include "afl/except/commandlineexception.hpp"
#include "afl/io/stream.hpp"
#include "afl/net/name.hpp"
#include "afl/net/protocolhandler.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/resp/protocolhandler.hpp"
#include "afl/net/server.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "server/application.hpp"
#include "server/file/commandhandler.hpp"
#include "server/file/directoryhandlerfactory.hpp"
#include "server/file/directoryitem.hpp"
#include "server/file/root.hpp"
#include "server/file/session.hpp"

namespace {
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
        virtual afl::base::Optional<Info> copyFile(DirectoryHandler& source, const Info& sourceInfo, String_t name);
        virtual void readContent(Callback& callback);
        virtual DirectoryHandler* getDirectory(const Info& info);
        virtual Info createDirectory(String_t name);
        virtual void removeDirectory(String_t name);
     private:
        server::file::DirectoryHandler& m_impl;
    };

    /** Session. This aggregates all session state into a ProtocolHandler. */
    class FileSession : public afl::net::ProtocolHandler {
     public:
        FileSession(server::file::Root& root)
            : m_session(),
              m_commandHandler(root, m_session),
              m_protocolHandler(m_commandHandler)
            { }

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
        server::file::Session m_session;
        server::file::CommandHandler m_commandHandler;
        afl::net::resp::ProtocolHandler m_protocolHandler;
    };

    /** ProtocolHandlerFactory for c2talk. */
    class FileProtocolHandlerFactory : public afl::net::ProtocolHandlerFactory {
     public:
        FileProtocolHandlerFactory(server::file::Root& root)
            : m_root(root)
            { }
        virtual FileSession* create()
            { return new FileSession(m_root); }
     private:
        server::file::Root& m_root;
    };

    /** c2talk server application. */
    class FileServerApplication : public server::Application {
     public:
        FileServerApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net)
            : Application(env, fs, net),
              m_listenAddress("127.0.0.1", "9998"),
              m_instanceName("FILE"),
              m_rootDirectory("."),
              m_maxFileSize(10UL*1024*1024)
            { }

        virtual bool handleCommandLineOption(const String_t& option, afl::sys::CommandLineParser& parser)
            {
                if (option == "instance") {
                    // @change was "-I" in PCC2
                    m_instanceName = afl::string::strUCase(parser.getRequiredParameter("I"));
                    return true;
                } else {
                    return false;
                }
            }

        virtual void serverMain()
            {
                // Set up file access
                afl::io::FileSystem& fs = fileSystem();
                server::file::DirectoryHandlerFactory dhFactory(fs, networkStack());
                server::file::DirectoryItem item("(root)", 0, std::auto_ptr<server::file::DirectoryHandler>(new ProxyDirectoryHandler(dhFactory.createDirectoryHandler(m_rootDirectory))));

                afl::base::Ref<afl::io::Directory> defaultSpecDirectory = fs.openDirectory(fs.makePathName(fs.makePathName(environment().getInstallationDirectoryName(), "share"), "specs"));

                // Set up root (global data)
                server::file::Root root(item, defaultSpecDirectory);
                root.log().addListener(log());
                root.setMaxFileSize(m_maxFileSize);

                // Protocol Handler
                FileProtocolHandlerFactory factory(root);

                // Server
                afl::net::Server server(networkStack().listen(m_listenAddress, 10), factory);
                log().write(afl::sys::LogListener::Info, "file", afl::string::Format("Listening on %s", m_listenAddress.toString()));
                server.run();
            }

        virtual bool handleConfiguration(const String_t& key, const String_t& value)
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

     private:
        afl::net::Name m_listenAddress;
        String_t m_instanceName;                     // ex arg_instance
        String_t m_rootDirectory;                    // ex arg_basedir
        afl::io::Stream::FileSize_t m_maxFileSize;   // ex arg_file_size_limit
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
ProxyDirectoryHandler::copyFile(DirectoryHandler& source, const Info& sourceInfo, String_t name)
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

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    afl::net::NetworkStack& net = afl::net::NetworkStack::getInstance();
    FileServerApplication(env, fs, net).run();
}
