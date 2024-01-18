/**
  *  \file test/server/file/ca/garbagecollectortest.cpp
  *  \brief Test for server::file::ca::GarbageCollector
  */

#include "server/file/ca/garbagecollector.hpp"

#include "afl/io/internaldirectory.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
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
    void createSomeFiles(afl::test::Assert a, server::file::InternalDirectoryHandler& rootHandler)
    {
        server::file::ca::Root t(rootHandler);
        a.checkEqual("createSomeFiles > getMasterCommitId", t.getMasterCommitId(), server::file::ca::ObjectId::nil);

        server::file::DirectoryItem rootItem("(ca-root)", 0, std::auto_ptr<server::file::DirectoryHandler>(t.createRootHandler()));
        server::file::DirectoryItem* subdirItem = rootItem.createDirectory("d");
        subdirItem->createFile("f", afl::string::toBytes("text"));
        subdirItem->createFile("g", afl::string::toBytes("text"));
    }

    /* Modify some files: updates the "f" file with different content */
    void modifyFiles(afl::test::Assert a, server::file::ca::Root& t)
    {
        server::file::DirectoryItem rootItem("(ca-root)", 0, std::auto_ptr<server::file::DirectoryHandler>(t.createRootHandler()));
        server::file::Root serverRoot(rootItem, afl::io::InternalDirectory::create("<spec>"));
        rootItem.readContent(serverRoot);
        a.checkEqual("modifyFiles > getNumDirectories", rootItem.getNumDirectories(), 1U);

        server::file::DirectoryItem* subdirItem = rootItem.getDirectoryByIndex(0);
        a.checkNonNull("modifyFiles > subdirItem", subdirItem);
        subdirItem->readContent(serverRoot);
        subdirItem->createFile("f", afl::string::toBytes("moretext"));
    }

    /* Modify some files (convenience version).
       Because the ca module is internally caching stuff, this version must not be used when another
       instance of ca::Root and its children is active. */
    void modifyFiles(afl::test::Assert a, server::file::InternalDirectoryHandler& rootHandler)
    {
        server::file::ca::Root t(rootHandler);
        modifyFiles(a, t);
    }

    /* Standard synchronous garbage collector loop */
    void runGC(afl::test::Assert a, server::file::ca::Root& t, server::file::ca::GarbageCollector& testee)
    {
        testee.addCommit(t.getMasterCommitId());
        int n = 0;
        while (testee.checkObject()) {
            a.check("runGC > checkObject", ++n < 10000);
        }
        while (testee.removeGarbageObjects()) {
            a.check("runGC > removeGarbageObjects", ++n < 10000);
        }
    }

    /* Check file content */
    void checkFileContent(afl::test::Assert a, server::file::InternalDirectoryHandler& rootHandler, const char* fContent, const char* gContent)
    {
        server::file::ca::Root t(rootHandler);
        server::file::DirectoryItem rootItem("(ca-root)", 0, std::auto_ptr<server::file::DirectoryHandler>(t.createRootHandler()));
        server::file::Root serverRoot(rootItem, afl::io::InternalDirectory::create("<spec>"));
        rootItem.readContent(serverRoot);

        // Look up 'd'
        a.checkEqual("checkFileContent > getNumDirectories", rootItem.getNumDirectories(), 1U);
        server::file::DirectoryItem* subdirItem = rootItem.getDirectoryByIndex(0);
        a.checkNonNull("checkFileContent > subdirItem", subdirItem);
        subdirItem->readContent(serverRoot);
        a.checkEqual("checkEqual > getNumFiles", subdirItem->getNumFiles(), 2U);

        // Look up 'f'
        server::file::FileItem* f = subdirItem->getFileByIndex(0);
        a.checkNonNull("checkFileContent > file 0", f);
        a.checkEqual("checkFileContent > file 0 getName", f->getName(), "f");
        a.check("checkFileContent > file 0 content", subdirItem->getFileContent(*f)->get().equalContent(afl::string::toBytes(fContent)));

        // Look up 'g'
        server::file::FileItem* g = subdirItem->getFileByIndex(1);
        a.checkNonNull("checkFileContent > file 1", g);
        a.checkEqual("checkFileContent > file 1 getName", g->getName(), "g");
        a.check("checkFileContent > file 1 content", subdirItem->getFileContent(*g)->get().equalContent(afl::string::toBytes(gContent)));
    }

    /* Get directory, given its name */
    server::file::DirectoryHandler* getDirectory(afl::test::Assert a, server::file::DirectoryHandler& parent, String_t name)
    {
        server::file::DirectoryHandler::Info info;
        if (!parent.findItem(name, info) || info.type != server::file::DirectoryHandler::IsDirectory) {
            a.fail("getDirectory: " + name + ": not found");
        }
        return parent.getDirectory(info);
    }
}

