/**
  *  \file test/server/interface/filebasetest.cpp
  *  \brief Test for server::interface::FileBase
  */

#include "server/interface/filebase.hpp"

#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"
#include <stdexcept>

/** Interface test. */
AFL_TEST_NOARG("server.interface.FileBase:interface")
{
    class Tester : public server::interface::FileBase {
     public:
        virtual void copyFile(String_t /*sourceFile*/, String_t /*destFile*/)
            { }
        virtual void forgetDirectory(String_t /*dirName*/)
            { }
        virtual void testFiles(afl::base::Memory<const String_t> /*fileNames*/, afl::data::IntegerList_t& /*resultFlags*/)
            { }
        virtual String_t getFile(String_t /*fileName*/)
            { return String_t(); }
        virtual void getDirectoryContent(String_t /*dirName*/, ContentInfoMap_t& /*result*/)
            { }
        virtual void getDirectoryPermission(String_t /*dirName*/, String_t& /*ownerUserId*/, std::vector<Permission>& /*result*/)
            { }
        virtual void createDirectory(String_t /*dirName*/)
            { }
        virtual void createDirectoryTree(String_t /*dirName*/)
            { }
        virtual void createDirectoryAsUser(String_t /*dirName*/, String_t /*userId*/)
            { }
        virtual afl::data::Value* getDirectoryProperty(String_t /*dirName*/, String_t /*propName*/)
            { return 0; }
        virtual void setDirectoryProperty(String_t /*dirName*/, String_t /*propName*/, String_t /*propValue*/)
            { }
        virtual void putFile(String_t /*fileName*/, String_t /*content*/)
            { }
        virtual void removeFile(String_t /*fileName*/)
            { }
        virtual void removeDirectory(String_t /*dirName*/)
            { }
        virtual void setDirectoryPermissions(String_t /*dirName*/, String_t /*userId*/, String_t /*permission*/)
            { }
        virtual Info getFileInformation(String_t /*fileName*/)
            { return Info(); }
        virtual Usage getDiskUsage(String_t /*dirName*/)
            { return Usage(); }
    };
    Tester t;
}

