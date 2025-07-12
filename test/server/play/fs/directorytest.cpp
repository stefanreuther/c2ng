/**
  *  \file test/server/play/fs/directorytest.cpp
  *  \brief Test for server::play::fs::Directory
  */

#include "server/play/fs/directory.hpp"

#include "afl/base/stoppable.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/stream.hpp"
#include "afl/net/internalnetworkstack.hpp"
#include "afl/net/name.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/resp/protocolhandler.hpp"
#include "afl/net/server.hpp"
#include "afl/sys/thread.hpp"
#include "afl/test/testrunner.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/interface/filebaseclient.hpp"

using afl::base::Enumerator;
using afl::base::Ptr;
using afl::base::Ref;
using afl::except::FileProblemException;
using afl::io::Directory;
using afl::io::DirectoryEntry;
using afl::io::FileSystem;
using afl::io::Stream;
using afl::net::InternalNetworkStack;
using afl::net::Name;
using afl::net::NetworkStack;
using server::interface::FileBaseClient;
using server::play::fs::Session;

namespace {
    /* Server mock.

       Caveat emptor:
       * this treats all connections as one session
       * no locking. Make sure that no background (network) accesses happen when client() is used. */
    class ServerMock : private afl::net::ProtocolHandlerFactory {
     public:
        ServerMock(NetworkStack& net, Name name)
            : m_filer(),
              m_server(net.listen(name, 10), *this),
              m_thread("ServerMock", m_server),
              m_client(m_filer)
            {
                m_thread.start();
            }
        ~ServerMock()
            {
                m_server.stop();
                m_thread.join();
            }

        virtual afl::net::ProtocolHandler* create()
            { return new afl::net::resp::ProtocolHandler(m_filer); }

        FileBaseClient& client()
            { return m_client; }

     private:
        server::file::InternalFileServer m_filer;
        afl::net::Server m_server;
        afl::sys::Thread m_thread;
        FileBaseClient m_client;
    };
}

/* Basic broad coverage test */
AFL_TEST("server.play.fs.Directory:basics", a)
{
    const Name netAddr("host", "port");
    Ref<InternalNetworkStack> net = InternalNetworkStack::create();
    ServerMock server(*net, netAddr);
    server.client().createDirectoryAsUser("dir", "fred");
    server.client().createDirectory("dir/sub");
    server.client().putFile("dir/test.txt", "content");
    server.client().putFile("dir/test2.txt", "y");

    Ref<Session> session = Session::create(*net, netAddr, "fred");
    Ref<Directory> dir = server::play::fs::Directory::create(session, "/dir");

    // Metadata
    // "ServerDirectory diff" = behaviour differences between original implementation and ServerDirectory-based implementation
    // a.checkEqual("01. dirName", dir->getDirectoryName(), "dir"); -- ServerDirectory diff
    a.checkEqual("01. title", dir->getTitle(), "/dir");
    a.checkNull("02. parent",  dir->getParentDirectory().get());

    // File access
    {
        Ref<Stream> f = dir->openFile("test.txt", FileSystem::OpenRead);
        a.checkEqual("11. pos",     f->getPos(), 0U);
        a.checkEqual("12. size",    f->getSize(), 7U);
        a.checkEqual("13. content", afl::string::fromBytes(f->createVirtualMapping()->get()), "content");
        // a.checkEqual("14. name",    f->getName(), "/dir/test.txt"); -- ServerDirectory diff
        a.checkEqual("15. read",    f->getCapabilities() & Stream::CanRead, Stream::CanRead);
        // a.checkEqual("16. write",   f->getCapabilities() & Stream::CanWrite, 0U); -- ServerDirectory diff
    }

    // Enumeration
    Ref<Enumerator<Ptr<DirectoryEntry> > > it = dir->getDirectoryEntries();
    Ptr<DirectoryEntry> e;
    bool hasFirst = false;
    bool hasSecond = false;
    bool hasSub = false;
    while (it->getNextElement(e)) {
        a.checkNonNull("21. ptr", e.get());
        if (e->getTitle() == "test.txt") {
            a.check("22a. has", !hasFirst);
            a.checkEqual("22b. type", e->getFileType(), DirectoryEntry::tFile);
            a.checkEqual("22c. size", e->getFileSize(), 7U);
            // a.checkEqual("22d. path", e->getPathName(), "/dir/test.txt"); -- ServerDirectory diff
            hasFirst = true;
        } else if (e->getTitle() == "test2.txt") {
            a.check("23a. has", !hasSecond);
            a.checkEqual("23b. type", e->getFileType(), DirectoryEntry::tFile);
            a.checkEqual("23c. size", e->getFileSize(), 1U);
            // a.checkEqual("23d. path", e->getPathName(), "/dir/test2.txt"); -- ServerDirectory diff
            hasSecond = true;
        } else if (e->getTitle() == "sub") {
            a.check("24a. has", !hasSub);
            a.checkEqual("24b. type", e->getFileType(), DirectoryEntry::tDirectory);
            // a.checkEqual("24d. path", e->getPathName(), "/dir/sub"); -- ServerDirectory diff
            hasSub = true;
        } else {
            a.check("25. fail", false);
        }
    }
    a.check("26. hasFirst", hasFirst);
    a.check("27. hasSecond", hasSecond);
    a.check("28. hasSub", hasSub);

    // Open by name / access
    AFL_CHECK_THROWS(a("31. erase"),   dir->erase("erase.txt"),                                                     FileProblemException);
    AFL_CHECK_THROWS(a("32. erase"),   dir->getDirectoryEntryByName("erase.txt")->erase(),                          FileProblemException);
    AFL_CHECK_THROWS(a("33. mkdir"),   dir->getDirectoryEntryByName("newdir")->createAsDirectory(),                 FileProblemException);
    AFL_CHECK_THROWS(a("34. opendir"), dir->openDirectory("subdir"),                                                FileProblemException);
    AFL_CHECK_THROWS(a("35. opendir"), dir->getDirectoryEntryByName("subdir")->openDirectory(),                     FileProblemException);
    AFL_CHECK_THROWS(a("36. rename"),  dir->getDirectoryEntryByName("rename")->renameTo("x"),                       FileProblemException);
    AFL_CHECK_THROWS(a("37. move"),    dir->getDirectoryEntryByName("move")->moveTo(*dir, "x"),                     FileProblemException);
    AFL_CHECK_THROWS(a("38. flag"),    dir->getDirectoryEntryByName("flag")->setFlag(DirectoryEntry::Hidden, true), FileProblemException);

    a.checkEqual("41. parent", &*dir->getDirectoryEntryByName("test.txt")->openContainingDirectory(), &*dir);
    a.checkEqual("42. size",  dir->getDirectoryEntryByName("test.txt")->getFileSize(), 7U);

    // Modify
    {
        Ref<Stream> f = dir->openFile("new.txt", FileSystem::Create);
        f->fullWrite(afl::string::toBytes("new content"));
        f->flush();
        dir->flush();
    }

    a.checkEqual("51. new file", server.client().getFile("dir/new.txt"), "new content");
}

