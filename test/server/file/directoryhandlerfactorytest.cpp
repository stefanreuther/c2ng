/**
  *  \file test/server/file/directoryhandlerfactorytest.cpp
  *  \brief Test for server::file::DirectoryHandlerFactory
  */

#include "server/file/directoryhandlerfactory.hpp"

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
#include "afl/test/testrunner.hpp"
#include "server/file/ca/root.hpp"
#include "server/file/directoryhandler.hpp"
#include <memory>

using server::file::DirectoryHandlerFactory;

namespace {
    enum Except { AllFiles, ExceptMaster, ExceptCommit, ExceptTree, ExceptBlob };

    /* Preload CA content. Allows skipping one file to create a broken tree. */
    void preloadCA(DirectoryHandlerFactory& dhf, int except)
    {
        afl::sys::Log log;
        server::file::DirectoryHandler& raw = dhf.createDirectoryHandler("int:", log);

        if (except != ExceptMaster) {
            std::auto_ptr<server::file::DirectoryHandler> refs(raw.getDirectory(raw.createDirectory("refs")));
            std::auto_ptr<server::file::DirectoryHandler> heads(refs->getDirectory(refs->createDirectory("heads")));
            heads->createFile("master", afl::string::toBytes("d736f50b5d7d74ebbaac88a9bed28f1748602d1b\n"));
        }

        static const uint8_t ca_objects_8e_27be7d6154a1f68ea9160ef0e18691d20560dc[] = {
            0x78, 0x9c, 0x4b, 0xca, 0xc9, 0x4f, 0x52, 0x30, 0x65, 0x28, 0x49, 0xad,
            0x28, 0xe1, 0x02, 0x00, 0x19, 0xb5, 0x03, 0xc4
        };
        static const uint8_t ca_objects_ba_3da7bf7f67e392f3994a087eec14200179dd58[] = {
            0x78, 0x9c, 0x2b, 0x29, 0x4a, 0x4d, 0x55, 0x30, 0xb2, 0x64, 0x30, 0x34,
            0x30, 0x30, 0x33, 0x31, 0x51, 0x48, 0x63, 0xe8, 0x53, 0xdf, 0x57, 0x9b,
            0x18, 0xb2, 0xf0, 0x5b, 0xdf, 0x4a, 0x31, 0xbe, 0x0f, 0x0f, 0xdb, 0x26,
            0x5e, 0x62, 0x4d, 0xb8, 0x03, 0x00, 0xe4, 0x28, 0x0e, 0x83
        };
        static const uint8_t ca_objects_d7_36f50b5d7d74ebbaac88a9bed28f1748602d1b[] = {
            0x78, 0x9c, 0x6d, 0x8c, 0xcb, 0x0a, 0x80, 0x20, 0x10, 0x45, 0x5b, 0xfb,
            0x15, 0xb3, 0x6f, 0xe3, 0xa8, 0x35, 0x0a, 0xd1, 0xbf, 0xf8, 0x18, 0x49,
            0x48, 0x84, 0xb0, 0xff, 0x2f, 0xaa, 0x65, 0x77, 0x79, 0xce, 0xe1, 0xc6,
            0x56, 0x6b, 0xe9, 0x80, 0x68, 0x86, 0x7e, 0x30, 0x43, 0xf0, 0x3a, 0x79,
            0x0a, 0x99, 0xf2, 0x4c, 0xac, 0x9d, 0xca, 0xda, 0x39, 0xe3, 0xa5, 0x25,
            0xe6, 0x88, 0x46, 0x49, 0x89, 0xe4, 0x52, 0x9a, 0xac, 0xf0, 0x67, 0xdf,
            0xda, 0x01, 0x51, 0xe5, 0xb2, 0x33, 0x2c, 0x2b, 0x20, 0x8c, 0xf2, 0x9e,
            0x88, 0xcf, 0x63, 0xe7, 0x3f, 0x27, 0x3e, 0xf4, 0x36, 0xe2, 0x02, 0xff,
            0xe8, 0x24, 0x1d
        };

        std::auto_ptr<server::file::DirectoryHandler> objects(raw.getDirectory(raw.createDirectory("objects")));
        if (except != ExceptBlob) {
            std::auto_ptr<server::file::DirectoryHandler> obj_8e(objects->getDirectory(objects->createDirectory("8e")));
            obj_8e->createFile("27be7d6154a1f68ea9160ef0e18691d20560dc", ca_objects_8e_27be7d6154a1f68ea9160ef0e18691d20560dc);
        }

        if (except != ExceptTree) {
            std::auto_ptr<server::file::DirectoryHandler> obj_ba(objects->getDirectory(objects->createDirectory("ba")));
            obj_ba->createFile("3da7bf7f67e392f3994a087eec14200179dd58", ca_objects_ba_3da7bf7f67e392f3994a087eec14200179dd58);
        }

        if (except != ExceptCommit) {
            std::auto_ptr<server::file::DirectoryHandler> obj_d7(objects->getDirectory(objects->createDirectory("d7")));
            obj_d7->createFile("36f50b5d7d74ebbaac88a9bed28f1748602d1b", ca_objects_d7_36f50b5d7d74ebbaac88a9bed28f1748602d1b);
        }
    }
}

