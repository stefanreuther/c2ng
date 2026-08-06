/**
  *  \file test/server/file/sharestoretest.cpp
  *  \brief Test for server::file::ShareStore
  */

#include "server/file/sharestore.hpp"

#include "afl/io/internaldirectory.hpp"
#include "afl/test/testrunner.hpp"
#include "server/file/directoryitem.hpp"
#include "server/file/internaldirectoryhandler.hpp"
#include "server/file/root.hpp"

using afl::io::InternalDirectory;
using server::file::DirectoryHandler;
using server::file::DirectoryItem;
using server::file::InternalDirectoryHandler;
using server::file::Root;
using server::file::ShareStore;

namespace {
    struct TestHarness {
        InternalDirectoryHandler::Directory dir;
        DirectoryItem rootDirectory;
        Root root;

        TestHarness()
            : dir("work"),
              rootDirectory("root", 0, std::auto_ptr<DirectoryHandler>(new InternalDirectoryHandler("work", dir))),
              root(rootDirectory, InternalDirectory::create("spec"))
            { }
    };
}

/** Test addShare, iteration, removeShare sequence. */
AFL_TEST("server.file.ShareStore:manage", a)
{
    ShareStore testee;

    // Add things
    testee.addShare("p1", "u1");
    testee.addShare("p2", "u2");
    testee.addShare("p1", "u1a"); // no-op
    testee.addShare("p3", "u3");

    // Verify
    a.checkEqual("01. getNumShares", testee.getNumShares(), 3U);
    a.checkEqual("02a. path", testee.getSharePathName(testee.getShareByIndex(0)), "p1");
    a.checkEqual("02b. user", testee.getShareOwner(testee.getShareByIndex(0)), "u1");
    a.checkEqual("02c. seq#", testee.getShareSequenceNumber(testee.getShareByIndex(0)), 1);
    a.checkEqual("03a. path", testee.getSharePathName(testee.getShareByIndex(1)), "p2");
    a.checkEqual("03b. user", testee.getShareOwner(testee.getShareByIndex(1)), "u2");
    a.checkEqual("03c. seq#", testee.getShareSequenceNumber(testee.getShareByIndex(1)), 2);
    a.checkEqual("04a. path", testee.getSharePathName(testee.getShareByIndex(2)), "p3");
    a.checkEqual("04b. user", testee.getShareOwner(testee.getShareByIndex(2)), "u3");
    a.checkEqual("04c. seq#", testee.getShareSequenceNumber(testee.getShareByIndex(2)), 3);
    a.checkEqual("05. seq#", testee.getSequenceNumber(), 3);

    // Remove one
    testee.removeShare("p2");
    testee.removeShare("p4"); // no-op

    // Verify
    a.checkEqual("11. getNumShares", testee.getNumShares(), 2U);
    a.checkEqual("12a. path", testee.getSharePathName(testee.getShareByIndex(0)), "p1");
    a.checkEqual("12b. user", testee.getShareOwner(testee.getShareByIndex(0)), "u1");
    a.checkEqual("12c. seq#", testee.getShareSequenceNumber(testee.getShareByIndex(0)), 1);
    a.checkEqual("13a. path", testee.getSharePathName(testee.getShareByIndex(1)), "p3");
    a.checkEqual("13b. user", testee.getShareOwner(testee.getShareByIndex(1)), "u3");
    a.checkEqual("13c. seq#", testee.getShareSequenceNumber(testee.getShareByIndex(1)), 3);
    a.checkEqual("14. seq#", testee.getSequenceNumber(), 3);

    // Out of range
    a.checkEqual("21a. path", testee.getSharePathName(testee.getShareByIndex(2)), "");
    a.checkEqual("21b. user", testee.getShareOwner(testee.getShareByIndex(2)), "");
    a.checkEqual("21c. seq#", testee.getShareSequenceNumber(testee.getShareByIndex(2)), 0);
}

/** Test loading, standard case. */
AFL_TEST("server.file.ShareStore:load", a)
{
    TestHarness h;

    h.dir.subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("_share"))
        ->files.pushBackNew(new InternalDirectoryHandler::File("u900"))
        ->content.append(afl::string::toBytes("SEQ=77\nENTRY=u/u700/dir\nEOWNER=u700\nESEQ=55\n"));

    ShareStore testee;
    testee.load("u900", h.root);

    a.checkEqual("01. seq#", testee.getSequenceNumber(), 77);
    a.checkEqual("02. num", testee.getNumShares(), 1U);
    a.checkEqual("03a. path", testee.getSharePathName(testee.getShareByIndex(0)), "u/u700/dir");
    a.checkEqual("03b. user", testee.getShareOwner(testee.getShareByIndex(0)), "u700");
    a.checkEqual("03c. seq#", testee.getShareSequenceNumber(testee.getShareByIndex(0)), 55);
}

