/**
  *  \file u/t_server_interface_filebaseclient.cpp
  *  \brief Test for server::interface::FileBaseClient
  */

#include "server/interface/filebaseclient.hpp"

#include <memory>
#include "t_server_interface.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "server/types.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;
using afl::data::Segment;

/** Test it. */
void
TestServerInterfaceFileBaseClient::testIt()
{
    using server::interface::FileBase;

    afl::test::CommandHandler mock("testIt");
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

        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0], 1);
        TS_ASSERT_EQUALS(result[1], 0);
        TS_ASSERT_EQUALS(result[2], 1);
    }

    // getFile
    mock.expectCall("GET, a/b/c");
    mock.provideNewResult(server::makeStringValue("...content..."));
    TS_ASSERT_EQUALS(testee.getFile("a/b/c"), "...content...");

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
        TS_ASSERT_EQUALS(result.size(), 3U);

        TS_ASSERT(result["f.txt"] != 0);
        TS_ASSERT_EQUALS(result["f.txt"]->type, FileBase::IsFile);
        TS_ASSERT_EQUALS(result["f.txt"]->size.orElse(99), 504);
        TS_ASSERT_EQUALS(result["f.txt"]->visibility.isValid(), false);

        TS_ASSERT(result["sub"] != 0);
        TS_ASSERT_EQUALS(result["sub"]->type, FileBase::IsDirectory);
        TS_ASSERT_EQUALS(result["sub"]->size.isValid(), false);
        TS_ASSERT_EQUALS(result["sub"]->visibility.orElse(99), 2);

        TS_ASSERT(result["ufo"] != 0);
        TS_ASSERT_EQUALS(result["ufo"]->type, FileBase::IsUnknown);
        TS_ASSERT_EQUALS(result["ufo"]->size.isValid(), false);
        TS_ASSERT_EQUALS(result["ufo"]->visibility.isValid(), false);
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

        TS_ASSERT_EQUALS(owner, "1001");
        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT_EQUALS(result[0].userId, "*");
        TS_ASSERT_EQUALS(result[0].permission, "0");
        TS_ASSERT_EQUALS(result[1].userId, "1002");
        TS_ASSERT_EQUALS(result[1].permission, "r");
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
        TS_ASSERT_EQUALS(server::toString(p.get()), "Dir Name");
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
        TS_ASSERT_EQUALS(out.type, FileBase::IsFile);
        TS_ASSERT_EQUALS(out.size.orElse(99), 999);
        TS_ASSERT_EQUALS(out.visibility.isValid(), false);
        TS_ASSERT_EQUALS(out.contentId.orElse(""), "55ca6286e3e4f4fba5d0448333fa99fc5a404a73");
    }

    // getDiskUsage
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("files", server::makeIntegerValue(1075));
        in->setNew("kbytes", server::makeIntegerValue(13427));

        mock.expectCall("USAGE, u");
        mock.provideNewResult(new HashValue(in));

        FileBase::Usage out = testee.getDiskUsage("u");
        TS_ASSERT_EQUALS(out.numItems, 1075);
        TS_ASSERT_EQUALS(out.totalKBytes, 13427);
    }

    mock.checkFinish();
}