/** Test normal behaviour (synchronous GC, no garbage).
    A: create some files. Run GC.
    E: expected stats generated, nothing removed. */
AFL_TEST("server.file.ca.GarbageCollector:normal", a)
{
    // Storage
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);
    createSomeFiles(a, rootHandler);

    // Garbage collector
    {
        afl::sys::Log log;
        server::file::ca::Root t(rootHandler);
        server::file::ca::GarbageCollector testee(t.objectStore(), log);

        // Add master commit
        testee.addCommit(t.getMasterCommitId());
        a.checkEqual("01. getNumObjectsToCheck", testee.getNumObjectsToCheck(), 1U);

        // Must refuse to remove garbage at this point
        a.check("11. removeGarbageObjects", !testee.removeGarbageObjects());

        // But must scan
        a.check("21. checkObject", testee.checkObject());

        // Remainder of the loop
        int n = 0;
        while (testee.checkObject()) {
            a.check("31. checkObject", ++n < 10000);
        }
        while (testee.removeGarbageObjects()) {
            a.check("32. removeGarbageObjects", ++n < 10000);
        }

        // Must not find any errors
        a.checkEqual("41. getNumErrors", testee.getNumErrors(), 0U);

        // Must keep 4 objects (commit, root tree, 'd', 'f'+'g' (one blob only))
        a.checkEqual("51. getNumObjectsToKeep", testee.getNumObjectsToKeep(), 4U);

        // Must not remove anything
        a.checkEqual("61. getNumObjectsRemoved", testee.getNumObjectsRemoved(), 0U);
    }

    // Verify content
    checkFileContent(a, rootHandler, "text", "text");
}

/** Test normal behaviour (synchronous GC, garbage present).
    A: create some files. Modify with a new instance (=creates garbage). Run GC.
    E: expected stats generated, garbage removed. */
AFL_TEST("server.file.ca.GarbageCollector:garbage", a)
{
    // Storage
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);
    createSomeFiles(a, rootHandler);
    modifyFiles(a, rootHandler);

    // Garbage collector
    {
        afl::sys::Log log;
        server::file::ca::Root t(rootHandler);
        server::file::ca::GarbageCollector testee(t.objectStore(), log);
        runGC(a, t, testee);

        // Must not find any errors
        a.checkEqual("01. getNumErrors", testee.getNumErrors(), 0U);

        // Must keep 4 objects (commit, root tree, 'd', 'f', 'g')
        a.checkEqual("11. getNumObjectsToKeep", testee.getNumObjectsToKeep(), 5U);

        // Must remove 3 objects (old commit, old root, old 'd')
        a.checkEqual("21. getNumObjectsRemoved", testee.getNumObjectsRemoved(), 3U);
    }

    // Verify content
    checkFileContent(a, rootHandler, "moretext", "text");
}

/** Test sliced garbage collection.
    A: create some files. Modify with a new instance (=creates garbage). Run GC, permanently pushing a new (=same) commit Id.
    E: GC completes with expected stats. */
AFL_TEST("server.file.ca.GarbageCollector:sliced", a)
{
    // Storage
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);
    createSomeFiles(a, rootHandler);
    modifyFiles(a, rootHandler);

    // Garbage collector
    {
        afl::sys::Log log;
        server::file::ca::Root t(rootHandler);
        server::file::ca::GarbageCollector testee(t.objectStore(), log);

        int n = 0;
        testee.addCommit(t.getMasterCommitId());
        while (testee.checkObject() || testee.removeGarbageObjects()) {
            testee.addCommit(t.getMasterCommitId());
            a.check("01. loop", ++n < 10000);
        }

        // Must not find any errors
        a.checkEqual("11. getNumErrors", testee.getNumErrors(), 0U);

        // Must keep 4 objects (commit, root tree, 'd', 'f', 'g')
        a.checkEqual("21. getNumObjectsToKeep", testee.getNumObjectsToKeep(), 5U);

        // Must remove 3 objects (old commit, old root, old 'd')
        a.checkEqual("31. getNumObjectsRemoved", testee.getNumObjectsRemoved(), 3U);
    }

    // Verify content
    checkFileContent(a, rootHandler, "moretext", "text");
}

