/**
  *  \file u/t_server_file_directoryhandlerfactory.cpp
  *  \brief Test for server::file::DirectoryHandlerFactory
  */

#include "server/file/directoryhandlerfactory.hpp"

#include <memory>
#include "t_server_file.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/name.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/net/nullnetworkstack.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/resp/protocolhandler.hpp"
#include "afl/net/server.hpp"
#include "afl/sys/thread.hpp"
#include "afl/test/commandhandler.hpp"
#include "server/file/directoryhandler.hpp"

/** Test makePathName. */
void
TestServerFileDirectoryHandlerFactory::testPathName()
{
    using server::file::DirectoryHandlerFactory;

    TS_ASSERT_EQUALS(DirectoryHandlerFactory::makePathName("/a/b/c", "d"), "d@/a/b/c");
    TS_ASSERT_EQUALS(DirectoryHandlerFactory::makePathName("ca:x", "d"), "d@ca:x");
    TS_ASSERT_EQUALS(DirectoryHandlerFactory::makePathName("a/b@ca:x", "d"), "a/b/d@ca:x");
    TS_ASSERT_EQUALS(DirectoryHandlerFactory::makePathName("a/b@g/h", "e"), "a/b/e@g/h");
}

/** Test createDirectoryHandler. */
void
TestServerFileDirectoryHandlerFactory::testCreate()
{
    afl::io::NullFileSystem fs;
    afl::net::NullNetworkStack net;
    server::file::DirectoryHandlerFactory testee(fs, net);

    // Create two file system elements.
    // Result must be identical.
    // The FileSystemHandler does not access the file system on creation, so using a NullFileSystem is ok.
    server::file::DirectoryHandler& a1 = testee.createDirectoryHandler("a");
    server::file::DirectoryHandler& a2 = testee.createDirectoryHandler("a");
    server::file::DirectoryHandler& b  = testee.createDirectoryHandler("b");
    TS_ASSERT_EQUALS(&a1, &a2);
    TS_ASSERT_DIFFERS(&a1, &b);
}

/** Test createDirectoryHandler for internal. */
void
TestServerFileDirectoryHandlerFactory::testCreateInternal()
{
    afl::io::NullFileSystem fs;
    afl::net::NullNetworkStack net;
    server::file::DirectoryHandlerFactory testee(fs, net);

    // Create with different uniquifier
    server::file::DirectoryHandler& a1 = testee.createDirectoryHandler("int:");
    server::file::DirectoryHandler& a2 = testee.createDirectoryHandler("int:");
    server::file::DirectoryHandler& b  = testee.createDirectoryHandler("int:uniq");
    TS_ASSERT_EQUALS(&a1, &a2);
    TS_ASSERT_DIFFERS(&a1, &b);

    // Must be able to access them
    static const uint8_t DATA[] = {'h','e','l','l','o'};
    a1.createFile("f", DATA);
    TS_ASSERT(a2.getFileByName("f")->get().equalContent(DATA));
    TS_ASSERT_THROWS(b.getFileByName("f"), afl::except::FileProblemException);
}

/** Test createDirectoryHandler for CA backend. */
void
TestServerFileDirectoryHandlerFactory::testCreateCA()
{
    afl::io::NullFileSystem fs;
    afl::net::NullNetworkStack net;
    server::file::DirectoryHandlerFactory testee(fs, net);

    // Create it
    server::file::DirectoryHandler& c = testee.createDirectoryHandler("ca:int:");

    // Create a file
    c.createFile("f", afl::string::toBytes("zz"));

    // Verify content
    server::file::DirectoryHandler& raw = testee.createDirectoryHandler("int:");
    TS_ASSERT(raw.getFileByName("HEAD")->get().equalContent(afl::string::toBytes("ref: refs/heads/master\n")));
}