/** Test loading, directory does not exist. */
AFL_TEST("server.file.ShareStore:load:no-dir", a)
{
    TestHarness h;

    ShareStore testee;
    testee.load("u900", h.root);

    a.checkEqual("01. seq#", testee.getSequenceNumber(), 0);
    a.checkEqual("02. num", testee.getNumShares(), 0U);
}

/** Test loading, file does not exist. */
AFL_TEST("server.file.ShareStore:load:no-file", a)
{
    TestHarness h;

    h.dir.subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("_share"));

    ShareStore testee;
    testee.load("u900", h.root);

    a.checkEqual("01. seq#", testee.getSequenceNumber(), 0);
    a.checkEqual("02. num", testee.getNumShares(), 0U);
}

/** Test loading, directory is file. */
AFL_TEST("server.file.ShareStore:load:error:isfile", a)
{
    TestHarness h;

    h.dir.files.pushBackNew(new InternalDirectoryHandler::File("_share"));

    ShareStore testee;
    testee.load("u900", h.root);

    a.checkEqual("01. seq#", testee.getSequenceNumber(), 0);
    a.checkEqual("02. num", testee.getNumShares(), 0U);
}

/** Test loading, file is a directory. */
AFL_TEST("server.file.ShareStore:load:error:isdir", a)
{
    TestHarness h;

    h.dir.subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("_share"))
        ->subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("u900"));

    ShareStore testee;
    testee.load("u900", h.root);

    a.checkEqual("01. seq#", testee.getSequenceNumber(), 0);
    a.checkEqual("02. num", testee.getNumShares(), 0U);
}

/** Test loading, invalid file content: invalid key. */
AFL_TEST("server.file.ShareStore:load:error:bad-content:key", a)
{
    TestHarness h;

    h.dir.subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("_share"))
        ->files.pushBackNew(new InternalDirectoryHandler::File("u900"))
        ->content.append(afl::string::toBytes("EGAL=88\n"));

    ShareStore testee;
    AFL_CHECK_SUCCEEDS(a, testee.load("u900", h.root));
}

/** Test loading, invalid file content: invalid SEQ. */
AFL_TEST("server.file.ShareStore:load:error:bad-content:seq-value", a)
{
    TestHarness h;

    h.dir.subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("_share"))
        ->files.pushBackNew(new InternalDirectoryHandler::File("u900"))
        ->content.append(afl::string::toBytes("SEQ=99zz\n"));

    ShareStore testee;
    AFL_CHECK_SUCCEEDS(a, testee.load("u900", h.root));
    a.checkEqual("01. seq", testee.getSequenceNumber(), 0);
}

/** Test loading, invalid file content: invalid ESEQ. */
AFL_TEST("server.file.ShareStore:load:error:bad-content:eseq-value", a)
{
    TestHarness h;

    h.dir.subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("_share"))
        ->files.pushBackNew(new InternalDirectoryHandler::File("u900"))
        ->content.append(afl::string::toBytes("SEQ=77\nENTRY=u/u700/dir\nEOWNER=u700\nESEQ=55qq\n"));

    ShareStore testee;
    testee.load("u900", h.root);

    a.checkEqual("01. seq#", testee.getSequenceNumber(), 77);
    a.checkEqual("02. num", testee.getNumShares(), 1U);
    a.checkEqual("03a. path", testee.getSharePathName(testee.getShareByIndex(0)), "u/u700/dir");
    a.checkEqual("03b. user", testee.getShareOwner(testee.getShareByIndex(0)), "u700");
    a.checkEqual("03c. seq#", testee.getShareSequenceNumber(testee.getShareByIndex(0)), 0);      // default because value is invalid
}

/** Test loading, invalid file content: EOWNER without ENTRY. */
AFL_TEST("server.file.ShareStore:load:error:bad-content:lone-eowner", a)
{
    TestHarness h;

    h.dir.subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("_share"))
        ->files.pushBackNew(new InternalDirectoryHandler::File("u900"))
        ->content.append(afl::string::toBytes("SEQ=77\nEOWNER=u700\n"));

    ShareStore testee;
    testee.load("u900", h.root);

    a.checkEqual("01. seq#", testee.getSequenceNumber(), 77);
    a.checkEqual("02. num", testee.getNumShares(), 0U);
}

