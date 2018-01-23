/**
  *  \file u/t_server_interface_filebase.cpp
  *  \brief Test for server::interface::FileBase
  */

#include "server/interface/filebase.hpp"

#include <stdexcept>
#include "t_server_interface.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/types.hpp"

/** Interface test. */
void
TestServerInterfaceFileBase::testInterface()
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
void
TestServerInterfaceFileBase::testProperty()
{
    class Tester : public server::interface::FileBase, public afl::test::CallReceiver {
     public:
        Tester()
            : CallReceiver("testProperty")
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
    Tester t;
    t.expectCall("get(dd,pp)");
    t.provideReturnValue(0);
    TS_ASSERT_EQUALS(t.getDirectoryIntegerProperty("dd", "pp"), 0);

    t.expectCall("get(dd2,pp2)");
    t.provideReturnValue(server::makeIntegerValue(99));
    TS_ASSERT_EQUALS(t.getDirectoryIntegerProperty("dd2", "pp2"), 99);

    t.expectCall("get(dd3,pp3)");
    t.provideReturnValue(server::makeStringValue("-3"));
    TS_ASSERT_EQUALS(t.getDirectoryIntegerProperty("dd3", "pp3"), -3);

    t.expectCall("get(dd4,pp4)");
    t.provideReturnValue(server::makeStringValue("foo"));
    TS_ASSERT_THROWS(t.getDirectoryIntegerProperty("dd4", "pp4"), std::exception);

    // String
    t.expectCall("get(a,b)");
    t.provideReturnValue(0);
    TS_ASSERT_EQUALS(t.getDirectoryStringProperty("a", "b"), "");

    t.expectCall("get(c,d)");
    t.provideReturnValue(server::makeIntegerValue(150));
    TS_ASSERT_EQUALS(t.getDirectoryStringProperty("c", "d"), "150");

    t.expectCall("get(e,f)");
    t.provideReturnValue(server::makeStringValue("hi"));
    TS_ASSERT_EQUALS(t.getDirectoryStringProperty("e", "f"), "hi");
}

/** Test getFileNT. */
void
TestServerInterfaceFileBase::testGetFileNT()
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
    TS_ASSERT_EQUALS(t.getFile("a"), "<a>");
    TS_ASSERT_THROWS(t.getFile("ab"), std::exception);
    TS_ASSERT_EQUALS(t.getFile("abc"), "<abc>");

    // getFileNT
    afl::base::Optional<String_t> s;
    TS_ASSERT_THROWS_NOTHING(s = t.getFileNT("a"));
    TS_ASSERT_EQUALS(s.isValid(), true);
    TS_ASSERT_EQUALS(*s.get(), "<a>");

    TS_ASSERT_THROWS_NOTHING(s = t.getFileNT("ab"));
    TS_ASSERT_EQUALS(s.isValid(), false);

    TS_ASSERT_THROWS_NOTHING(s = t.getFileNT("abc"));
    TS_ASSERT_EQUALS(s.isValid(), true);
    TS_ASSERT_EQUALS(*s.get(), "<abc>");
}

