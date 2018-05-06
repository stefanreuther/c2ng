/**
  *  \file u/t_server_file_utils.cpp
  *  \brief Test for server::file::Utils
  */

#include <memory>
#include "server/file/utils.hpp"

#include "t_server_file.hpp"
#include "server/file/internaldirectoryhandler.hpp"
#include "afl/except/fileproblemexception.hpp"

using server::file::InternalDirectoryHandler;

namespace {
    /** Populate the given DirectoryHandler.
        Creates files /a, /b, d/f. */
    void populate(InternalDirectoryHandler& rootHandler)
    {
        rootHandler.createFile("a", afl::string::toBytes("xyz"));
        rootHandler.createFile("b", afl::string::toBytes("pqr"));
        std::auto_ptr<server::file::DirectoryHandler> d(rootHandler.getDirectory(rootHandler.createDirectory("d")));
        d->createFile("f", afl::string::toBytes("abc"));
    }
}

/** Test copying, recursively. */
void
TestServerFileUtils::testCopy()
{
    InternalDirectoryHandler::Directory root("");
    InternalDirectoryHandler rootHandler("root", root);
    populate(rootHandler);

    // Copy, recursively
    InternalDirectoryHandler::Directory out("");
    InternalDirectoryHandler outHandler("root", out);

    server::file::copyDirectory(outHandler, rootHandler, server::file::CopyFlags_t(server::file::CopyRecursively));

    // Verify
    TS_ASSERT(outHandler.getFileByName("a")->get().equalContent(afl::string::toBytes("xyz")));
    TS_ASSERT(outHandler.getFileByName("b")->get().equalContent(afl::string::toBytes("pqr")));
    TS_ASSERT(outHandler.findDirectory("d") != 0);
    TS_ASSERT_EQUALS(outHandler.findDirectory("d")->files.size(), 1U);
    TS_ASSERT(outHandler.findDirectory("d")->files[0]->name == "f");
    TS_ASSERT(outHandler.findDirectory("d")->files[0]->content.equalContent(afl::string::toBytes("abc")));
}

/** Test copying, non-recursively. */
void
TestServerFileUtils::testCopyFlat()
{
    InternalDirectoryHandler::Directory root("");
    InternalDirectoryHandler rootHandler("root", root);
    populate(rootHandler);

    // Copy, non-recursively
    InternalDirectoryHandler::Directory out("");
    InternalDirectoryHandler outHandler("root", out);

    server::file::copyDirectory(outHandler, rootHandler, server::file::CopyFlags_t());

    // Verify
    TS_ASSERT(outHandler.getFileByName("a")->get().equalContent(afl::string::toBytes("xyz")));
    TS_ASSERT(outHandler.getFileByName("b")->get().equalContent(afl::string::toBytes("pqr")));
    TS_ASSERT(outHandler.findDirectory("d") == 0);
}

/** Test copy conflict. */
void
TestServerFileUtils::testCopyConflict()
{
    InternalDirectoryHandler::Directory root("");
    InternalDirectoryHandler rootHandler("root", root);
    populate(rootHandler);

    // Copy, recursively, but there is a file where the source has a directory
    InternalDirectoryHandler::Directory out("");
    InternalDirectoryHandler outHandler("root", out);
    outHandler.createFile("d", afl::base::Nothing);

    TS_ASSERT_THROWS(server::file::copyDirectory(outHandler, rootHandler, server::file::CopyFlags_t(server::file::CopyRecursively)), afl::except::FileProblemException);
}

/** Test copy conflict. */
void
TestServerFileUtils::testCopyConflict2()
{
    InternalDirectoryHandler::Directory root("");
    InternalDirectoryHandler rootHandler("root", root);
    populate(rootHandler);

    // Copy, recursively, but there is a directory where the source has a file
    InternalDirectoryHandler::Directory out("");
    InternalDirectoryHandler outHandler("root", out);
    outHandler.createDirectory("a");

    TS_ASSERT_THROWS(server::file::copyDirectory(outHandler, rootHandler, server::file::CopyFlags_t(server::file::CopyRecursively)), afl::except::FileProblemException);
}

/** Test removeDirectoryContent. */
void
TestServerFileUtils::testRemoveDir()
{
    InternalDirectoryHandler::Directory root("");
    InternalDirectoryHandler rootHandler("root", root);
    populate(rootHandler);

    TS_ASSERT(!root.subdirectories.empty());
    TS_ASSERT(!root.files.empty());

    TS_ASSERT_THROWS_NOTHING(server::file::removeDirectoryContent(rootHandler));

    TS_ASSERT(root.subdirectories.empty());
    TS_ASSERT(root.files.empty());
}