/** Test createDirectoryHandler for subdirectories. */
void
TestServerFileDirectoryHandlerFactory::testCreateSubdir()
{
    using server::file::DirectoryHandler;

    afl::io::NullFileSystem fs;
    afl::net::NullNetworkStack net;
    server::file::DirectoryHandlerFactory testee(fs, net);

    // Create an internal source
    DirectoryHandler& root = testee.createDirectoryHandler("int:");
    std::auto_ptr<DirectoryHandler> a(root.getDirectory(root.createDirectory("a")));
    std::auto_ptr<DirectoryHandler> b(a->getDirectory(a->createDirectory("b")));
    std::auto_ptr<DirectoryHandler> c(b->getDirectory(b->createDirectory("c")));
    c->createFile("f", afl::string::toBytes("zz"));

    // Create an internal source inside a path
    DirectoryHandler& sub = testee.createDirectoryHandler("a/b@int:");
    DirectoryHandler::Info it;
    TS_ASSERT(sub.findItem("c", it));
    TS_ASSERT_EQUALS(it.type, DirectoryHandler::IsDirectory);
    std::auto_ptr<DirectoryHandler> c1(sub.getDirectory(it));
    TS_ASSERT(c1->getFileByName("f")->get().equalContent(afl::string::toBytes("zz")));
}

/** Try invalid elements. */
void
TestServerFileDirectoryHandlerFactory::testCreateErrors()
{
   afl::io::NullFileSystem fs;
   afl::net::NullNetworkStack net;
   server::file::DirectoryHandlerFactory testee(fs, net);

   // The internal source is initially empty, so creating subpaths does not work
   TS_ASSERT_THROWS(testee.createDirectoryHandler("a@int:"), afl::except::FileProblemException);
   TS_ASSERT_THROWS(testee.createDirectoryHandler("a/b@int:"), afl::except::FileProblemException);

   // Invalid URL
   TS_ASSERT_THROWS(testee.createDirectoryHandler("c2file://@invalid"), afl::except::FileProblemException);
   TS_ASSERT_THROWS(testee.createDirectoryHandler("c2file://127.0.0.1:/"), afl::except::FileProblemException);

   // Nonexistant subdirectory
   TS_ASSERT_THROWS(testee.createDirectoryHandler("a/b@c/d"), afl::except::FileProblemException);
}

/** Test creation of remote elements. */
void
TestServerFileDirectoryHandlerFactory::testCreateRemote()
{
    class ServerStuff : private afl::net::ProtocolHandlerFactory {
     public:
        ServerStuff(afl::net::CommandHandler& handler, afl::net::NetworkStack& stack, afl::net::Name listenAddress)
            : m_handler(handler),
              m_server(stack.listen(listenAddress, 10), *this),
              m_serverThread("testCreateRemote", m_server)
            {
                m_serverThread.start();
            }

        ~ServerStuff()
            {
                m_server.stop();
                m_serverThread.join();
            }

     private:
        // ProtocolHandlerFactory:
        virtual afl::net::ProtocolHandler* create()
            { return new afl::net::resp::ProtocolHandler(m_handler); }

        afl::net::CommandHandler& m_handler;
        afl::net::Server m_server;
        afl::sys::Thread m_serverThread;
    };

    // Set up a server
    const int16_t PORT_NR = 25289;
    afl::net::NetworkStack& stack = afl::net::NetworkStack::getInstance();
    afl::test::CommandHandler mock("testCreateRemote");
    ServerStuff stuff(mock, stack, afl::net::Name("127.0.0.1", PORT_NR));

    // Set up test infrastructure
    using server::file::DirectoryHandler;

    afl::io::NullFileSystem fs;
    server::file::DirectoryHandlerFactory testee(fs, stack);

    // Create two instances. Should be unified due to caching (but we get two user-logons).
    mock.expectCall("USER, 1022");
    mock.provideNewResult(0);
    DirectoryHandler& a = testee.createDirectoryHandler("c2file://1022@127.0.0.1:25289/");

    mock.expectCall("USER, 1022");
    mock.provideNewResult(0);
    DirectoryHandler& b = testee.createDirectoryHandler("c2file://1022@127.0.0.1:25289/b");

    // Create a file in a
    mock.expectCall("PUT, z, cc");
    mock.provideNewResult(0);
    a.createFile("z", afl::string::toBytes("cc"));

    // Create a file in b
    mock.expectCall("PUT, b/f, cc");
    mock.provideNewResult(0);
    b.createFile("f", afl::string::toBytes("cc"));

    // Copy a to b
    mock.expectCall("CP, src, b/dst");
    mock.provideNewResult(0);
    TS_ASSERT(b.copyFile(a, DirectoryHandler::Info("src", DirectoryHandler::IsFile), "dst").isValid());

    mock.checkFinish();
}

