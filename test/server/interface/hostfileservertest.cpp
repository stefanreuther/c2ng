/**
  *  \file test/server/interface/hostfileservertest.cpp
  *  \brief Test for server::interface::HostFileServer
  */

#include "server/interface/hostfileserver.hpp"

#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/hostfile.hpp"
#include "server/interface/hostfileclient.hpp"
#include "server/types.hpp"
#include <stdexcept>

using afl::data::Access;
using afl::data::Segment;
using afl::string::Format;
using server::Value_t;
using server::interface::HostFile;
using server::interface::FileBase;

namespace {
    class HostFileMock : public afl::test::CallReceiver, public HostFile {
     public:
        HostFileMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual String_t getFile(String_t fileName)
            {
                checkCall(Format("getFile(%s)", fileName));
                return consumeReturnValue<String_t>();
            }
        virtual void getDirectoryContent(String_t dirName, InfoVector_t& result)
            {
                checkCall(Format("getDirectoryContent(%s)", dirName));
                int n = consumeReturnValue<int>();
                while (n-- > 0) {
                    result.push_back(consumeReturnValue<Info>());
                }
            }

        virtual Info getFileInformation(String_t fileName)
            {
                checkCall(Format("getFileInformation(%s)", fileName));
                return consumeReturnValue<Info>();
            }

        virtual void getPathDescription(String_t dirName, InfoVector_t& result)
            {
                checkCall(Format("getPathDescription(%s)", dirName));
                int n = consumeReturnValue<int>();
                while (n-- > 0) {
                    result.push_back(consumeReturnValue<Info>());
                }
            }
    };

    HostFile::Info makeInfo(const String_t& name, int turnNumber)
    {
        HostFile::Info i;
        i.name = name;
        i.turnNumber = turnNumber;
        return i;
    }
}

/** Test server operations. */
AFL_TEST("server.interface.HostFileServer:commands", a)
{
    HostFileMock mock(a);
    server::interface::HostFileServer testee(mock);

    // getFile
    mock.expectCall("getFile(d/f)");
    mock.provideReturnValue(String_t("cont..."));
    a.checkEqual("01. get", testee.callString(Segment().pushBackString("GET").pushBackString("d/f")), "cont...");

    mock.expectCall("getFile(d/f2)");
    mock.provideReturnValue(String_t("x2"));
    a.checkEqual("11. get", testee.callString(Segment().pushBackString("get").pushBackString("d/f2")), "x2");

    // getFileInformation, full info
    {
        HostFile::Info i;
        i.type = server::interface::FileBase::IsDirectory;
        i.visibility = 2;
        i.size = 99;
        i.contentId = "c14";
        i.name = "dd";
        i.label = HostFile::SlotLabel;
        i.turnNumber = 42;
        i.slotId = 9;
        i.slotName = "The Robots";
        i.gameId = 3;
        i.gameName = "Third";
        i.toolName = "Ragnarok";
        mock.expectCall("getFileInformation(u/d)");
        mock.provideReturnValue(i);

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("STAT").pushBackString("u/d")));
        Access ap(p);

        a.checkEqual("21. type",       ap("type").toString(), "dir");
        a.checkEqual("22. visibility", ap("visibility").toInteger(), 2);
        a.checkEqual("23. size",       ap("size").toInteger(), 99);
        a.checkEqual("24. id",         ap("id").toString(), "c14");
        a.checkEqual("25. name",       ap("name").toString(), "dd");
        a.checkEqual("26. label",      ap("label").toString(), "slot");
        a.checkEqual("27. turn",       ap("turn").toInteger(), 42);
        a.checkEqual("28. slot",       ap("slot").toInteger(), 9);
        a.checkEqual("29. slotname",   ap("slotname").toString(), "The Robots");
        a.checkEqual("30. game",       ap("game").toInteger(), 3);
        a.checkEqual("31. gamename",   ap("gamename").toString(), "Third");
        a.checkEqual("32. toolname",   ap("toolname").toString(), "Ragnarok");
    }

    // getDirectoryContent
    {
        mock.expectCall("getDirectoryContent(a/b/c)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(makeInfo("f1", 42));
        mock.provideReturnValue(makeInfo("q", 9));

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("LS").pushBackString("a/b/c")));
        Access ap(p);

        a.checkEqual("41. getArraySize", ap.getArraySize(), 4U);
        a.checkEqual("42. name", ap[0].toString(), "f1");
        a.checkEqual("43. name", ap[1]("name").toString(), "f1");
        a.checkEqual("44. turn", ap[1]("turn").toInteger(), 42);
        a.checkEqual("45. name", ap[2].toString(), "q");
        a.checkEqual("46. name", ap[3]("name").toString(), "q");
        a.checkEqual("47. turn", ap[3]("turn").toInteger(), 9);
    }

    // getPathDescription
    {
        mock.expectCall("getPathDescription(pp)");
        mock.provideReturnValue(3);
        mock.provideReturnValue(makeInfo("a", 99));
        mock.provideReturnValue(makeInfo("b", 88));
        mock.provideReturnValue(makeInfo("c", 77));

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("PSTAT").pushBackString("pp")));
        Access ap(p);

        a.checkEqual("51. getArraySize", ap.getArraySize(), 6U);
        a.checkEqual("52. name", ap[0].toString(), "a");
        a.checkEqual("53. name", ap[1]("name").toString(), "a");
        a.checkEqual("54. name", ap[2].toString(), "b");
        a.checkEqual("55. name", ap[3]("name").toString(), "b");
        a.checkEqual("56. name", ap[4].toString(), "c");
        a.checkEqual("57. name", ap[5]("name").toString(), "c");
    }
}

