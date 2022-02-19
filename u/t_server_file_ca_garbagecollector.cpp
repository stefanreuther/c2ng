/**
  *  \file u/t_server_file_ca_garbagecollector.cpp
  *  \brief Test for server::file::ca::GarbageCollector
  */

#include "server/file/ca/garbagecollector.hpp"

#include "t_server_file_ca.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/sys/log.hpp"
#include "server/file/ca/objectstore.hpp"
#include "server/file/ca/root.hpp"
#include "server/file/directoryitem.hpp"
#include "server/file/internaldirectoryhandler.hpp"
#include "server/file/root.hpp"

namespace {
    /* Create some files:
         (root)             27e3fd6748cef25cf1eb1ae583b3d273c643001d
          \+ d              a06cfb66e52f140169cbf8e5062df94d1a303c1e
            \+ f "text"     f3a34851d44d6b97c90fbb99dd3d18c261b9a237
            \+ g "text"     f3a34851d44d6b97c90fbb99dd3d18c261b9a237 */
    void createSomeFiles(server::file::InternalDirectoryHandler& rootHandler)
    {
        server::file::ca::Root t(rootHandler);
        TS_ASSERT_EQUALS(t.getMasterCommitId(), server::file::ca::ObjectId::nil);

        server::file::DirectoryItem rootItem("(ca-root)", 0, std::auto_ptr<server::file::DirectoryHandler>(t.createRootHandler()));
        server::file::DirectoryItem* subdirItem = rootItem.createDirectory("d");
        subdirItem->createFile("f", afl::string::toBytes("text"));
        subdirItem->createFile("g", afl::string::toBytes("text"));
    }

    /* Modify some files: updates the "f" file with different content */
    void modifyFiles(server::file::ca::Root& t)
    {
        server::file::DirectoryItem rootItem("(ca-root)", 0, std::auto_ptr<server::file::DirectoryHandler>(t.createRootHandler()));
        server::file::Root serverRoot(rootItem, afl::io::InternalDirectory::create("<spec>"));
        rootItem.readContent(serverRoot);
        TS_ASSERT_EQUALS(rootItem.getNumDirectories(), 1U);

        server::file::DirectoryItem* subdirItem = rootItem.getDirectoryByIndex(0);
        TS_ASSERT(subdirItem != 0);
        subdirItem->readContent(serverRoot);
        subdirItem->createFile("f", afl::string::toBytes("moretext"));
    }

    /* Modify some files (convenience version).
       Because the ca module is internally caching stuff, this version must not be used when another
       instance of ca::Root and its children is active. */
    void modifyFiles(server::file::InternalDirectoryHandler& rootHandler)
    {
        server::file::ca::Root t(rootHandler);
        modifyFiles(t);
    }

    /* Standard synchronous garbage collector loop */
    void runGC(server::file::ca::Root& t, server::file::ca::GarbageCollector& testee)
    {
        testee.addCommit(t.getMasterCommitId());
        int n = 0;
        while (testee.checkObject()) {
            TS_ASSERT(++n < 10000);
        }
        while (testee.removeGarbageObjects()) {
            TS_ASSERT(++n < 10000);
        }
    }

    /* Check file content */
    void checkFileContent(server::file::InternalDirectoryHandler& rootHandler, const char* fContent, const char* gContent)
    {
        server::file::ca::Root t(rootHandler);
        server::file::DirectoryItem rootItem("(ca-root)", 0, std::auto_ptr<server::file::DirectoryHandler>(t.createRootHandler()));
        server::file::Root serverRoot(rootItem, afl::io::InternalDirectory::create("<spec>"));
        rootItem.readContent(serverRoot);

        // Look up 'd'
        TS_ASSERT_EQUALS(rootItem.getNumDirectories(), 1U);
        server::file::DirectoryItem* subdirItem = rootItem.getDirectoryByIndex(0);
        TS_ASSERT(subdirItem != 0);
        subdirItem->readContent(serverRoot);
        TS_ASSERT_EQUALS(subdirItem->getNumFiles(), 2U);

        // Look up 'f'
        server::file::FileItem* f = subdirItem->getFileByIndex(0);
        TS_ASSERT(f != 0);
        TS_ASSERT_EQUALS(f->getName(), "f");
        TS_ASSERT(subdirItem->getFileContent(*f)->get().equalContent(afl::string::toBytes(fContent)));

        // Look up 'g'
        server::file::FileItem* g = subdirItem->getFileByIndex(1);
        TS_ASSERT(g != 0);
        TS_ASSERT_EQUALS(g->getName(), "g");
        TS_ASSERT(subdirItem->getFileContent(*g)->get().equalContent(afl::string::toBytes(gContent)));
    }