/** Test makePathName. */
AFL_TEST("server.file.DirectoryHandlerFactory:makePathName", a)
{
    a.checkEqual("01", DirectoryHandlerFactory::makePathName("/a/b/c", "d"), "d@/a/b/c");
    a.checkEqual("02", DirectoryHandlerFactory::makePathName("ca:x", "d"), "d@ca:x");
    a.checkEqual("03", DirectoryHandlerFactory::makePathName("a/b@ca:x", "d"), "a/b/d@ca:x");
    a.checkEqual("04", DirectoryHandlerFactory::makePathName("a/b@g/h", "e"), "a/b/e@g/h");
    a.checkEqual("05", DirectoryHandlerFactory::makePathName("c2file://a@b:c/d", "e"), "c2file://a@b:c/d/e");
}

/** Test createDirectoryHandler. */
AFL_TEST("server.file.DirectoryHandlerFactory:createDirectoryHandler", a)
{
    afl::io::NullFileSystem fs;
    afl::net::NullNetworkStack net;
    afl::sys::Log log;
    DirectoryHandlerFactory testee(fs, net);

    // Create two file system elements.
    // Result must be identical.
    // The FileSystemHandler does not access the file system on creation, so using a NullFileSystem is ok.
    server::file::DirectoryHandler& a1 = testee.createDirectoryHandler("a", log);
    server::file::DirectoryHandler& a2 = testee.createDirectoryHandler("a", log);
    server::file::DirectoryHandler& b  = testee.createDirectoryHandler("b", log);
    a.checkEqual("01", &a1, &a2);
    a.checkDifferent("02", &a1, &b);
}

/** Test createDirectoryHandler for internal. */
AFL_TEST("server.file.DirectoryHandlerFactory:createDirectoryHandler:int", a)
{
    afl::io::NullFileSystem fs;
    afl::net::NullNetworkStack net;
    afl::sys::Log log;
    DirectoryHandlerFactory testee(fs, net);

    // Create with different uniquifier
    server::file::DirectoryHandler& a1 = testee.createDirectoryHandler("int:", log);
    server::file::DirectoryHandler& a2 = testee.createDirectoryHandler("int:", log);
    server::file::DirectoryHandler& b  = testee.createDirectoryHandler("int:uniq", log);
    a.checkEqual("01", &a1, &a2);
    a.checkDifferent("02", &a1, &b);

    // Must be able to access them
    static const uint8_t DATA[] = {'h','e','l','l','o'};
    a1.createFile("f", DATA);
    a.check("11", a2.getFileByName("f")->get().equalContent(DATA));
    AFL_CHECK_THROWS(a("12. file does not exist in other"), b.getFileByName("f"), afl::except::FileProblemException);
}

/** Test createDirectoryHandler for CA backend. */
AFL_TEST("server.file.DirectoryHandlerFactory:createDirectoryHandler:ca:int", a)
{
    afl::io::NullFileSystem fs;
    afl::net::NullNetworkStack net;
    afl::sys::Log log;
    DirectoryHandlerFactory testee(fs, net);

    // Create it
    server::file::DirectoryHandler& c = testee.createDirectoryHandler("ca:int:", log);

    // Create a file
    c.createFile("f", afl::string::toBytes("zz"));

    // Verify content
    server::file::DirectoryHandler& raw = testee.createDirectoryHandler("int:", log);
    a.check("01", raw.getFileByName("HEAD")->get().equalContent(afl::string::toBytes("ref: refs/heads/master\n")));
}

/** Test createDirectoryHandler for subdirectories. */
AFL_TEST("server.file.DirectoryHandlerFactory:createDirectoryHandler:int:subdir", a)
{
    using server::file::DirectoryHandler;

    afl::io::NullFileSystem fs;
    afl::net::NullNetworkStack net;
    afl::sys::Log log;
    DirectoryHandlerFactory testee(fs, net);

    // Create an internal source
    DirectoryHandler& root = testee.createDirectoryHandler("int:", log);
    std::auto_ptr<DirectoryHandler> da(root.getDirectory(root.createDirectory("a")));
    std::auto_ptr<DirectoryHandler> db(da->getDirectory(da->createDirectory("b")));
    std::auto_ptr<DirectoryHandler> dc(db->getDirectory(db->createDirectory("c")));
    dc->createFile("f", afl::string::toBytes("zz"));

    // Create an internal source inside a path
    DirectoryHandler& sub = testee.createDirectoryHandler("a/b@int:", log);
    DirectoryHandler::Info it;
    a.check("01", sub.findItem("c", it));
    a.checkEqual("02. type", it.type, DirectoryHandler::IsDirectory);
    std::auto_ptr<DirectoryHandler> c1(sub.getDirectory(it));
    a.check("03", c1->getFileByName("f")->get().equalContent(afl::string::toBytes("zz")));
}