/** Test synchronizeDirectories between empty directories (border case). */
void
TestServerFileUtils::testSyncEmpty()
{
    InternalDirectoryHandler::Directory inDir("in");
    InternalDirectoryHandler inHandler("in", inDir);

    InternalDirectoryHandler::Directory outDir("out");
    InternalDirectoryHandler outHandler("out", outDir);

    TS_ASSERT_THROWS_NOTHING(server::file::synchronizeDirectories(outHandler, inHandler));

    TS_ASSERT(inDir.subdirectories.empty());
    TS_ASSERT(inDir.files.empty());
    TS_ASSERT(outDir.subdirectories.empty());
    TS_ASSERT(outDir.files.empty());
}

/** Test synchronizeDirectories of populated directory into empty directory. */
void
TestServerFileUtils::testSyncIntoEmpty()
{
    InternalDirectoryHandler::Directory inDir("in");
    InternalDirectoryHandler inHandler("in", inDir);
    populate(inHandler);

    InternalDirectoryHandler::Directory outDir("out");
    InternalDirectoryHandler outHandler("out", outDir);

    TS_ASSERT_THROWS_NOTHING(server::file::synchronizeDirectories(outHandler, inHandler));

    TS_ASSERT(outHandler.getFileByName("a")->get().equalContent(afl::string::toBytes("xyz")));
    TS_ASSERT(outHandler.getFileByName("b")->get().equalContent(afl::string::toBytes("pqr")));
    TS_ASSERT(outHandler.findDirectory("d") != 0);
}

/** Test synchronizeDirectories of empty into populated directory. */
void
TestServerFileUtils::testSyncFromEmpty()
{
    InternalDirectoryHandler::Directory inDir("in");
    InternalDirectoryHandler inHandler("in", inDir);

    InternalDirectoryHandler::Directory outDir("out");
    InternalDirectoryHandler outHandler("out", outDir);
    populate(outHandler);

    TS_ASSERT_THROWS_NOTHING(server::file::synchronizeDirectories(outHandler, inHandler));

    TS_ASSERT(inDir.subdirectories.empty());
    TS_ASSERT(inDir.files.empty());
    TS_ASSERT(outDir.subdirectories.empty());
    TS_ASSERT(outDir.files.empty());
}

/** Test synchronizeDirectories of populated directory into identical directory. */
void
TestServerFileUtils::testSyncSame()
{
    InternalDirectoryHandler::Directory inDir("in");
    InternalDirectoryHandler inHandler("in", inDir);
    populate(inHandler);

    InternalDirectoryHandler::Directory outDir("out");
    InternalDirectoryHandler outHandler("out", outDir);
    populate(outHandler);

    TS_ASSERT_THROWS_NOTHING(server::file::synchronizeDirectories(outHandler, inHandler));

    TS_ASSERT(outHandler.getFileByName("a")->get().equalContent(afl::string::toBytes("xyz")));
    TS_ASSERT(outHandler.getFileByName("b")->get().equalContent(afl::string::toBytes("pqr")));
    TS_ASSERT(outHandler.findDirectory("d") != 0);
}

/** Test synchronizeDirectories when the target has a directory where the source has a file. */
void
TestServerFileUtils::testSyncFileOverDir()
{
    InternalDirectoryHandler::Directory inDir("in");
    InternalDirectoryHandler inHandler("in", inDir);
    populate(inHandler);

    InternalDirectoryHandler::Directory outDir("out");
    InternalDirectoryHandler outHandler("out", outDir);
    outHandler.createDirectory("a");

    TS_ASSERT_THROWS_NOTHING(server::file::synchronizeDirectories(outHandler, inHandler));

    TS_ASSERT(outHandler.getFileByName("a")->get().equalContent(afl::string::toBytes("xyz")));
    TS_ASSERT(outHandler.getFileByName("b")->get().equalContent(afl::string::toBytes("pqr")));
    TS_ASSERT(outHandler.findDirectory("d") != 0);
}

/** Test synchronizeDirectories when the target has a file where the source has a directory. */
void
TestServerFileUtils::testSyncDirOverFile()
{
    InternalDirectoryHandler::Directory inDir("in");
    InternalDirectoryHandler inHandler("in", inDir);
    populate(inHandler);

    InternalDirectoryHandler::Directory outDir("out");
    InternalDirectoryHandler outHandler("out", outDir);
    outHandler.createFile("d", afl::string::toBytes("qqq"));

    TS_ASSERT_THROWS_NOTHING(server::file::synchronizeDirectories(outHandler, inHandler));

    TS_ASSERT(outHandler.getFileByName("a")->get().equalContent(afl::string::toBytes("xyz")));
    TS_ASSERT(outHandler.getFileByName("b")->get().equalContent(afl::string::toBytes("pqr")));
    TS_ASSERT(outHandler.findDirectory("d") != 0);
}