/** Test sliced garbage collection with parallel modification.
    A: create some files. Modify with a new instance (=creates garbage). Run GC, adding new content after the checkObject() phase.
    E: GC completes with expected stats. */
AFL_TEST("server.file.ca.GarbageCollector:sliced-modified", a)
{
    // Storage
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);
    createSomeFiles(a, rootHandler);

    // Garbage collector
    {
        afl::sys::Log log;
        server::file::ca::Root t(rootHandler);
        server::file::ca::GarbageCollector testee(t.objectStore(), log);

        int n = 0;
        testee.addCommit(t.getMasterCommitId());
        while (testee.checkObject()) {
            a.check("01. loop", ++n < 10000);
        }

        modifyFiles(a, t);
        testee.addCommit(t.getMasterCommitId());
        while (testee.checkObject() || testee.removeGarbageObjects()) {
            a.check("11. loop", ++n < 10000);
        }

        // Must not find any errors
        a.checkEqual("21. getNumErrors", testee.getNumErrors(), 0U);

        // Must keep 8 objects: live modification will be conservative and preserve more than needed
        a.checkEqual("31. getNumObjectsToKeep", testee.getNumObjectsToKeep(), 8U);

        // Must remove 0 objects
        a.checkEqual("41. getNumObjectsRemoved", testee.getNumObjectsRemoved(), 0U);
    }

    // Verify content
    checkFileContent(a, rootHandler, "moretext", "text");
}

/** Test error: missing commit.
    A: create some files. Remove the root commit. Run GC.
    E: GC completes with expected stats; in particular, on error report. */
AFL_TEST("server.file.ca.GarbageCollector:error:missing-commit", a)
{
    // Storage
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);
    createSomeFiles(a, rootHandler);

    // Remove the commit file (0d/6c4c6f0d33fbe7ecda7604b0237b5ee02d3e4d)
    std::auto_ptr<server::file::DirectoryHandler> objects(getDirectory(a, rootHandler, "objects"));
    std::auto_ptr<server::file::DirectoryHandler> zd(getDirectory(a, *objects, "0d"));
    zd->removeFile("6c4c6f0d33fbe7ecda7604b0237b5ee02d3e4d");

    // Garbage collector
    {
        afl::sys::Log log;
        server::file::ca::Root t(rootHandler);
        server::file::ca::GarbageCollector testee(t.objectStore(), log);

        runGC(a, t, testee);

        // Must find one error: the missing commit
        a.checkEqual("01. getNumErrors", testee.getNumErrors(), 1U);

        // Must (try to) keep 1 object: the commit
        a.checkEqual("11. getNumObjectsToKeep", testee.getNumObjectsToKeep(), 1U);

        // Must remove 3 objects (=everything else)
        a.checkEqual("21. getNumObjectsRemoved", testee.getNumObjectsRemoved(), 3U);
    }
}

/** Test error: missing tree.
    A: create some files. Remove a tree commit. Run GC.
    E: GC completes with expected stats; in particular, on error report. */
AFL_TEST("server.file.ca.GarbageCollector:error:missing-tree", a)
{

    // Storage
    server::file::InternalDirectoryHandler::Directory rootDir("");
    server::file::InternalDirectoryHandler rootHandler("root", rootDir);
    createSomeFiles(a, rootHandler);

    // Remove the tree (a0/6cfb66e52f140169cbf8e5062df94d1a303c1e)
    std::auto_ptr<server::file::DirectoryHandler> objects(getDirectory(a, rootHandler, "objects"));
    std::auto_ptr<server::file::DirectoryHandler> a0(getDirectory(a, *objects, "a0"));
    a0->removeFile("6cfb66e52f140169cbf8e5062df94d1a303c1e");

    // Garbage collector
    {
        afl::sys::Log log;
        server::file::ca::Root t(rootHandler);
        server::file::ca::GarbageCollector testee(t.objectStore(), log);

        runGC(a, t, testee);

        // Must find one error: the missing tree
        a.checkEqual("01. getNumErrors", testee.getNumErrors(), 1U);

        // Must (try to) keep 3 objects: commit, root tree, missing tree
        a.checkEqual("11. getNumObjectsToKeep", testee.getNumObjectsToKeep(), 3U);

        // Must remove 1 object (file content)
        a.checkEqual("21. getNumObjectsRemoved", testee.getNumObjectsRemoved(), 1U);
    }
}