/** Test errors. */
AFL_TEST("server.interface.HostFileServer:errors", a)
{
    HostFileMock mock(a);
    server::interface::HostFileServer testee(mock);

    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    AFL_CHECK_THROWS(a("01. empty"),         testee.call(empty), std::exception);
    AFL_CHECK_THROWS(a("02. bad verb"),      testee.call(Segment().pushBackString("wut")), std::exception);
    AFL_CHECK_THROWS(a("03. missing arg"),   testee.call(Segment().pushBackString("LS")), std::exception);
    AFL_CHECK_THROWS(a("04. too many args"), testee.call(Segment().pushBackString("LS").pushBackString("x").pushBackString("y")), std::exception);

    // ComposableCommandHandler personality
    interpreter::Arguments args(empty, 0, 0);
    std::auto_ptr<afl::data::Value> p;
    a.checkEqual("11. bad verb", testee.handleCommand("huhu", args, p), false);

    mock.checkFinish();
}

/** Test roundtrip operation. */
AFL_TEST("server.interface.HostFileServer:roundtrip", a)
{
    HostFileMock mock(a);
    server::interface::HostFileServer level1(mock);
    server::interface::HostFileClient level2(level1);
    server::interface::HostFileServer level3(level2);
    server::interface::HostFileClient level4(level3);

    // getFile
    mock.expectCall("getFile(x/y)");
    mock.provideReturnValue(String_t("z"));
    a.checkEqual("01", level4.getFile("x/y"), "z");

    // getFileInformation, full info
    {
        HostFile::Info i;
        i.type = server::interface::FileBase::IsFile;
        i.visibility = 1;
        i.size = 10005;
        i.contentId = "32168";
        i.name = "fq";
        i.label = HostFile::TurnLabel;
        i.turnNumber = 42;
        i.slotId = 1;
        i.slotName = "The Feds";
        i.gameId = 2;
        i.gameName = "Second";
        i.toolName = "Sphere";
        mock.expectCall("getFileInformation(a/f/q)");
        mock.provideReturnValue(i);

        HostFile::Info i2 = level4.getFileInformation("a/f/q");

        a.checkEqual("11. type", i2.type, server::interface::FileBase::IsFile);
        a.checkEqual("12. visibility", i2.visibility.orElse(0), 1);
        a.checkEqual("13. size", i2.size.orElse(0), 10005);
        a.checkEqual("14. contentId", i2.contentId.orElse(""), "32168");
        a.checkEqual("15. name", i2.name, "fq");
        a.checkEqual("16. label", i2.label, HostFile::TurnLabel);
        a.checkEqual("17. turnNumber", i2.turnNumber.orElse(0), 42);
        a.checkEqual("18. slotId", i2.slotId.orElse(0), 1);
        a.checkEqual("19. slotName", i2.slotName.orElse(""), "The Feds");
        a.checkEqual("20. gameId", i2.gameId.orElse(0), 2);
        a.checkEqual("21. gameName", i2.gameName.orElse(""), "Second");
        a.checkEqual("22. toolName", i2.toolName.orElse(""), "Sphere");
    }

    // getDirectoryContent
    {
        mock.expectCall("getDirectoryContent(a/b/c)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(makeInfo("f1", 42));
        mock.provideReturnValue(makeInfo("q", 9));

        HostFile::InfoVector_t v;
        level4.getDirectoryContent("a/b/c", v);

        a.checkEqual("31. size", v.size(), 2U);
        a.checkEqual("32", v[0].name, "f1");
        a.checkEqual("33", v[1].name, "q");
    }

    // getPathDescription
    {
        mock.expectCall("getPathDescription(pp)");
        mock.provideReturnValue(3);
        mock.provideReturnValue(makeInfo("e", 99));
        mock.provideReturnValue(makeInfo("f", 88));
        mock.provideReturnValue(makeInfo("g", 77));

        HostFile::InfoVector_t v;
        level4.getPathDescription("pp", v);

        a.checkEqual("41. size", v.size(), 3U);
        a.checkEqual("42", v[0].name, "e");
        a.checkEqual("43", v[1].name, "f");
        a.checkEqual("44", v[2].name, "g");
    }
}

/** Test interoperability with FileBase. */
AFL_TEST("server.interface.HostFileServer:FileBase", a)
{
    HostFileMock mock(a);
    server::interface::HostFileServer srv(mock);
    server::interface::FileBaseClient client(srv);

    // getFile
    mock.expectCall("getFile(x/y)");
    mock.provideReturnValue(String_t("z"));
    a.checkEqual("01. getFile", client.getFile("x/y"), "z");

    // getFileInformation, full info
    {
        HostFile::Info i;
        i.type = server::interface::FileBase::IsFile;
        i.visibility = 1;
        i.size = 10005;
        i.contentId = "32168";
        i.name = "fq";
        i.label = HostFile::TurnLabel;
        mock.expectCall("getFileInformation(q/f)");
        mock.provideReturnValue(i);

        FileBase::Info i2 = client.getFileInformation("q/f");

        a.checkEqual("11. type",       i2.type, FileBase::IsFile);
        a.checkEqual("12. visibility", i2.visibility.orElse(0), 1);
        a.checkEqual("13. size",       i2.size.orElse(0), 10005);
        a.checkEqual("14. contentId",  i2.contentId.orElse(""), "32168");
    }

    // getDirectoryContent
    {
        mock.expectCall("getDirectoryContent(a/b/c)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(makeInfo("f1", 42));
        mock.provideReturnValue(makeInfo("q", 9));

        FileBase::ContentInfoMap_t m;
        client.getDirectoryContent("a/b/c", m);

        a.checkEqual("21. size", m.size(), 2U);
        a.checkNonNull("22. f1", m["f1"]);
        a.checkNonNull("23. q",  m["q"]);
    }
}
