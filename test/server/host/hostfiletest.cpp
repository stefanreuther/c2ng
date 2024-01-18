/**
  *  \file test/server/host/hostfiletest.cpp
  *  \brief Test for server::host::HostFile
  */

#include "server/host/hostfile.hpp"

#include "afl/test/testrunner.hpp"
#include <stdexcept>

using server::host::file::Item;

namespace {
    class Two : public Item {
     public:
        virtual String_t getName()
            { return "two"; }
        virtual Info_t getInfo()
            {
                Info_t i;
                i.name = getName();
                i.type = server::interface::FileBase::IsFile;
                i.label = server::interface::HostFile::SlotLabel;
                i.slotId = 3;
                return i;
            }
        virtual Item* find(const String_t& name)
            { return defaultFind(name); }
        virtual void listContent(ItemVector_t& out)
            { defaultList(out); }
        virtual String_t getContent()
            { return "The Content"; }
    };
    class One : public Item {
     public:
        virtual String_t getName()
            { return "one"; }
        virtual Info_t getInfo()
            {
                Info_t i;
                i.name = getName();
                i.type = server::interface::FileBase::IsDirectory;
                i.label = server::interface::HostFile::GameLabel;
                i.gameId = 12;
                i.gameName = "twelve";
                return i;
            }
        virtual Item* find(const String_t& name)
            { return defaultFind(name); }
        virtual void listContent(ItemVector_t& out)
            { out.pushBackNew(new Two()); }
        virtual String_t getContent()
            { return defaultGetContent(); }
    };
    class Zero : public Item {
     public:
        virtual String_t getName()
            { throw std::runtime_error("This call is not expected to occur"); }
        virtual Info_t getInfo()
            { return Info_t(); }
        virtual Item* find(const String_t& name)
            { return defaultFind(name); }
        virtual void listContent(ItemVector_t& out)
            { out.pushBackNew(new One()); }
        virtual String_t getContent()
            { return defaultGetContent(); }
    };
}

/** Simple test. */
AFL_TEST("server.host.HostFile", a)
{
    Zero z;
    server::host::HostFile testee(z);

    // File access
    a.checkEqual      ("01. getFile one/two",     testee.getFile("one/two"), "The Content");
    AFL_CHECK_THROWS(a("02. getFile one/three"),  testee.getFile("one/three"), std::exception);
    AFL_CHECK_THROWS(a("03. getFile one"),        testee.getFile("one"), std::exception);
    AFL_CHECK_THROWS(a("04. getFile empty"),      testee.getFile(""), std::exception);
    AFL_CHECK_THROWS(a("05. getFile /one/three"), testee.getFile("/one/three"), std::exception);

    // Information
    AFL_CHECK_THROWS(a("11. getFileInformation ''"),   testee.getFileInformation(""), std::exception);
    AFL_CHECK_THROWS(a("12. getFileInformation /"),    testee.getFileInformation("/"), std::exception);
    AFL_CHECK_THROWS(a("13. getFileInformation one/"), testee.getFileInformation("one/"), std::exception);
    AFL_CHECK_THROWS(a("14. getFileInformation two"),  testee.getFileInformation("two"), std::exception);

    server::interface::HostFile::Info i = testee.getFileInformation("one");
    a.checkEqual("21. name",   i.name, "one");
    a.checkEqual("22. gameId", i.gameId.orElse(0), 12);
    a.checkEqual("23. slotId", i.slotId.isValid(), false);

    i = testee.getFileInformation("one/two");
    a.checkEqual("31. name",   i.name, "two");
    a.checkEqual("32. gameId", i.gameId.orElse(0), 12);
    a.checkEqual("33. slotId", i.slotId.orElse(0), 3);

    // Directory content
    {
        server::interface::HostFile::InfoVector_t vec;
        AFL_CHECK_THROWS(a("41. getDirectoryContent ''"),      testee.getDirectoryContent("", vec), std::exception);
        AFL_CHECK_THROWS(a("42. getDirectoryContent one/two"), testee.getDirectoryContent("one/two", vec), std::exception);
        AFL_CHECK_THROWS(a("43. getDirectoryContent /one"),    testee.getDirectoryContent("/one", vec), std::exception);
        AFL_CHECK_THROWS(a("44. getDirectoryContent one/x"),   testee.getDirectoryContent("one/x", vec), std::exception);
        AFL_CHECK_THROWS(a("45. getDirectoryContent one/"),    testee.getDirectoryContent("one/", vec), std::exception);
    }
    {
        server::interface::HostFile::InfoVector_t vec;
        AFL_CHECK_SUCCEEDS(a("46. getDirectoryContent"), testee.getDirectoryContent("one", vec));
        a.checkEqual("47. size",   vec.size(), 1U);
        a.checkEqual("48. name",   vec[0].name, "two");
        a.checkEqual("49. gameId", vec[0].gameId.orElse(0), 12);  // inherited from parent
        a.checkEqual("50. slotId", vec[0].slotId.orElse(0), 3);   // from directory entry
    }

    // Path
    {
        server::interface::HostFile::InfoVector_t vec;
        AFL_CHECK_THROWS(a("51. getPathDescription ''"),    testee.getPathDescription("", vec), std::exception);
        AFL_CHECK_THROWS(a("52. getPathDescription /one"),  testee.getPathDescription("/one", vec), std::exception);
        AFL_CHECK_THROWS(a("53. getPathDescription one/x"), testee.getPathDescription("one/x", vec), std::exception);
        AFL_CHECK_THROWS(a("54. getPathDescription one/"),  testee.getPathDescription("one/", vec), std::exception);
    }
    {
        server::interface::HostFile::InfoVector_t vec;
        AFL_CHECK_SUCCEEDS(a("55. getPathDescription"), testee.getPathDescription("one", vec));
        a.checkEqual("56. size",   vec.size(), 1U);
        a.checkEqual("57. name",   vec[0].name, "one");
        a.checkEqual("58. gameId", vec[0].gameId.orElse(0), 12);
        a.checkEqual("59. slotId", vec[0].slotId.isValid(), false);
    }
    {
        server::interface::HostFile::InfoVector_t vec;
        AFL_CHECK_SUCCEEDS(a("60. getPathDescription"), testee.getPathDescription("one/two", vec));
        a.checkEqual("61. size",   vec.size(), 2U);
        a.checkEqual("62. name",   vec[0].name, "one");
        a.checkEqual("63. gameId", vec[0].gameId.orElse(0), 12);
        a.checkEqual("64. slotId", vec[0].slotId.isValid(), false);
        a.checkEqual("65. name",   vec[1].name, "two");
        a.checkEqual("66. gameId", vec[1].gameId.orElse(0), 12);
        a.checkEqual("67. slotId", vec[1].slotId.orElse(0), 3);
    }
}
