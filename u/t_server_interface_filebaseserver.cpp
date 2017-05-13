/**
  *  \file u/t_server_interface_filebaseserver.cpp
  *  \brief Test for server::interface::FileBaseServer
  */

#include <stdexcept>
#include "server/interface/filebaseserver.hpp"

#include "t_server_interface.hpp"
#include "afl/string/format.hpp"
#include "u/helper/callreceiver.hpp"
#include "afl/data/access.hpp"
#include "server/types.hpp"
#include "server/interface/filebaseclient.hpp"

using afl::string::Format;
using afl::data::Segment;
using afl::data::Access;
using server::interface::FileBase;

namespace {
    class FileBaseMock : public FileBase, public CallReceiver {
     public:
        virtual void copyFile(String_t sourceFile, String_t destFile)
            { checkCall(Format("copyFile(%s,%s)", sourceFile, destFile)); }
        virtual void forgetDirectory(String_t dirName)
            { checkCall(Format("forgetDirectory(%s)", dirName)); }
        virtual void testFiles(afl::base::Memory<const String_t> fileNames, afl::data::IntegerList_t& resultFlags)
            {
                String_t cmd = "testFiles(";
                while (const String_t* p = fileNames.eat()) {
                    cmd += *p;
                    if (!fileNames.empty()) {
                        cmd += ",";
                    }
                    resultFlags.push_back(consumeReturnValue<int32_t>());
                }
                cmd += ")";
                checkCall(cmd);
            }
        virtual String_t getFile(String_t fileName)
            {
                checkCall(Format("getFile(%s)", fileName));
                return consumeReturnValue<String_t>();
            }
        virtual void getDirectoryContent(String_t dirName, ContentInfoMap_t& result)
            {
                // always produces two string/Info
                checkCall(Format("getDirectoryContent(%s)", dirName));

                String_t a = consumeReturnValue<String_t>();
                result.insertNew(a, new Info(consumeReturnValue<Info>()));

                a = consumeReturnValue<String_t>();
                result.insertNew(a, new Info(consumeReturnValue<Info>()));
            }
        virtual void getDirectoryPermission(String_t dirName, String_t& ownerUserId, std::vector<Permission>& result)
            {
                // always produces one string, one Permission
                checkCall(Format("getDirectoryPermission(%s)", dirName));
                ownerUserId = consumeReturnValue<String_t>();
                result.push_back(consumeReturnValue<Permission>());
            }
        virtual void createDirectory(String_t dirName)
            { checkCall(Format("createDirectory(%s)", dirName)); }
        virtual void createDirectoryTree(String_t dirName)
            { checkCall(Format("createDirectoryTree(%s)", dirName)); }
        virtual void createDirectoryAsUser(String_t dirName, String_t userId)
            { checkCall(Format("createDirectoryAsUser(%s,%s)", dirName, userId)); }
        virtual afl::data::Value* getDirectoryProperty(String_t dirName, String_t propName)
            {
                checkCall(Format("getDirectoryProperty(%s,%s)", dirName, propName));
                return consumeReturnValue<afl::data::Value*>();
            }
        virtual void setDirectoryProperty(String_t dirName, String_t propName, String_t propValue)
            { checkCall(Format("setDirectoryProperty(%s,%s,%s)", dirName, propName, propValue)); }
        virtual void putFile(String_t fileName, String_t content)
            { checkCall(Format("putFile(%s,%s)", fileName, content)); }
        virtual void removeFile(String_t fileName)
            { checkCall(Format("removeFile(%s)", fileName)); }
        virtual void removeDirectory(String_t dirName)
            { checkCall(Format("removeDirectory(%s)", dirName)); }
        virtual void setDirectoryPermissions(String_t dirName, String_t userId, String_t permission)
            { checkCall(Format("setDirectoryPermissions(%s,%s,%s)", dirName, userId, permission)); }
        virtual Info getFileInformation(String_t fileName)
            {
                checkCall(Format("getFileInformation(%s)", fileName));
                return consumeReturnValue<Info>();
            }
        virtual Usage getDiskUsage(String_t dirName)
            {
                checkCall(Format("getDiskUsage(%s)", dirName));
                return consumeReturnValue<Usage>();
            }

    };
}