/** Test getDirectoryIntegerProperty, getDirectoryStringProperty. */
AFL_TEST("server.interface.FileBase:typed-properties", a)
{
    class Tester : public server::interface::FileBase, public afl::test::CallReceiver {
     public:
        Tester(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual void copyFile(String_t /*sourceFile*/, String_t /*destFile*/)
            { }
        virtual void forgetDirectory(String_t /*dirName*/)
            { }
        virtual void testFiles(afl::base::Memory<const String_t> /*fileNames*/, afl::data::IntegerList_t& /*resultFlags*/)
            { }
        virtual String_t getFile(String_t /*fileName*/)
            { return String_t(); }
        virtual void getDirectoryContent(String_t /*dirName*/, ContentInfoMap_t& /*result*/)
            { }
        virtual void getDirectoryPermission(String_t /*dirName*/, String_t& /*ownerUserId*/, std::vector<Permission>& /*result*/)
            { }
        virtual void createDirectory(String_t /*dirName*/)
            { }
        virtual void createDirectoryTree(String_t /*dirName*/)
            { }
        virtual void createDirectoryAsUser(String_t /*dirName*/, String_t /*userId*/)
            { }
        virtual afl::data::Value* getDirectoryProperty(String_t dirName, String_t propName)
            {
                checkCall(afl::string::Format("get(%s,%s)", dirName, propName));
                return consumeReturnValue<afl::data::Value*>();
            }
        virtual void setDirectoryProperty(String_t /*dirName*/, String_t /*propName*/, String_t /*propValue*/)
            { }
        virtual void putFile(String_t /*fileName*/, String_t /*content*/)
            { }
        virtual void removeFile(String_t /*fileName*/)
            { }
        virtual void removeDirectory(String_t /*dirName*/)
            { }
        virtual void setDirectoryPermissions(String_t /*dirName*/, String_t /*userId*/, String_t /*permission*/)
            { }
        virtual Info getFileInformation(String_t /*fileName*/)
            { return Info(); }
        virtual Usage getDiskUsage(String_t /*dirName*/)
            { return Usage(); }

        void provideReturnValue(afl::data::Value* p)
            { CallReceiver::provideReturnValue(p); }
    };

    // Integer
    Tester t(a);
    t.expectCall("get(dd,pp)");
    t.provideReturnValue(0);
    a.checkEqual("01. getDirectoryIntegerProperty", t.getDirectoryIntegerProperty("dd", "pp"), 0);

    t.expectCall("get(dd2,pp2)");
    t.provideReturnValue(server::makeIntegerValue(99));
    a.checkEqual("11. getDirectoryIntegerProperty", t.getDirectoryIntegerProperty("dd2", "pp2"), 99);

    t.expectCall("get(dd3,pp3)");
    t.provideReturnValue(server::makeStringValue("-3"));
    a.checkEqual("21. getDirectoryIntegerProperty", t.getDirectoryIntegerProperty("dd3", "pp3"), -3);

    t.expectCall("get(dd4,pp4)");
    t.provideReturnValue(server::makeStringValue("foo"));
    AFL_CHECK_THROWS(a("31. getDirectoryIntegerProperty"), t.getDirectoryIntegerProperty("dd4", "pp4"), std::exception);

    // String
    t.expectCall("get(a,b)");
    t.provideReturnValue(0);
    a.checkEqual("41. getDirectoryStringProperty", t.getDirectoryStringProperty("a", "b"), "");

    t.expectCall("get(c,d)");
    t.provideReturnValue(server::makeIntegerValue(150));
    a.checkEqual("51. getDirectoryStringProperty", t.getDirectoryStringProperty("c", "d"), "150");

    t.expectCall("get(e,f)");
    t.provideReturnValue(server::makeStringValue("hi"));
    a.checkEqual("61. getDirectoryStringProperty", t.getDirectoryStringProperty("e", "f"), "hi");
}

/** Test getFileNT. */
AFL_TEST("server.interface.FileBase:getFileNT", a)
{
    class Tester : public server::interface::FileBase {
     public:
        virtual void copyFile(String_t /*sourceFile*/, String_t /*destFile*/)
            { }
        virtual void forgetDirectory(String_t /*dirName*/)
            { }
        virtual void testFiles(afl::base::Memory<const String_t> /*fileNames*/, afl::data::IntegerList_t& /*resultFlags*/)
            { }
        virtual String_t getFile(String_t fileName)
            {
                if (fileName.size() % 2 == 0) {
                    throw std::runtime_error("boom");
                } else {
                    return "<" + fileName + ">";
                }
            }
        virtual void getDirectoryContent(String_t /*dirName*/, ContentInfoMap_t& /*result*/)
            { }
        virtual void getDirectoryPermission(String_t /*dirName*/, String_t& /*ownerUserId*/, std::vector<Permission>& /*result*/)
            { }
        virtual void createDirectory(String_t /*dirName*/)
            { }
        virtual void createDirectoryTree(String_t /*dirName*/)
            { }
        virtual void createDirectoryAsUser(String_t /*dirName*/, String_t /*userId*/)
            { }
        virtual afl::data::Value* getDirectoryProperty(String_t /*dirName*/, String_t /*propName*/)
            { return 0; }
        virtual void setDirectoryProperty(String_t /*dirName*/, String_t /*propName*/, String_t /*propValue*/)
            { }
        virtual void putFile(String_t /*fileName*/, String_t /*content*/)
            { }
        virtual void removeFile(String_t /*fileName*/)
            { }
        virtual void removeDirectory(String_t /*dirName*/)
            { }
        virtual void setDirectoryPermissions(String_t /*dirName*/, String_t /*userId*/, String_t /*permission*/)
            { }
        virtual Info getFileInformation(String_t /*fileName*/)
            { return Info(); }
        virtual Usage getDiskUsage(String_t /*dirName*/)
            { return Usage(); }
    };
    Tester t;

    // getFile
    a.checkEqual("01. getFile", t.getFile("a"), "<a>");
    AFL_CHECK_THROWS(a("02. getFile"), t.getFile("ab"), std::exception);
    a.checkEqual("03. getFile", t.getFile("abc"), "<abc>");

    // getFileNT
    afl::base::Optional<String_t> s;
    AFL_CHECK_SUCCEEDS(a("11. getFileNT"), s = t.getFileNT("a"));
    a.checkEqual("12. isValid", s.isValid(), true);
    a.checkEqual("13. content", *s.get(), "<a>");

    AFL_CHECK_SUCCEEDS(a("21. getFileNT"), s = t.getFileNT("ab"));
    a.checkEqual("22. isValid", s.isValid(), false);

    AFL_CHECK_SUCCEEDS(a("31. getFileNT"), s = t.getFileNT("abc"));
    a.checkEqual("32. isValid", s.isValid(), true);
    a.checkEqual("33. content", *s.get(), "<abc>");
}