    /* Get directory, given its name */
    server::file::DirectoryHandler* getDirectory(server::file::DirectoryHandler& parent, String_t name)
    {
        server::file::DirectoryHandler::Info info;
        if (!parent.findItem(name, info) || info.type != server::file::DirectoryHandler::IsDirectory) {
            TS_FAIL(name + ": not found");
        }
        return parent.getDirectory(info);
    }
}

/** Test normal behaviour (synchronous GC, no garbage).
    A: create some files. Run GC.
    E: expected stats generated, nothing removed. */
void
TestServerFileCaGarbageCollector::testNormal()
{
    CxxTest::setAbortTestOnFail(true);

    // Storage
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);
    createSomeFiles(rootHandler);

    // Garbage collector
    {
        afl::sys::Log log;
        server::file::ca::Root t(rootHandler);
        server::file::ca::GarbageCollector testee(t.objectStore(), log);

        // Add master commit
        testee.addCommit(t.getMasterCommitId());
        TS_ASSERT_EQUALS(testee.getNumObjectsToCheck(), 1U);

        // Must refuse to remove garbage at this point
        TS_ASSERT(!testee.removeGarbageObjects());

        // But must scan
        TS_ASSERT(testee.checkObject());

        // Remainder of the loop
        int n = 0;
        while (testee.checkObject()) {
            TS_ASSERT(++n < 10000);
        }
        while (testee.removeGarbageObjects()) {
            TS_ASSERT(++n < 10000);
        }

        // Must not find any errors
        TS_ASSERT_EQUALS(testee.getNumErrors(), 0U);

        // Must keep 4 objects (commit, root tree, 'd', 'f'+'g' (one blob only))
        TS_ASSERT_EQUALS(testee.getNumObjectsToKeep(), 4U);

        // Must not remove anything
        TS_ASSERT_EQUALS(testee.getNumObjectsRemoved(), 0U);
    }

    // Verify content
    checkFileContent(rootHandler, "text", "text");
}

/** Test normal behaviour (synchronous GC, garbage present).
    A: create some files. Modify with a new instance (=creates garbage). Run GC.
    E: expected stats generated, garbage removed. */
void
TestServerFileCaGarbageCollector::testGarbage()
{
    CxxTest::setAbortTestOnFail(true);

    // Storage
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);
    createSomeFiles(rootHandler);
    modifyFiles(rootHandler);

    // Garbage collector
    {
        afl::sys::Log log;
        server::file::ca::Root t(rootHandler);
        server::file::ca::GarbageCollector testee(t.objectStore(), log);
        runGC(t, testee);

        // Must not find any errors
        TS_ASSERT_EQUALS(testee.getNumErrors(), 0U);

        // Must keep 4 objects (commit, root tree, 'd', 'f', 'g')
        TS_ASSERT_EQUALS(testee.getNumObjectsToKeep(), 5U);

        // Must remove 3 objects (old commit, old root, old 'd')
        TS_ASSERT_EQUALS(testee.getNumObjectsRemoved(), 3U);
    }

    // Verify content
    checkFileContent(rootHandler, "moretext", "text");
}

/** Test sliced garbage collection.
    A: create some files. Modify with a new instance (=creates garbage). Run GC, permanently pushing a new (=same) commit Id.
    E: GC completes with expected stats. */
void
TestServerFileCaGarbageCollector::testSliced()
{
    CxxTest::setAbortTestOnFail(true);

    // Storage
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);
    createSomeFiles(rootHandler);
    modifyFiles(rootHandler);

    // Garbage collector
    {
        afl::sys::Log log;
        server::file::ca::Root t(rootHandler);
        server::file::ca::GarbageCollector testee(t.objectStore(), log);

        int n = 0;
        testee.addCommit(t.getMasterCommitId());
        while (testee.checkObject() || testee.removeGarbageObjects()) {
            testee.addCommit(t.getMasterCommitId());
            TS_ASSERT(++n < 10000);
        }

        // Must not find any errors
        TS_ASSERT_EQUALS(testee.getNumErrors(), 0U);

        // Must keep 4 objects (commit, root tree, 'd', 'f', 'g')
        TS_ASSERT_EQUALS(testee.getNumObjectsToKeep(), 5U);

        // Must remove 3 objects (old commit, old root, old 'd')
        TS_ASSERT_EQUALS(testee.getNumObjectsRemoved(), 3U);
    }

    // Verify content
    checkFileContent(rootHandler, "moretext", "text");
}