/** Try invalid elements. */
AFL_TEST("server.file.DirectoryHandlerFactory:createDirectoryHandler:invalid", a)
{
   afl::io::NullFileSystem fs;
   afl::net::NullNetworkStack net;
   afl::sys::Log log;
   DirectoryHandlerFactory testee(fs, net);

   // The internal source is initially empty, so creating subpaths does not work
   AFL_CHECK_THROWS(a("01. int"), testee.createDirectoryHandler("a@int:", log), afl::except::FileProblemException);
   AFL_CHECK_THROWS(a("02. int"), testee.createDirectoryHandler("a/b@int:", log), afl::except::FileProblemException);

   // Invalid URL
   AFL_CHECK_THROWS(a("11. c2file"), testee.createDirectoryHandler("c2file://@invalid", log), afl::except::FileProblemException);
   AFL_CHECK_THROWS(a("12. c2file"), testee.createDirectoryHandler("c2file://127.0.0.1:/", log), afl::except::FileProblemException);

   // Nonexistant subdirectory
   AFL_CHECK_THROWS(a("21. subdir"), testee.createDirectoryHandler("a/b@c/d", log), afl::except::FileProblemException);
}

/** Test creation of remote elements. */
AFL_TEST("server.file.DirectoryHandlerFactory:createDirectoryHandler:c2file", a)
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
    afl::test::CommandHandler mock(a);
    afl::sys::Log log;
    ServerStuff stuff(mock, stack, afl::net::Name("127.0.0.1", PORT_NR));

    // Set up test infrastructure
    using server::file::DirectoryHandler;

    afl::io::NullFileSystem fs;
    DirectoryHandlerFactory testee(fs, stack);

    // Create two instances. Should be unified due to caching (but we get two user-logons).
    mock.expectCall("USER, 1022");
    mock.provideNewResult(0);
    DirectoryHandler& da = testee.createDirectoryHandler("c2file://1022@127.0.0.1:25289/", log);

    mock.expectCall("USER, 1022");
    mock.provideNewResult(0);
    DirectoryHandler& db = testee.createDirectoryHandler("c2file://1022@127.0.0.1:25289/b", log);

    // Create a file in a
    mock.expectCall("PUT, z, cc");
    mock.provideNewResult(0);
    da.createFile("z", afl::string::toBytes("cc"));

    // Create a file in b
    mock.expectCall("PUT, b/f, cc");
    mock.provideNewResult(0);
    db.createFile("f", afl::string::toBytes("cc"));

    // Copy a to b
    mock.expectCall("CP, src, b/dst");
    mock.provideNewResult(0);
    a.check("01", db.copyFile(da, DirectoryHandler::Info("src", DirectoryHandler::IsFile), "dst").isValid());

    mock.checkFinish();
}

/** Test a preloaded CA tree. */
AFL_TEST("server.file.DirectoryHandlerFactory:createDirectoryHandler:ca:preloaded", a)
{
    afl::io::NullFileSystem fs;
    afl::net::NullNetworkStack net;
    afl::sys::Log log;
    DirectoryHandlerFactory testee(fs, net);
    testee.setGarbageCollection(true);

    // Create content in CA format
    preloadCA(testee, AllFiles);

    // Create it
    server::file::DirectoryHandler& c = testee.createDirectoryHandler("ca:int:", log);

    // Verify content
    afl::base::ConstBytes_t b;
    a.check("01", c.getFileByName("f")->get().equalContent(afl::string::toBytes("text\n")));
}

/** Test an erroneous preloaded CA tree, missing commit.
    A: create a CA tree that is missing the root commit object. Create CA DirectoryHandler with GC enabled.
    E: creation fails due to GC error */
AFL_TEST("server.file.DirectoryHandlerFactory:createDirectoryHandler:ca:error:missing-commit", a)
{
    afl::io::NullFileSystem fs;
    afl::net::NullNetworkStack net;
    afl::sys::Log log;
    DirectoryHandlerFactory testee(fs, net);
    testee.setGarbageCollection(true);

    // Create content in CA format
    preloadCA(testee, ExceptCommit);

    // Create it
    AFL_CHECK_THROWS(a, testee.createDirectoryHandler("ca:int:", log), afl::except::FileProblemException);
}

/** Test an erroneous preloaded CA tree, missing tree.
    A: create a CA tree that is missing the root tree object. Create CA DirectoryHandler with GC enabled.
    E: creation fails due to GC error */
