/**
  *  \file server/doc/serverapplication.cpp
  *  \brief Class server::doc::ServerApplication
  */

#include "server/doc/serverapplication.hpp"
#include "afl/async/controller.hpp"
#include "afl/net/protocolhandler.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/resp/protocolhandler.hpp"
#include "afl/net/server.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/thread.hpp"
#include "server/doc/documentationimpl.hpp"
#include "server/doc/root.hpp"
#include "server/interface/documentationserver.hpp"
#include "server/ports.hpp"
#include "util/doc/blobstore.hpp"
#include "util/doc/fileblobstore.hpp"
#include "util/doc/index.hpp"
#include "util/doc/singleblobstore.hpp"
#include "version.hpp"

using afl::async::InterruptOperation;
using afl::base::Ptr;
using afl::base::Ref;
using afl::io::Directory;
using afl::io::FileSystem;
using afl::io::Stream;
using afl::string::Format;
using afl::sys::LogListener;
using util::doc::BlobStore;
using util::doc::FileBlobStore;
using util::doc::SingleBlobStore;

namespace {
    const char LOG_NAME[] = "doc";

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

server::doc::ServerApplication::ServerApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net, afl::async::Interrupt& intr)
    : Application(LOG_NAME, "DOC", env, fs, net),
      m_listenAddress(DEFAULT_ADDRESS, DOC_PORT),
      m_directoryName(),
      m_interrupt(intr)
{ }

server::doc::ServerApplication::~ServerApplication()
{ }

bool
server::doc::ServerApplication::handleCommandLineOption(const String_t& /*option*/, afl::sys::CommandLineParser& /*parser*/)
{
    return false;
}

void
server::doc::ServerApplication::serverMain()
{
    // Open directory
    Ref<Directory> dir = fileSystem().openDirectory(m_directoryName.empty()
                                                    ? fileSystem().getWorkingDirectoryName()
                                                    : m_directoryName);

    // Detect content type: prefer tar if present
    Ptr<Stream> file = dir->openFileNT("content.tar", FileSystem::OpenRead);
    std::auto_ptr<BlobStore> blobStore;
    if (file.get() != 0) {
        log().write(LogListener::Info, LOG_NAME, "Using single-file mode.");
        blobStore.reset(new SingleBlobStore(*file));
    } else {
        log().write(LogListener::Info, LOG_NAME, "Using directory mode.");
        blobStore.reset(new FileBlobStore(dir->openDirectory("content")));
    }

    // Set up root (global data)
    Root root(*blobStore);
    root.index().load(*dir->openFile("index.xml", FileSystem::OpenRead));

    // Command handler
    DocumentationImpl impl(root);
    server::interface::DocumentationServer cmdHandler(impl);
    ProtocolHandlerFactory factory(cmdHandler);

    // Server
    afl::net::Server server(networkStack().listen(m_listenAddress, 10), factory);
    log().write(LogListener::Info, LOG_NAME, Format("Listening on %s", m_listenAddress.toString()));

    // Server thread
    afl::sys::Thread serverThread("talk.server", server);
    serverThread.start();

    // Wait for termination request
    afl::async::Controller ctl;
    m_interrupt.wait(ctl, InterruptOperation::Kinds_t() + InterruptOperation::Break + InterruptOperation::Terminate);

    // Stop
    log().write(LogListener::Info, LOG_NAME, "Received stop signal, shutting down.");
    server.stop();
    serverThread.join();
}

bool
server::doc::ServerApplication::handleConfiguration(const String_t& key, const String_t& value)
{
    if (isInstanceOption(key, "HOST")) {
        /* @q Doc.Host:Str (Config)
           Listen address
           @since PCC2 2.40.12 */
        m_listenAddress.setName(value);
        return true;
    } else if (isInstanceOption(key, "PORT")) {
        /* @q Doc.Port:Int (Config)
           Port number.
           @since PCC2 2.40.12 */
        m_listenAddress.setService(value);
        return true;
    } else if (isInstanceOption(key, "DIR")) {
        /* @q Doc.Dir:Str (Config)
           Directory name of documentation repository.
           Directory must contain a "index.xml" file and a "content/" directory or a "content.tar" file.
           @since PCC2 2.40.12 */
        m_directoryName = value;
        return true;
    } else {
        return false;
    }
}

String_t
server::doc::ServerApplication::getApplicationName() const
{
    return Format("PCC2 Documentation Server v%s - (c) 2021-2025 Stefan Reuther", PCC2_VERSION);
}

String_t
server::doc::ServerApplication::getCommandLineOptionHelp() const
{
    return String_t();
}