/** Test loading, invalid file content: ESEQ without ENTRY. */
AFL_TEST("server.file.ShareStore:load:error:bad-content:lone-eseq", a)
{
    TestHarness h;

    h.dir.subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("_share"))
        ->files.pushBackNew(new InternalDirectoryHandler::File("u900"))
        ->content.append(afl::string::toBytes("SEQ=77\nESEQ=55\n"));

    ShareStore testee;
    testee.load("u900", h.root);

    a.checkEqual("01. seq#", testee.getSequenceNumber(), 77);
    a.checkEqual("02. num", testee.getNumShares(), 0U);
}

/** Test saving, normal case. */
AFL_TEST("server.file.ShareStore:save", a)
{
    TestHarness h;
    InternalDirectoryHandler::Directory* shareDir = h.dir.subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("_share"));

    ShareStore testee;
    testee.addShare("p3", "u3");
    testee.save("u333", h.root);

    a.checkEqual("01. file exists", shareDir->files.size(), 1U);
    a.checkEqual("02. file name", shareDir->files[0]->name, "u333");
    a.checkEqual("03. file content", afl::string::fromBytes(shareDir->files[0]->content), "SEQ=1\nENTRY=p3\nEOWNER=u3\nESEQ=1\n");
}

/** Test saving, directory does not exist. */
AFL_TEST("server.file.ShareStore:save:create-dir", a)
{
    TestHarness h;

    ShareStore testee;
    testee.addShare("p3", "u3");
    testee.save("u333", h.root);

    a.checkEqual("01. dir exists", h.dir.subdirectories.size(), 1U);
    a.checkEqual("02. dir name", h.dir.subdirectories[0]->name, "_share");
    InternalDirectoryHandler::Directory* shareDir = h.dir.subdirectories[0];

    a.checkEqual("11. file exists", shareDir->files.size(), 1U);
    a.checkEqual("12. file name", shareDir->files[0]->name, "u333");
    a.checkEqual("13. file content", afl::string::fromBytes(shareDir->files[0]->content), "SEQ=1\nENTRY=p3\nEOWNER=u3\nESEQ=1\n");
}

/** Test saving, directory in the way. */
AFL_TEST("server.file.ShareStore:save:error:dir", a)
{
    TestHarness h;
    h.dir.subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("_share"))
        ->subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("u333"));

    ShareStore testee;
    testee.addShare("p3", "u3");
    AFL_CHECK_SUCCEEDS(a, testee.save("u333", h.root));
}

/** Test saving, file in the way. */
AFL_TEST("server.file.ShareStore:save:error:file", a)
{
    TestHarness h;
    h.dir.files.pushBackNew(new InternalDirectoryHandler::File("_share"));

    ShareStore testee;
    testee.addShare("p3", "u3");
    AFL_CHECK_SUCCEEDS(a, testee.save("u333", h.root));
}

/** Test saveIfModified. */
AFL_TEST("server.file.ShareStore:saveIfModified", a)
{
    TestHarness h;
    InternalDirectoryHandler::Directory* shareDir = h.dir.subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("_share"));
    shareDir->files.pushBackNew(new InternalDirectoryHandler::File("u111"))
        ->content.append(afl::string::toBytes("SEQ=77\nENTRY=u/u700/dir\nEOWNER=u700\nESEQ=55qq\n"));

    ShareStore testee;
    testee.load("u111", h.root);
    testee.saveIfModified("u222", h.root);
    testee.addShare("p", "up");
    testee.saveIfModified("u333", h.root);

    a.checkEqual("01. file exists", shareDir->files.size(), 2U);
    a.checkEqual("02. file name 1", shareDir->files[0]->name, "u111");
    a.checkEqual("03. file name 2", shareDir->files[1]->name, "u333");
}

namespace {
    void testUserId(afl::test::Assert a, String_t userId, bool expectFile)
    {
        TestHarness h;
        InternalDirectoryHandler::Directory* shareDir = h.dir.subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("_share"));

        ShareStore testee;
        testee.addShare("a", "b");
        testee.save(userId, h.root);

        if (expectFile) {
            a(userId).check("must create a file", !shareDir->files.empty());
        } else {
            a(userId).check("must not create a file", shareDir->files.empty());
        }
    }
}

AFL_TEST("server.file.ShareStore:user-id-validation", a)
{
    // Valid user Ids
    testUserId(a, "u",    true);
    testUserId(a, "100",  true);
    testUserId(a, "u100", true);
    testUserId(a, "a-b",  true);
    testUserId(a, "a_b",  true);
    testUserId(a, "Ax",   true);

    // Invalid user Ids
    testUserId(a, "",                false);
    testUserId(a, "*",               false);
    testUserId(a, " ",               false);
    testUserId(a, String_t(1, '\0'), false);
    testUserId(a, "/",               false);
    testUserId(a, "a\\b",            false);
    testUserId(a, ".x",              false);
    testUserId(a, "..",              false);
    testUserId(a, "a.mp3",           false);
}
