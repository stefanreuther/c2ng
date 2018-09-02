/**
  *  \file u/t_server_interface_hostfileserver.cpp
  *  \brief Test for server::interface::HostFileServer
  */

#include <stdexcept>
#include "server/interface/hostfileserver.hpp"

#include "t_server_interface.hpp"
#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/interface/hostfile.hpp"
#include "server/types.hpp"
#include "server/interface/hostfileclient.hpp"
#include "server/interface/filebaseclient.hpp"

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
void
TestServerInterfaceHostFileServer::testServer()
{
    HostFileMock mock("testServer");
    server::interface::HostFileServer testee(mock);

    // getFile
    mock.expectCall("getFile(d/f)");
    mock.provideReturnValue(String_t("cont..."));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("GET").pushBackString("d/f")), "cont...");

    mock.expectCall("getFile(d/f2)");
    mock.provideReturnValue(String_t("x2"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("get").pushBackString("d/f2")), "x2");

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
        Access a(p);

        TS_ASSERT_EQUALS(a("type").toString(), "dir");
        TS_ASSERT_EQUALS(a("visibility").toInteger(), 2);
        TS_ASSERT_EQUALS(a("size").toInteger(), 99);
        TS_ASSERT_EQUALS(a("id").toString(), "c14");
        TS_ASSERT_EQUALS(a("name").toString(), "dd");
        TS_ASSERT_EQUALS(a("label").toString(), "slot");
        TS_ASSERT_EQUALS(a("turn").toInteger(), 42);
        TS_ASSERT_EQUALS(a("slot").toInteger(), 9);
        TS_ASSERT_EQUALS(a("slotname").toString(), "The Robots");
        TS_ASSERT_EQUALS(a("game").toInteger(), 3);
        TS_ASSERT_EQUALS(a("gamename").toString(), "Third");
        TS_ASSERT_EQUALS(a("toolname").toString(), "Ragnarok");
    }

    // getDirectoryContent
    {
        mock.expectCall("getDirectoryContent(a/b/c)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(makeInfo("f1", 42));
        mock.provideReturnValue(makeInfo("q", 9));

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("LS").pushBackString("a/b/c")));
        Access a(p);

        TS_ASSERT_EQUALS(a.getArraySize(), 4U);
        TS_ASSERT_EQUALS(a[0].toString(), "f1");
        TS_ASSERT_EQUALS(a[1]("name").toString(), "f1");
        TS_ASSERT_EQUALS(a[1]("turn").toInteger(), 42);
        TS_ASSERT_EQUALS(a[2].toString(), "q");
        TS_ASSERT_EQUALS(a[3]("name").toString(), "q");
        TS_ASSERT_EQUALS(a[3]("turn").toInteger(), 9);
    }

    // getPathDescription
    {
        mock.expectCall("getPathDescription(pp)");
        mock.provideReturnValue(3);
        mock.provideReturnValue(makeInfo("a", 99));
        mock.provideReturnValue(makeInfo("b", 88));
        mock.provideReturnValue(makeInfo("c", 77));

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("PSTAT").pushBackString("pp")));
        Access a(p);

        TS_ASSERT_EQUALS(a.getArraySize(), 6U);
        TS_ASSERT_EQUALS(a[0].toString(), "a");
        TS_ASSERT_EQUALS(a[1]("name").toString(), "a");
        TS_ASSERT_EQUALS(a[2].toString(), "b");
        TS_ASSERT_EQUALS(a[3]("name").toString(), "b");
        TS_ASSERT_EQUALS(a[4].toString(), "c");
        TS_ASSERT_EQUALS(a[5]("name").toString(), "c");
    }
}

