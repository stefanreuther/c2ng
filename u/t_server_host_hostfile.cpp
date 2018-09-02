/**
  *  \file u/t_server_host_hostfile.cpp
  *  \brief Test for server::host::HostFile
  */

#include <stdexcept>
#include "server/host/hostfile.hpp"

#include "t_server_host.hpp"

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
void
TestServerHostHostFile::testIt()
{
    Zero z;
    server::host::HostFile testee(z);

    // File access
    TS_ASSERT_EQUALS(testee.getFile("one/two"), "The Content");
    TS_ASSERT_THROWS(testee.getFile("one/three"), std::exception);
    TS_ASSERT_THROWS(testee.getFile("one"), std::exception);
    TS_ASSERT_THROWS(testee.getFile(""), std::exception);
    TS_ASSERT_THROWS(testee.getFile("/one/three"), std::exception);

    // Information
    TS_ASSERT_THROWS(testee.getFileInformation(""), std::exception);
    TS_ASSERT_THROWS(testee.getFileInformation("/"), std::exception);
    TS_ASSERT_THROWS(testee.getFileInformation("one/"), std::exception);
    TS_ASSERT_THROWS(testee.getFileInformation("two"), std::exception);

    server::interface::HostFile::Info i = testee.getFileInformation("one");
    TS_ASSERT_EQUALS(i.name, "one");
    TS_ASSERT_EQUALS(i.gameId.orElse(0), 12);
    TS_ASSERT_EQUALS(i.slotId.isValid(), false);

    i = testee.getFileInformation("one/two");
    TS_ASSERT_EQUALS(i.name, "two");
    TS_ASSERT_EQUALS(i.gameId.orElse(0), 12);
    TS_ASSERT_EQUALS(i.slotId.orElse(0), 3);

    // Directory content
    {
        server::interface::HostFile::InfoVector_t vec;
        TS_ASSERT_THROWS(testee.getDirectoryContent("", vec), std::exception);
        TS_ASSERT_THROWS(testee.getDirectoryContent("one/two", vec), std::exception);
        TS_ASSERT_THROWS(testee.getDirectoryContent("/one", vec), std::exception);
        TS_ASSERT_THROWS(testee.getDirectoryContent("one/x", vec), std::exception);
        TS_ASSERT_THROWS(testee.getDirectoryContent("one/", vec), std::exception);
    }
    {
        server::interface::HostFile::InfoVector_t vec;
        TS_ASSERT_THROWS_NOTHING(testee.getDirectoryContent("one", vec));
        TS_ASSERT_EQUALS(vec.size(), 1U);
        TS_ASSERT_EQUALS(vec[0].name, "two");
        TS_ASSERT_EQUALS(vec[0].gameId.orElse(0), 12);  // inherited from parent
        TS_ASSERT_EQUALS(vec[0].slotId.orElse(0), 3);   // from directory entry
    }

    // Path
    {
        server::interface::HostFile::InfoVector_t vec;
        TS_ASSERT_THROWS(testee.getPathDescription("", vec), std::exception);
        TS_ASSERT_THROWS(testee.getPathDescription("/one", vec), std::exception);
        TS_ASSERT_THROWS(testee.getPathDescription("one/x", vec), std::exception);
        TS_ASSERT_THROWS(testee.getPathDescription("one/", vec), std::exception);
    }
    {
        server::interface::HostFile::InfoVector_t vec;
        TS_ASSERT_THROWS_NOTHING(testee.getPathDescription("one", vec));
        TS_ASSERT_EQUALS(vec.size(), 1U);
        TS_ASSERT_EQUALS(vec[0].name, "one");
        TS_ASSERT_EQUALS(vec[0].gameId.orElse(0), 12);
        TS_ASSERT_EQUALS(vec[0].slotId.isValid(), false);
    }
    {
        server::interface::HostFile::InfoVector_t vec;
        TS_ASSERT_THROWS_NOTHING(testee.getPathDescription("one/two", vec));
        TS_ASSERT_EQUALS(vec.size(), 2U);
        TS_ASSERT_EQUALS(vec[0].name, "one");
        TS_ASSERT_EQUALS(vec[0].gameId.orElse(0), 12);
        TS_ASSERT_EQUALS(vec[0].slotId.isValid(), false);
        TS_ASSERT_EQUALS(vec[1].name, "two");
        TS_ASSERT_EQUALS(vec[1].gameId.orElse(0), 12);
        TS_ASSERT_EQUALS(vec[1].slotId.orElse(0), 3);
    }
}