AFL_TEST("server.file.DirectoryHandlerFactory:createDirectoryHandler:ca:error:missing-tree", a)
{
    afl::io::NullFileSystem fs;
    afl::net::NullNetworkStack net;
    afl::sys::Log log;
    DirectoryHandlerFactory testee(fs, net);
    testee.setGarbageCollection(true);

    // Create content in CA format
    preloadCA(testee, ExceptTree);

    // Create it
    AFL_CHECK_THROWS(a, testee.createDirectoryHandler("ca:int:", log), afl::except::FileProblemException);
}

/** Test an erroneous preloaded CA tree, missing blob, no GC.
    A: create a CA tree that is missing a blob object. Create CA DirectoryHandler with GC enabled.
    E: creation succeeds, but access of the blob fails */
AFL_TEST("server.file.DirectoryHandlerFactory:createDirectoryHandler:ca:error:missing-blob", a)
{
    afl::io::NullFileSystem fs;
    afl::net::NullNetworkStack net;
    afl::sys::Log log;
    DirectoryHandlerFactory testee(fs, net);
    testee.setGarbageCollection(false);

    // Create content in CA format, lacking the Blob object
    preloadCA(testee, ExceptBlob);

    // Create it
    // We have garbage collection disabled, so it's unspecified when an error is detected.
    // As of 20200220,
    // - ExceptCommit will fail the construction
    // - ExceptTree will fail the file access
    // but this is not contractual.
    server::file::DirectoryHandler& c = testee.createDirectoryHandler("ca:int:", log);
    AFL_CHECK_THROWS(a, c.getFileByName("f"), afl::except::FileProblemException);
}

/** Test access to CA snapshot.
    A: create a CA tree with snapshot. Access using "ca:" and "snapshot:" URLs.
    E: Snapshot has correct content and is not writable */
AFL_TEST("server.file.DirectoryHandlerFactory:createDirectoryHandler:snapshot", a)
{
    // Create and preload a file system
    afl::io::NullFileSystem fs;
    afl::net::NullNetworkStack net;
    afl::sys::Log log;
    DirectoryHandlerFactory testee(fs, net);
    testee.setGarbageCollection(false);

    // - Root
    server::file::ca::Root root(testee.createDirectoryHandler("int:", log), log);
    std::auto_ptr<server::file::DirectoryHandler> rootDir(root.createRootHandler());

    // - Store one file, take snapshot
    rootDir->createFile("f", afl::string::toBytes("original content"));
    root.setSnapshotCommitId("s", root.getMasterCommitId());

    // - Update file
    rootDir->createFile("f", afl::string::toBytes("new content"));

    // Verify read access through DirectoryHandlerFactory
    server::file::DirectoryHandler& liveHandler = testee.createDirectoryHandler("ca:int:", log);
    a.checkEqual("live read access", afl::string::fromBytes(liveHandler.getFileByName("f")->get()), "new content");

    server::file::DirectoryHandler& snapHandler = testee.createDirectoryHandler("snapshot:s:int:", log);
    a.checkEqual("snapshot read access", afl::string::fromBytes(snapHandler.getFileByName("f")->get()), "original content");

    // Verify write access
    AFL_CHECK_SUCCEEDS(a("live write access"), liveHandler.createFile("n", afl::string::toBytes("data...")));
    AFL_CHECK_THROWS(a("snapshot write access"), snapHandler.createFile("n", afl::string::toBytes("data...")), afl::except::FileProblemException);
}

/** Test access to CA snapshot, error case.
    A: create a CA tree. Try to access with "snapshot:" URL for missing snapshot.
    E: Creation is rejected */
AFL_TEST("server.file.DirectoryHandlerFactory:createDirectoryHandler:snapshot:error", a)
{
    // Create and preload a file system
    afl::io::NullFileSystem fs;
    afl::net::NullNetworkStack net;
    afl::sys::Log log;
    DirectoryHandlerFactory testee(fs, net);
    testee.setGarbageCollection(false);

    server::file::ca::Root root(testee.createDirectoryHandler("int:", log), log);
    std::auto_ptr<server::file::DirectoryHandler> rootDir(root.createRootHandler());
    rootDir->createFile("f", afl::string::toBytes("content"));

    // Try to access snapshot
    AFL_CHECK_THROWS(a("nonexistant snapshot"), testee.createDirectoryHandler("snapshot:s:int:", log), afl::except::FileProblemException);
    AFL_CHECK_THROWS(a("syntax error"),         testee.createDirectoryHandler("snapshot:x",      log), afl::except::FileProblemException);
    AFL_CHECK_THROWS(a("nonexistant path"),     testee.createDirectoryHandler("snapshot:s:/foo", log), afl::except::FileProblemException);
}