/* Test file update */
AFL_TEST("server.play.fs.Directory:update", a)
{
    const Name netAddr("host", "port");
    Ref<InternalNetworkStack> net = InternalNetworkStack::create();
    ServerMock server(*net, netAddr);
    server.client().createDirectoryAsUser("dir", "fred");
    server.client().putFile("dir/test.txt", "content");

    Ref<Session> session = Session::create(*net, netAddr, "fred");
    Ref<Directory> dir = server::play::fs::Directory::create(session, "/dir");

    // Update file
    {
        Ref<Stream> f = dir->openFile("test.txt", FileSystem::OpenWrite);
        f->setPos(2);
        f->fullWrite(afl::string::toBytes("ol new data"));
        f->flush();

        a.checkEqual("01. pos",     f->getPos(), 13U);
        a.checkEqual("02. size",    f->getSize(), 13U);
        a.checkEqual("03. read",    f->getCapabilities() & Stream::CanRead, Stream::CanRead);
        a.checkEqual("04. write",   f->getCapabilities() & Stream::CanWrite, Stream::CanWrite);
    }

    // flush()
    // Note that this will invalidate open files (a limitation of ServerDirectory)
    dir->flush();

    a.checkEqual("11. size",  dir->getDirectoryEntryByName("test.txt")->getFileSize(), 13U);
    a.checkEqual("12. new file", server.client().getFile("dir/test.txt"), "cool new data");
}

/* Test erase */
AFL_TEST("server.play.fs.Directory:erase", a)
{
    const Name netAddr("host", "port");
    Ref<InternalNetworkStack> net = InternalNetworkStack::create();
    ServerMock server(*net, netAddr);
    server.client().createDirectoryAsUser("dir", "fred");
    server.client().putFile("dir/test.txt", "content");

    Ref<Session> session = Session::create(*net, netAddr, "fred");
    Ref<Directory> dir = server::play::fs::Directory::create(session, "/dir");

    AFL_CHECK_SUCCEEDS(a("01. erase"), dir->erase("test.txt"));
    dir->flush();

    AFL_CHECK_THROWS(a("11. gone"), server.client().getFile("dir/test.txt"), std::exception);
}
