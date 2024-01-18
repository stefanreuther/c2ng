/**
  *  \file test/server/file/utilstest.cpp
  *  \brief Test for server::file::Utils
  */

#include "server/file/utils.hpp"

#include "afl/except/fileproblemexception.hpp"
#include "afl/test/testrunner.hpp"
#include "server/file/internaldirectoryhandler.hpp"
#include <memory>

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
AFL_TEST("server.file.Utils:copyDirectory:CopyRecursively", a)
{
    InternalDirectoryHandler::Directory root("");
    InternalDirectoryHandler rootHandler("root", root);
    populate(rootHandler);

    // Copy, recursively
    InternalDirectoryHandler::Directory out("");
    InternalDirectoryHandler outHandler("root", out);

    server::file::copyDirectory(outHandler, rootHandler, server::file::CopyFlags_t(server::file::CopyRecursively));

    // Verify
    a.check       ("01. content a",   outHandler.getFileByName("a")->get().equalContent(afl::string::toBytes("xyz")));
    a.check       ("02. content b",   outHandler.getFileByName("b")->get().equalContent(afl::string::toBytes("pqr")));
    a.checkNonNull("03. dir d",       outHandler.findDirectory("d"));
    a.checkEqual  ("04. content d",   outHandler.findDirectory("d")->files.size(), 1U);
    a.check       ("05. file d/f",    outHandler.findDirectory("d")->files[0]->name == "f");
    a.check       ("06. content d/f", outHandler.findDirectory("d")->files[0]->content.equalContent(afl::string::toBytes("abc")));
}

/** Test copying, non-recursively. */
AFL_TEST("server.file.Utils:copyDirectory:flat", a)
{
    InternalDirectoryHandler::Directory root("");
    InternalDirectoryHandler rootHandler("root", root);
    populate(rootHandler);

    // Copy, non-recursively
    InternalDirectoryHandler::Directory out("");
    InternalDirectoryHandler outHandler("root", out);

    server::file::copyDirectory(outHandler, rootHandler, server::file::CopyFlags_t());

    // Verify
    a.check    ("01. content a", outHandler.getFileByName("a")->get().equalContent(afl::string::toBytes("xyz")));
    a.check    ("02. content b", outHandler.getFileByName("b")->get().equalContent(afl::string::toBytes("pqr")));
    a.checkNull("03. dir d",     outHandler.findDirectory("d"));
}

/** Test copy conflict. */
AFL_TEST("server.file.Utils:copyDirectory:error:dir-over-file", a)
{
    InternalDirectoryHandler::Directory root("");
    InternalDirectoryHandler rootHandler("root", root);
    populate(rootHandler);

    // Copy, recursively, but there is a file where the source has a directory
    InternalDirectoryHandler::Directory out("");
    InternalDirectoryHandler outHandler("root", out);
    outHandler.createFile("d", afl::base::Nothing);

    AFL_CHECK_THROWS(a, server::file::copyDirectory(outHandler, rootHandler, server::file::CopyFlags_t(server::file::CopyRecursively)), afl::except::FileProblemException);
}

/** Test copy conflict. */
AFL_TEST("server.file.Utils:copyDirectory:error:file-over-dir", a)
{
    InternalDirectoryHandler::Directory root("");
    InternalDirectoryHandler rootHandler("root", root);
    populate(rootHandler);

    // Copy, recursively, but there is a directory where the source has a file
    InternalDirectoryHandler::Directory out("");
    InternalDirectoryHandler outHandler("root", out);
    outHandler.createDirectory("a");

    AFL_CHECK_THROWS(a, server::file::copyDirectory(outHandler, rootHandler, server::file::CopyFlags_t(server::file::CopyRecursively)), afl::except::FileProblemException);
}

/** Test removeDirectoryContent. */
AFL_TEST("server.file.Utils:removeDirectoryContent", a)
{
    InternalDirectoryHandler::Directory root("");
    InternalDirectoryHandler rootHandler("root", root);
    populate(rootHandler);

    a.check("01. subdirectories", !root.subdirectories.empty());
    a.check("02. files",          !root.files.empty());

    AFL_CHECK_SUCCEEDS(a("11. removeDirectoryContent"), server::file::removeDirectoryContent(rootHandler));

    a.check("21. subdirectories", root.subdirectories.empty());
    a.check("22. files",          root.files.empty());
}

/** Test synchronizeDirectories between empty directories (border case). */
AFL_TEST("server.file.Utils:synchronizeDirectories:empty", a)
{
    InternalDirectoryHandler::Directory inDir("in");
    InternalDirectoryHandler inHandler("in", inDir);

    InternalDirectoryHandler::Directory outDir("out");
    InternalDirectoryHandler outHandler("out", outDir);

    AFL_CHECK_SUCCEEDS(a("01. synchronizeDirectories"), server::file::synchronizeDirectories(outHandler, inHandler));

    a.check("11. subdirectories", inDir.subdirectories.empty());
    a.check("12. files",          inDir.files.empty());
    a.check("13. subdirectories", outDir.subdirectories.empty());
    a.check("14. files",          outDir.files.empty());
}