/** Main test. */
void
TestServerInterfaceFileBaseServer::testIt()
{
    FileBaseMock mock;
    server::interface::FileBaseServer testee(mock);

    // copyFile
    mock.expectCall("copyFile(a,b)");
    testee.callVoid(Segment().pushBackString("CP").pushBackString("a").pushBackString("b"));

    // forgetDirectory
    mock.expectCall("forgetDirectory(f)");
    testee.callVoid(Segment().pushBackString("FORGET").pushBackString("f"));

    // testFiles
    {
        mock.expectCall("testFiles()");
        std::auto_ptr<afl::data::Value> v(testee.call(Segment().pushBackString("FTEST")));
        TS_ASSERT_EQUALS(Access(v).getArraySize(), 0U);
    }
    {
        mock.expectCall("testFiles(x,y,z)");
        mock.provideReturnValue<int32_t>(0);
        mock.provideReturnValue<int32_t>(1);
        mock.provideReturnValue<int32_t>(1);
        std::auto_ptr<afl::data::Value> v(testee.call(Segment().pushBackString("FTEST").pushBackString("x").pushBackString("y").pushBackString("z")));

        TS_ASSERT_EQUALS(Access(v).getArraySize(), 3U);
        TS_ASSERT_EQUALS(Access(v)[0].toInteger(), 0);
        TS_ASSERT_EQUALS(Access(v)[1].toInteger(), 1);
        TS_ASSERT_EQUALS(Access(v)[2].toInteger(), 1);
    }

    // getFile
    mock.expectCall("getFile(autoexec.bat)");
    mock.provideReturnValue<String_t>("@echo off");
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("GET").pushBackString("autoexec.bat")), "@echo off");

    // getDirectoryContent
    {
        mock.expectCall("getDirectoryContent(a/dir)");
        {
            FileBase::Info i1;
            i1.type = FileBase::IsDirectory;
            i1.visibility = 1;
            mock.provideReturnValue<String_t>("f1");
            mock.provideReturnValue<FileBase::Info>(i1);
        }
        {
            FileBase::Info i2;
            i2.type = FileBase::IsFile;
            i2.size = 10204;
            i2.contentId = "55ca6286e3e4f4fba5d0448333fa99fc5a404a73";
            mock.provideReturnValue<String_t>("f2");
            mock.provideReturnValue<FileBase::Info>(i2);
        }

        std::auto_ptr<afl::data::Value> v(testee.call(Segment().pushBackString("LS").pushBackString("a/dir")));

        TS_ASSERT_EQUALS(Access(v).getArraySize(), 4U);
        TS_ASSERT_EQUALS(Access(v)[0].toString(), "f1");
        TS_ASSERT_EQUALS(Access(v)[1]("type").toString(), "dir");
        TS_ASSERT_EQUALS(Access(v)[1]("visibility").toInteger(), 1);
        TS_ASSERT_EQUALS(Access(v)[2].toString(), "f2");
        TS_ASSERT_EQUALS(Access(v)[3]("type").toString(), "file");
        TS_ASSERT_EQUALS(Access(v)[3]("size").toInteger(), 10204);
        TS_ASSERT_EQUALS(Access(v)[3]("id").toString(), "55ca6286e3e4f4fba5d0448333fa99fc5a404a73");
    }

    // getDirectoryPermission
    {
        mock.expectCall("getDirectoryPermission(a/b)");
        mock.provideReturnValue<String_t>("1092");
        mock.provideReturnValue(FileBase::Permission("1030", "w"));

        std::auto_ptr<afl::data::Value> v(testee.call(Segment().pushBackString("LSPERM").pushBackString("a/b")));

        TS_ASSERT_EQUALS(Access(v)("owner").toString(), "1092");
        TS_ASSERT_EQUALS(Access(v)("perms").getArraySize(), 1U);
        TS_ASSERT_EQUALS(Access(v)("perms")[0]("user").toString(), "1030");
        TS_ASSERT_EQUALS(Access(v)("perms")[0]("perms").toString(), "w");
    }

    // createDirectory etc
    mock.expectCall("createDirectory(newdir1)");
    testee.callVoid(Segment().pushBackString("MKDIR").pushBackString("newdir1"));

    mock.expectCall("createDirectoryTree(newdir2/sub3)");
    testee.callVoid(Segment().pushBackString("MKDIRHIER").pushBackString("newdir2/sub3"));

    mock.expectCall("createDirectoryAsUser(newdir3,1203)");
    testee.callVoid(Segment().pushBackString("MKDIRAS").pushBackString("newdir3").pushBackString("1203"));

    // getDirectoryProperty
    mock.expectCall("getDirectoryProperty(d,p)");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("PROPGET").pushBackString("d").pushBackString("p")), 9);

    mock.expectCall("getDirectoryProperty(d,q)");
    mock.provideReturnValue<afl::data::Value*>(server::makeStringValue("rr"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("PROPGET").pushBackString("d").pushBackString("q")), "rr");

    // setDirectoryProperty
    mock.expectCall("setDirectoryProperty(dd,pp,vv)");
    testee.callVoid(Segment().pushBackString("PROPSET").pushBackString("dd").pushBackString("pp").pushBackString("vv"));

    // putFile
    mock.expectCall("putFile(ff.txt,content)");
    testee.callVoid(Segment().pushBackString("PUT").pushBackString("ff.txt").pushBackString("content"));

    // removeFile etc
    mock.expectCall("removeFile(file-be-gone)");
    testee.callVoid(Segment().pushBackString("RM").pushBackString("file-be-gone"));

    mock.expectCall("removeDirectory(dir-be-gone)");
    testee.callVoid(Segment().pushBackString("RMDIR").pushBackString("dir-be-gone"));

    // setDirectoryPermissions
    mock.expectCall("setDirectoryPermissions(u/1/dir,2,r)");
    testee.callVoid(Segment().pushBackString("SETPERM").pushBackString("u/1/dir").pushBackString("2").pushBackString("r"));

    // getFileInformation
    {
        FileBase::Info in;
        in.type = FileBase::IsDirectory;
        in.visibility = 2;
        mock.expectCall("getFileInformation(what)");
        mock.provideReturnValue(in);

        std::auto_ptr<afl::data::Value> v(testee.call(Segment().pushBackString("STAT").pushBackString("what")));

        TS_ASSERT_EQUALS(Access(v)("type").toString(), "dir");
        TS_ASSERT_EQUALS(Access(v)("visibility").toInteger(), 2);
    }

    // getDiskUsage
    {
        FileBase::Usage in;
        in.numItems = 9300;
        in.totalKBytes = 2348;
        mock.expectCall("getDiskUsage(u)");
        mock.provideReturnValue(in);

        std::auto_ptr<afl::data::Value> v(testee.call(Segment().pushBackString("USAGE").pushBackString("u")));

        TS_ASSERT_EQUALS(Access(v)("files").toInteger(), 9300);
        TS_ASSERT_EQUALS(Access(v)("kbytes").toInteger(), 2348);
    }

    // Variations
    // (Not much to do. The only variation we have is the keyword case.)
    mock.expectCall("setDirectoryProperty(dd,pp,vv)");
    testee.callVoid(Segment().pushBackString("propset").pushBackString("dd").pushBackString("pp").pushBackString("vv"));

    mock.checkFinish();
}