/** Test sliced garbage collection with parallel modification.
    A: create some files. Modify with a new instance (=creates garbage). Run GC, adding new content after the checkObject() phase.
    E: GC completes with expected stats. */
void
TestServerFileCaGarbageCollector::testModified()
{
    CxxTest::setAbortTestOnFail(true);

    // Storage
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);
    createSomeFiles(rootHandler);

    // Garbage collector
    {
        afl::sys::Log log;
        server::file::ca::Root t(rootHandler);
        server::file::ca::GarbageCollector testee(t.objectStore(), log);

        int n = 0;
        testee.addCommit(t.getMasterCommitId());
        while (testee.checkObject()) {
            TS_ASSERT(++n < 10000);
        }

        modifyFiles(t);
        testee.addCommit(t.getMasterCommitId());
        while (testee.checkObject() || testee.removeGarbageObjects()) {
            TS_ASSERT(++n < 10000);
        }

        // Must not find any errors
        TS_ASSERT_EQUALS(testee.getNumErrors(), 0U);

        // Must keep 8 objects: live modification will be conservative and preserve more than needed
        TS_ASSERT_EQUALS(testee.getNumObjectsToKeep(), 8U);

        // Must remove 0 objects
        TS_ASSERT_EQUALS(testee.getNumObjectsRemoved(), 0U);
    }

    // Verify content
    checkFileContent(rootHandler, "moretext", "text");
}

/** Test error: missing commit.
    A: create some files. Remove the root commit. Run GC.
    E: GC completes with expected stats; in particular, on error report. */
void
TestServerFileCaGarbageCollector::testErrorCommit()
{
    CxxTest::setAbortTestOnFail(true);

    // Storage
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);
    createSomeFiles(rootHandler);

    // Remove the commit file (0d/6c4c6f0d33fbe7ecda7604b0237b5ee02d3e4d)
    std::auto_ptr<server::file::DirectoryHandler> objects(getDirectory(rootHandler, "objects"));
    std::auto_ptr<server::file::DirectoryHandler> zd(getDirectory(*objects, "0d"));
    zd->removeFile("6c4c6f0d33fbe7ecda7604b0237b5ee02d3e4d");

    // Garbage collector
    {
        afl::sys::Log log;
        server::file::ca::Root t(rootHandler);
        server::file::ca::GarbageCollector testee(t.objectStore(), log);

        runGC(t, testee);

        // Must find one error: the missing commit
        TS_ASSERT_EQUALS(testee.getNumErrors(), 1U);

        // Must (try to) keep 1 object: the commit
        TS_ASSERT_EQUALS(testee.getNumObjectsToKeep(), 1U);

        // Must remove 3 objects (=everything else)
        TS_ASSERT_EQUALS(testee.getNumObjectsRemoved(), 3U);
    }
}

/** Test error: missing tree.
    A: create some files. Remove a tree commit. Run GC.
    E: GC completes with expected stats; in particular, on error report. */
void
TestServerFileCaGarbageCollector::testErrorTree()
{
    CxxTest::setAbortTestOnFail(true);

    // Storage
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);
    createSomeFiles(rootHandler);

    // Remove the tree (a0/6cfb66e52f140169cbf8e5062df94d1a303c1e)
    std::auto_ptr<server::file::DirectoryHandler> objects(getDirectory(rootHandler, "objects"));
    std::auto_ptr<server::file::DirectoryHandler> a0(getDirectory(*objects, "a0"));
    a0->removeFile("6cfb66e52f140169cbf8e5062df94d1a303c1e");

    // Garbage collector
    {
        afl::sys::Log log;
        server::file::ca::Root t(rootHandler);
        server::file::ca::GarbageCollector testee(t.objectStore(), log);

        runGC(t, testee);

        // Must find one error: the missing tree
        TS_ASSERT_EQUALS(testee.getNumErrors(), 1U);

        // Must (try to) keep 3 objects: commit, root tree, missing tree
        TS_ASSERT_EQUALS(testee.getNumObjectsToKeep(), 3U);

        // Must remove 1 object (file content)
        TS_ASSERT_EQUALS(testee.getNumObjectsRemoved(), 1U);
    }
}

