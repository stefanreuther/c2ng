/**
  *  \file u/t_server_interface_hostfile.cpp
  *  \brief Test for server::interface::HostFile
  */

#include "server/interface/hostfile.hpp"

#include "t_server_interface.hpp"

namespace {
    using server::interface::HostFile;

    void testFormatParse(HostFile::Label label, const char* name)
    {
        // Format
        TSM_ASSERT_EQUALS(name, HostFile::formatLabel(label), name);

        // Parse
        HostFile::Label result;
        TSM_ASSERT_EQUALS(name, HostFile::parseLabel(name, result), true);
        TSM_ASSERT_EQUALS(name, result, label);
    }
}

/** Interface test. */
void
TestServerInterfaceHostFile::testInterface()
{
    class Tester : public server::interface::HostFile {
     public:
        virtual String_t getFile(String_t /*fileName*/)
            { return String_t(); }
        virtual void getDirectoryContent(String_t /*dirName*/, InfoVector_t& /*result*/)
            { }
        virtual Info getFileInformation(String_t /*fileName*/)
            { return Info(); }
        virtual void getPathDescription(String_t /*dirName*/, InfoVector_t& /*result*/)
            { }
    };
    Tester t;
}

/** Test format/parse. */
void
TestServerInterfaceHostFile::testFormat()
{
    // Good cases
    testFormatParse(HostFile::NameLabel, "name");
    testFormatParse(HostFile::GameLabel, "game");
    testFormatParse(HostFile::SlotLabel, "slot");
    testFormatParse(HostFile::TurnLabel, "turn");
    testFormatParse(HostFile::ToolLabel, "tool");
    testFormatParse(HostFile::NoLabel, "none");
    testFormatParse(HostFile::HistoryLabel, "history");

    // Bad cases
    HostFile::Label tmp;
    TS_ASSERT_EQUALS(HostFile::parseLabel("", tmp), false);
    TS_ASSERT_EQUALS(HostFile::parseLabel("NAME", tmp), false);
    TS_ASSERT_EQUALS(HostFile::parseLabel("what", tmp), false);
}

/** Test mergeInfo(). */
void
TestServerInterfaceHostFile::testMergeInfo()
{
    {
        HostFile::Info a, b;
        HostFile::mergeInfo(a, b);
        TS_ASSERT(!a.gameId.isValid());
    }
    {
        HostFile::Info a, b;
        a.gameId = 9;
        HostFile::mergeInfo(a, b);
        TS_ASSERT_EQUALS(a.gameId.orElse(0), 9);
    }
    {
        HostFile::Info a, b;
        b.gameId = 9;
        HostFile::mergeInfo(a, b);
        TS_ASSERT_EQUALS(a.gameId.orElse(0), 9);
    }
    {
        HostFile::Info a, b;
        a.gameId = 1;
        b.gameId = 2;
        HostFile::mergeInfo(a, b);
        TS_ASSERT_EQUALS(a.gameId.orElse(0), 1);
    }
    {
        HostFile::Info a, b;
        b.gameId = 7;
        b.slotId = 9;
        b.turnNumber = 11;
        b.gameName = "s";
        b.slotName = "t";
        b.toolName = "u";
        HostFile::mergeInfo(a, b);
        TS_ASSERT_EQUALS(a.gameId.orElse(0), 7);
        TS_ASSERT_EQUALS(a.slotId.orElse(0), 9);
        TS_ASSERT_EQUALS(a.turnNumber.orElse(0), 11);
        TS_ASSERT_EQUALS(a.gameName.orElse(""), "s");
        TS_ASSERT_EQUALS(a.slotName.orElse(""), "t");
        TS_ASSERT_EQUALS(a.toolName.orElse(""), "u");
    }
}