/** Test error cases. */
void
TestServerInterfaceFileBaseServer::testErrors()
{
    FileBaseMock mock;
    server::interface::FileBaseServer testee(mock);

    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    TS_ASSERT_THROWS(testee.call(empty), std::exception);
    TS_ASSERT_THROWS(testee.call(Segment().pushBackString("wut")), std::exception);
    TS_ASSERT_THROWS(testee.call(Segment().pushBackString("PUT")), std::exception);
    TS_ASSERT_THROWS(testee.call(Segment().pushBackString("PUT").pushBackString("x")), std::exception);
    TS_ASSERT_THROWS(testee.call(Segment().pushBackString("PUT").pushBackString("x").pushBackString("x").pushBackString("x")), std::exception);

    // ComposableCommandHandler personality
    interpreter::Arguments args(empty, 0, 0);
    std::auto_ptr<afl::data::Value> p;
    TS_ASSERT_EQUALS(testee.handleCommand("huhu", args, p), false);

    mock.checkFinish();
}

/** Test roundtrip. */
void
TestServerInterfaceFileBaseServer::testRoundtrip()
{
    FileBaseMock mock;
    server::interface::FileBaseServer level1(mock);
    server::interface::FileBaseClient level2(level1);
    server::interface::FileBaseClient level3(level2);
    server::interface::FileBaseClient level4(level3);

    // copyFile
    mock.expectCall("copyFile(a,b)");
    level4.copyFile("a", "b");

    // forgetDirectory
    mock.expectCall("forgetDirectory(f)");
    level4.forgetDirectory("f");

    // testFiles
    {
        mock.expectCall("testFiles()");

        afl::data::IntegerList_t result;
        level4.testFiles(afl::base::Nothing, result);
        TS_ASSERT_EQUALS(result.size(), 0U);
    }
    {
        const String_t files[] = { "fx", "fy", "fz" };
        mock.expectCall("testFiles(fx,fy,fz)");
        mock.provideReturnValue<int32_t>(0);
        mock.provideReturnValue<int32_t>(1);
        mock.provideReturnValue<int32_t>(1);

        afl::data::IntegerList_t result;
        level4.testFiles(files, result);

        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0], 0);
        TS_ASSERT_EQUALS(result[1], 1);
        TS_ASSERT_EQUALS(result[2], 1);
    }

    // getFile
    mock.expectCall("getFile(config.sys)");
    mock.provideReturnValue<String_t>("files=30");
    TS_ASSERT_EQUALS(level4.getFile("config.sys"), "files=30");

    // getDirectoryContent
    {
        mock.expectCall("getDirectoryContent(a/dir)");
        {
            FileBase::Info i1;
            i1.type = FileBase::IsDirectory;
            i1.visibility = 1;
            mock.provideReturnValue<String_t>("dir");
            mock.provideReturnValue<FileBase::Info>(i1);
        }
        {
            FileBase::Info i2;
            i2.type = FileBase::IsFile;
            i2.size = 10204;
            mock.provideReturnValue<String_t>("file");
            mock.provideReturnValue<FileBase::Info>(i2);
        }

        FileBase::ContentInfoMap_t result;
        level4.getDirectoryContent("a/dir", result);

        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT(result["dir"] != 0);
        TS_ASSERT(result["file"] != 0);
        TS_ASSERT_EQUALS(result["dir"]->type, FileBase::IsDirectory);
        TS_ASSERT_EQUALS(result["dir"]->visibility.orElse(99), 1);
        TS_ASSERT_EQUALS(result["file"]->type, FileBase::IsFile);
        TS_ASSERT_EQUALS(result["file"]->size.orElse(99), 10204);
    }

    // getDirectoryPermission
    {
        mock.expectCall("getDirectoryPermission(a/b)");
        mock.provideReturnValue<String_t>("1091");
        mock.provideReturnValue(FileBase::Permission("1130", "w"));

        String_t owner;
        std::vector<FileBase::Permission> perm;
        level4.getDirectoryPermission("a/b", owner, perm);

        TS_ASSERT_EQUALS(owner, "1091");
        TS_ASSERT_EQUALS(perm.size(), 1U);
        TS_ASSERT_EQUALS(perm[0].userId, "1130");
        TS_ASSERT_EQUALS(perm[0].permission, "w");
    }

    // createDirectory etc
    mock.expectCall("createDirectory(newdir1)");
    level4.createDirectory("newdir1");

    mock.expectCall("createDirectoryTree(newdir2/sub3)");
    level4.createDirectoryTree("newdir2/sub3");

    mock.expectCall("createDirectoryAsUser(newdir3,1203)");
    level4.createDirectoryAsUser("newdir3", "1203");

    // getDirectoryProperty
    mock.expectCall("getDirectoryProperty(d,p)");
    mock.provideReturnValue<afl::data::Value*>(server::makeIntegerValue(9));
    TS_ASSERT_EQUALS(level4.getDirectoryIntegerProperty("d", "p"), 9);

    mock.expectCall("getDirectoryProperty(d,q)");
    mock.provideReturnValue<afl::data::Value*>(server::makeStringValue("rr"));
    TS_ASSERT_EQUALS(level4.getDirectoryStringProperty("d", "q"), "rr");

    // setDirectoryProperty
    mock.expectCall("setDirectoryProperty(dd,pp,vv)");
    level4.setDirectoryProperty("dd", "pp", "vv");

    // putFile
    mock.expectCall("putFile(ff.txt,content)");
    level4.putFile("ff.txt", "content");

    // removeFile etc
    mock.expectCall("removeFile(file-be-gone)");
    level4.removeFile("file-be-gone");

    mock.expectCall("removeDirectory(dir-be-gone)");
    level4.removeDirectory("dir-be-gone");

    // setDirectoryPermissions
    mock.expectCall("setDirectoryPermissions(u/1/dir,u2,r)");
    level4.setDirectoryPermissions("u/1/dir", "u2", "r");

    // getFileInformation
    {
        FileBase::Info in;
        in.type = FileBase::IsDirectory;
        in.visibility = 2;
        in.contentId = "xyz";
        mock.expectCall("getFileInformation(what)");
        mock.provideReturnValue(in);

        FileBase::Info out = level4.getFileInformation("what");
        TS_ASSERT_EQUALS(out.type, FileBase::IsDirectory);
        TS_ASSERT_EQUALS(out.visibility.orElse(99), 2);
        TS_ASSERT_EQUALS(out.size.isValid(), false);
        TS_ASSERT_EQUALS(out.contentId.orElse(""), "xyz");
    }

    // getDiskUsage
    {
        FileBase::Usage in;
        in.numItems = 9300;
        in.totalKBytes = 2348;
        mock.expectCall("getDiskUsage(u)");
        mock.provideReturnValue(in);

        FileBase::Usage out = level4.getDiskUsage("u");
        TS_ASSERT_EQUALS(out.numItems, in.numItems);
        TS_ASSERT_EQUALS(out.totalKBytes, in.totalKBytes);
    }

    mock.checkFinish();
}