/** Test synchronizeDirectories of populated directory into empty directory. */
AFL_TEST("server.file.Utils:synchronizeDirectories:full-to-empty", a)
{
    InternalDirectoryHandler::Directory inDir("in");
    InternalDirectoryHandler inHandler("in", inDir);
    populate(inHandler);

    InternalDirectoryHandler::Directory outDir("out");
    InternalDirectoryHandler outHandler("out", outDir);

    AFL_CHECK_SUCCEEDS(a("01. synchronizeDirectories"), server::file::synchronizeDirectories(outHandler, inHandler));

    a.check       ("11. file a", outHandler.getFileByName("a")->get().equalContent(afl::string::toBytes("xyz")));
    a.check       ("12. file b", outHandler.getFileByName("b")->get().equalContent(afl::string::toBytes("pqr")));
    a.checkNonNull("13. dir d",  outHandler.findDirectory("d"));
}

/** Test synchronizeDirectories of empty into populated directory. */
AFL_TEST("server.file.Utils:synchronizeDirectories:empty-to-full", a)
{
    InternalDirectoryHandler::Directory inDir("in");
    InternalDirectoryHandler inHandler("in", inDir);

    InternalDirectoryHandler::Directory outDir("out");
    InternalDirectoryHandler outHandler("out", outDir);
    populate(outHandler);

    AFL_CHECK_SUCCEEDS(a("01. synchronizeDirectories"), server::file::synchronizeDirectories(outHandler, inHandler));

    a.check("11. subdirectories", inDir.subdirectories.empty());
    a.check("12. files",          inDir.files.empty());
    a.check("13. subdirectories", outDir.subdirectories.empty());
    a.check("14. files",          outDir.files.empty());
}

/** Test synchronizeDirectories of populated directory into identical directory. */
AFL_TEST("server.file.Utils:synchronizeDirectories:same", a)
{
    InternalDirectoryHandler::Directory inDir("in");
    InternalDirectoryHandler inHandler("in", inDir);
    populate(inHandler);

    InternalDirectoryHandler::Directory outDir("out");
    InternalDirectoryHandler outHandler("out", outDir);
    populate(outHandler);

    AFL_CHECK_SUCCEEDS(a("01. synchronizeDirectories"), server::file::synchronizeDirectories(outHandler, inHandler));

    a.check       ("11. file a", outHandler.getFileByName("a")->get().equalContent(afl::string::toBytes("xyz")));
    a.check       ("12. file b", outHandler.getFileByName("b")->get().equalContent(afl::string::toBytes("pqr")));
    a.checkNonNull("13. dir d",  outHandler.findDirectory("d"));
}

/** Test synchronizeDirectories when the target has a directory where the source has a file. */
AFL_TEST("server.file.Utils:synchronizeDirectories:file-over-dir", a)
{
    InternalDirectoryHandler::Directory inDir("in");
    InternalDirectoryHandler inHandler("in", inDir);
    populate(inHandler);

    InternalDirectoryHandler::Directory outDir("out");
    InternalDirectoryHandler outHandler("out", outDir);
    outHandler.createDirectory("a");

    AFL_CHECK_SUCCEEDS(a("01. synchronizeDirectories"), server::file::synchronizeDirectories(outHandler, inHandler));

    a.check       ("11. file a", outHandler.getFileByName("a")->get().equalContent(afl::string::toBytes("xyz")));
    a.check       ("12. file b", outHandler.getFileByName("b")->get().equalContent(afl::string::toBytes("pqr")));
    a.checkNonNull("13. dir d",  outHandler.findDirectory("d"));
}

/** Test synchronizeDirectories when the target has a file where the source has a directory. */
AFL_TEST("server.file.Utils:synchronizeDirectories:dir-over-file", a)
{
    InternalDirectoryHandler::Directory inDir("in");
    InternalDirectoryHandler inHandler("in", inDir);
    populate(inHandler);

    InternalDirectoryHandler::Directory outDir("out");
    InternalDirectoryHandler outHandler("out", outDir);
    outHandler.createFile("d", afl::string::toBytes("qqq"));

    AFL_CHECK_SUCCEEDS(a("01. synchronizeDirectories"), server::file::synchronizeDirectories(outHandler, inHandler));

    a.check       ("11. file a", outHandler.getFileByName("a")->get().equalContent(afl::string::toBytes("xyz")));
    a.check       ("12. file b", outHandler.getFileByName("b")->get().equalContent(afl::string::toBytes("pqr")));
    a.checkNonNull("13. dir d",  outHandler.findDirectory("d"));
}