/** Test errors. */
void
TestServerInterfaceHostFileServer::testError()
{
    HostFileMock mock("testServer");
    server::interface::HostFileServer testee(mock);

    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    TS_ASSERT_THROWS(testee.call(empty), std::exception);
    TS_ASSERT_THROWS(testee.call(Segment().pushBackString("wut")), std::exception);
    TS_ASSERT_THROWS(testee.call(Segment().pushBackString("LS")), std::exception);
    TS_ASSERT_THROWS(testee.call(Segment().pushBackString("LS").pushBackString("x").pushBackString("y")), std::exception);

    // ComposableCommandHandler personality
    interpreter::Arguments args(empty, 0, 0);
    std::auto_ptr<afl::data::Value> p;
    TS_ASSERT_EQUALS(testee.handleCommand("huhu", args, p), false);

    mock.checkFinish();
}

/** Test roundtrip operation. */
void
TestServerInterfaceHostFileServer::testRoundtrip()
{
    HostFileMock mock("testServer");
    server::interface::HostFileServer level1(mock);
    server::interface::HostFileClient level2(level1);
    server::interface::HostFileServer level3(level2);
    server::interface::HostFileClient level4(level3);

    // getFile
    mock.expectCall("getFile(x/y)");
    mock.provideReturnValue(String_t("z"));
    TS_ASSERT_EQUALS(level4.getFile("x/y"), "z");

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

        TS_ASSERT_EQUALS(i2.type, server::interface::FileBase::IsFile);
        TS_ASSERT_EQUALS(i2.visibility.orElse(0), 1);
        TS_ASSERT_EQUALS(i2.size.orElse(0), 10005);
        TS_ASSERT_EQUALS(i2.contentId.orElse(""), "32168");
        TS_ASSERT_EQUALS(i2.name, "fq");
        TS_ASSERT_EQUALS(i2.label, HostFile::TurnLabel);
        TS_ASSERT_EQUALS(i2.turnNumber.orElse(0), 42);
        TS_ASSERT_EQUALS(i2.slotId.orElse(0), 1);
        TS_ASSERT_EQUALS(i2.slotName.orElse(""), "The Feds");
        TS_ASSERT_EQUALS(i2.gameId.orElse(0), 2);
        TS_ASSERT_EQUALS(i2.gameName.orElse(""), "Second");
        TS_ASSERT_EQUALS(i2.toolName.orElse(""), "Sphere");
    }

    // getDirectoryContent
    {
        mock.expectCall("getDirectoryContent(a/b/c)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(makeInfo("f1", 42));
        mock.provideReturnValue(makeInfo("q", 9));

        HostFile::InfoVector_t v;
        level4.getDirectoryContent("a/b/c", v);

        TS_ASSERT_EQUALS(v.size(), 2U);
        TS_ASSERT_EQUALS(v[0].name, "f1");
        TS_ASSERT_EQUALS(v[1].name, "q");
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

        TS_ASSERT_EQUALS(v.size(), 3U);
        TS_ASSERT_EQUALS(v[0].name, "e");
        TS_ASSERT_EQUALS(v[1].name, "f");
        TS_ASSERT_EQUALS(v[2].name, "g");
    }
}

/** Test interoperability with FileBase. */
void
TestServerInterfaceHostFileServer::testInteroperability()
{
    HostFileMock mock("testServer");
    server::interface::HostFileServer srv(mock);
    server::interface::FileBaseClient client(srv);

    // getFile
    mock.expectCall("getFile(x/y)");
    mock.provideReturnValue(String_t("z"));
    TS_ASSERT_EQUALS(client.getFile("x/y"), "z");

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

        TS_ASSERT_EQUALS(i2.type, FileBase::IsFile);
        TS_ASSERT_EQUALS(i2.visibility.orElse(0), 1);
        TS_ASSERT_EQUALS(i2.size.orElse(0), 10005);
        TS_ASSERT_EQUALS(i2.contentId.orElse(""), "32168");
    }

    // getDirectoryContent
    {
        mock.expectCall("getDirectoryContent(a/b/c)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(makeInfo("f1", 42));
        mock.provideReturnValue(makeInfo("q", 9));

        FileBase::ContentInfoMap_t m;
        client.getDirectoryContent("a/b/c", m);

        TS_ASSERT_EQUALS(m.size(), 2U);
        TS_ASSERT(m["f1"] != 0);
        TS_ASSERT(m["q"] != 0);
    }
}

