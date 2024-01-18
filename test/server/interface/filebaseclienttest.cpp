/**
  *  \file test/server/interface/filebaseclienttest.cpp
  *  \brief Test for server::interface::FileBaseClient
  */

#include "server/interface/filebaseclient.hpp"

#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"
#include <memory>

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;
using afl::data::Segment;

/** Test it. */
AFL_TEST("server.interface.FileBaseClient", a)
{
    using server::interface::FileBase;

    afl::test::CommandHandler mock(a);
    server::interface::FileBaseClient testee(mock);

    // copyFile
    mock.expectCall("CP, a/from, b/to");
    mock.provideNewResult(0);
    testee.copyFile("a/from", "b/to");

    // forgetDirectory
    mock.expectCall("FORGET, a/b");
    mock.provideNewResult(0);
    testee.forgetDirectory("a/b");

    // testFiles
    {
        mock.expectCall("FTEST, f1, f2, ff");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackInteger(1).pushBackInteger(0).pushBackInteger(1))));

        String_t fileNames[] = { "f1", "f2", "ff" };
        afl::data::IntegerList_t result;
        testee.testFiles(fileNames, result);

        a.checkEqual("01. size", result.size(), 3U);
        a.checkEqual("02. result", result[0], 1);
        a.checkEqual("03. result", result[1], 0);
        a.checkEqual("04. result", result[2], 1);
    }

    // getFile
    mock.expectCall("GET, a/b/c");
    mock.provideNewResult(server::makeStringValue("...content..."));
    a.checkEqual("11. getFile", testee.getFile("a/b/c"), "...content...");

    // getDirectoryContent
    {
        // Input data: 3 items in a folder
        Vector::Ref_t in = Vector::create();

        Hash::Ref_t file = Hash::create();
        file->setNew("type", server::makeStringValue("file"));
        file->setNew("size", server::makeIntegerValue(504));
        in->pushBackString("f.txt");
        in->pushBackNew(new HashValue(file));

        Hash::Ref_t dir = Hash::create();
        dir->setNew("type", server::makeStringValue("dir"));
        dir->setNew("visibility", server::makeIntegerValue(2));
        in->pushBackString("sub");
        in->pushBackNew(new HashValue(dir));

        Hash::Ref_t ufo = Hash::create();
        ufo->setNew("type", server::makeStringValue("ufo"));
        in->pushBackString("ufo");
        in->pushBackNew(new HashValue(ufo));

        // Test
        mock.expectCall("LS, a");
        mock.provideNewResult(new VectorValue(in));
        afl::container::PtrMap<String_t, FileBase::Info> result;
        testee.getDirectoryContent("a", result);

        // Verify output data
        a.checkEqual("21. size", result.size(), 3U);

        a.checkNonNull("31. f.txt",    result["f.txt"]);
        a.checkEqual("32. type",       result["f.txt"]->type, FileBase::IsFile);
        a.checkEqual("33. size",       result["f.txt"]->size.orElse(99), 504);
        a.checkEqual("34. visibility", result["f.txt"]->visibility.isValid(), false);

        a.checkNonNull("41. sub",      result["sub"]);
        a.checkEqual("42. type",       result["sub"]->type, FileBase::IsDirectory);
        a.checkEqual("43. size",       result["sub"]->size.isValid(), false);
        a.checkEqual("44. visibility", result["sub"]->visibility.orElse(99), 2);

        a.checkNonNull("51. sub",      result["ufo"]);
        a.checkEqual("52. type",       result["ufo"]->type, FileBase::IsUnknown);
        a.checkEqual("53. size",       result["ufo"]->size.isValid(), false);
        a.checkEqual("54. visibility", result["ufo"]->visibility.isValid(), false);
    }

    // getDirectoryPermission
    {
        // Input data: 2 permission entries
        Hash::Ref_t perm1 = Hash::create();
        perm1->setNew("user", server::makeStringValue("*"));
        perm1->setNew("perms", server::makeStringValue("0"));

        Hash::Ref_t perm2 = Hash::create();
        perm2->setNew("user", server::makeStringValue("1002"));
        perm2->setNew("perms", server::makeStringValue("r"));

        // Complete result
        Hash::Ref_t in = Hash::create();
        in->setNew("owner", server::makeStringValue("1001"));
        in->setNew("perms", new VectorValue(Vector::create(Segment().pushBackNew(new HashValue(perm1)).pushBackNew(new HashValue(perm2)))));

        // Do it
        mock.expectCall("LSPERM, u/foo");
        mock.provideNewResult(new HashValue(in));

        std::vector<FileBase::Permission> result;
        String_t owner;
        testee.getDirectoryPermission("u/foo", owner, result);

        a.checkEqual("61. owner",      owner, "1001");
        a.checkEqual("62. size",       result.size(), 2U);
        a.checkEqual("63. userId",     result[0].userId, "*");
        a.checkEqual("64. permission", result[0].permission, "0");
        a.checkEqual("65. userId",     result[1].userId, "1002");
        a.checkEqual("66. permission", result[1].permission, "r");
    }

    // createDirectory etc.
    mock.expectCall("MKDIR, a/dir");
    mock.provideNewResult(0);
    testee.createDirectory("a/dir");

    mock.expectCall("MKDIRHIER, a/b/c/d/e/f");
    mock.provideNewResult(0);
    testee.createDirectoryTree("a/b/c/d/e/f");

    mock.expectCall("MKDIRAS, u/user, 1030");
    mock.provideNewResult(0);
    testee.createDirectoryAsUser("u/user", "1030");

    // getDirectoryProperty
    {
        mock.expectCall("PROPGET, dir, name");
        mock.provideNewResult(server::makeStringValue("Dir Name"));

        std::auto_ptr<afl::data::Value> p(testee.getDirectoryProperty("dir", "name"));
        a.checkEqual("71. result", server::toString(p.get()), "Dir Name");
    }

    // setDirectoryProperty
    mock.expectCall("PROPSET, dir, name, New Name");
    mock.provideNewResult(0);
    testee.setDirectoryProperty("dir", "name", "New Name");

    // putFile
    mock.expectCall("PUT, file, content...");
    mock.provideNewResult(0);
    testee.putFile("file", "content...");

    // remove
    mock.expectCall("RM, a/file");
    mock.provideNewResult(0);
    testee.removeFile("a/file");

    mock.expectCall("RMDIR, a/d");
    mock.provideNewResult(0);
    testee.removeDirectory("a/d");

    // setDirectoryPermissions
    mock.expectCall("SETPERM, dir, 1050, rw");
    mock.provideNewResult(0);
    testee.setDirectoryPermissions("dir", "1050", "rw");

    // getFileInformation
    {
        Hash::Ref_t file = Hash::create();
        file->setNew("type", server::makeStringValue("file"));
        file->setNew("size", server::makeIntegerValue(999));
        file->setNew("id", server::makeStringValue("55ca6286e3e4f4fba5d0448333fa99fc5a404a73"));

        mock.expectCall("STAT, a/x/file.bin");
        mock.provideNewResult(new HashValue(file));

        FileBase::Info out = testee.getFileInformation("a/x/file.bin");
        a.checkEqual("81. type",       out.type, FileBase::IsFile);
        a.checkEqual("82. size",       out.size.orElse(99), 999);
        a.checkEqual("83. visibility", out.visibility.isValid(), false);
        a.checkEqual("84. contentId",  out.contentId.orElse(""), "55ca6286e3e4f4fba5d0448333fa99fc5a404a73");
    }

    // getDiskUsage
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("files", server::makeIntegerValue(1075));
        in->setNew("kbytes", server::makeIntegerValue(13427));

        mock.expectCall("USAGE, u");
        mock.provideNewResult(new HashValue(in));

        FileBase::Usage out = testee.getDiskUsage("u");
        a.checkEqual("91. numItems",    out.numItems, 1075);
        a.checkEqual("92. totalKBytes", out.totalKBytes, 13427);
    }

    mock.checkFinish();
}
