/**
  *  \file test/server/interface/hostfiletest.cpp
  *  \brief Test for server::interface::HostFile
  */

#include "server/interface/hostfile.hpp"
#include "afl/test/testrunner.hpp"

namespace {
    using server::interface::HostFile;

    void testFormatParse(afl::test::Assert a, HostFile::Label label, const char* name)
    {
        // Format
        a(name).checkEqual("formatLabel", HostFile::formatLabel(label), name);

        // Parse
        HostFile::Label result;
        a(name).checkEqual("parseLabel", HostFile::parseLabel(name, result), true);
        a(name).checkEqual("result",     result, label);
    }
}

/** Interface test. */
AFL_TEST_NOARG("server.interface.HostFile:interface")
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
AFL_TEST("server.interface.HostFile:format", a)
{
    // Good cases
    testFormatParse(a, HostFile::NameLabel, "name");
    testFormatParse(a, HostFile::GameLabel, "game");
    testFormatParse(a, HostFile::SlotLabel, "slot");
    testFormatParse(a, HostFile::TurnLabel, "turn");
    testFormatParse(a, HostFile::ToolLabel, "tool");
    testFormatParse(a, HostFile::NoLabel, "none");
    testFormatParse(a, HostFile::HistoryLabel, "history");

    // Bad cases
    HostFile::Label tmp;
    a.checkEqual("01. parseLabel error", HostFile::parseLabel("", tmp), false);
    a.checkEqual("02. parseLabel error", HostFile::parseLabel("NAME", tmp), false);
    a.checkEqual("03. parseLabel error", HostFile::parseLabel("what", tmp), false);
}

/** Test mergeInfo(). */

AFL_TEST("server.interface.HostFile:mergeInfo:empty", a)
{
    HostFile::Info aa, bb;
    HostFile::mergeInfo(aa, bb);
    a.check("", !aa.gameId.isValid());
}

AFL_TEST("server.interface.HostFile:mergeInfo:gameId:left", a)
{
    HostFile::Info aa, bb;
    aa.gameId = 9;
    HostFile::mergeInfo(aa, bb);
    a.checkEqual("gameId", aa.gameId.orElse(0), 9);
}

AFL_TEST("server.interface.HostFile:mergeInfo:gameId:right", a)
{
    HostFile::Info aa, bb;
    bb.gameId = 9;
    HostFile::mergeInfo(aa, bb);
    a.checkEqual("gameId", aa.gameId.orElse(0), 9);
}

AFL_TEST("server.interface.HostFile:mergeInfo:gameId:both", a)
{
    HostFile::Info aa, bb;
    aa.gameId = 1;
    bb.gameId = 2;
    HostFile::mergeInfo(aa, bb);
    a.checkEqual("gameId", aa.gameId.orElse(0), 1);
}

AFL_TEST("server.interface.HostFile:mergeInfo:all-fields", a)
{
    HostFile::Info aa, bb;
    bb.gameId = 7;
    bb.slotId = 9;
    bb.turnNumber = 11;
    bb.gameName = "s";
    bb.slotName = "t";
    bb.toolName = "u";
    HostFile::mergeInfo(aa, bb);
    a.checkEqual("05. gameId",     aa.gameId.orElse(0), 7);
    a.checkEqual("06. slotId",     aa.slotId.orElse(0), 9);
    a.checkEqual("07. turnNumber", aa.turnNumber.orElse(0), 11);
    a.checkEqual("08. gameName",   aa.gameName.orElse(""), "s");
    a.checkEqual("09. slotName",   aa.slotName.orElse(""), "t");
    a.checkEqual("10. toolName",   aa.toolName.orElse(""), "u");
}
