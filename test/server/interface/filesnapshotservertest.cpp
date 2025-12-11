/**
  *  \file test/server/interface/filesnapshotservertest.cpp
  *  \brief Test for server::interface::FileSnapshotServer
  */

#include "server/interface/filesnapshotserver.hpp"

#include "afl/data/segment.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "afl/data/access.hpp"
#include "server/interface/filesnapshotclient.hpp"

using afl::data::Segment;
using afl::string::Format;

namespace {
    class FileSnapshotMock : public server::interface::FileSnapshot, public afl::test::CallReceiver {
     public:
        FileSnapshotMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual void createSnapshot(String_t name)
            { checkCall(Format("createSnapshot(%s)", name)); }
        virtual void copySnapshot(String_t oldName, String_t newName)
            { checkCall(Format("copySnapshot(%s,%s)", oldName, newName)); }
        virtual void removeSnapshot(String_t name)
            { checkCall(Format("removeSnapshot(%s)", name)); }
        virtual void listSnapshots(afl::data::StringList_t& out)
            {
                checkCall("listSnapshots()");
                int n = consumeReturnValue<int>();
                for (int i = 0; i < n; ++i) {
                    out.push_back(consumeReturnValue<String_t>());
                }
            }
    };
}

/* Test basic command handling */
AFL_TEST("server.interface.FileSnapshotServer:commands", a)
{
    FileSnapshotMock mock(a);
    server::interface::FileSnapshotServer testee(mock);

    // createSnapshot
    mock.expectCall("createSnapshot(oo)");
    testee.callVoid(Segment().pushBackString("SNAPSHOTADD").pushBackString("oo"));

    // copySnapshot
    mock.expectCall("copySnapshot(bbb,ccc)");
    testee.callVoid(Segment().pushBackString("SNAPSHOTCP").pushBackString("bbb").pushBackString("ccc"));

    // removeSnapshot
    mock.expectCall("removeSnapshot(ggg)");
    testee.callVoid(Segment().pushBackString("SNAPSHOTRM").pushBackString("ggg"));

    // listSnapshots
    {
        mock.expectCall("listSnapshots()");
        mock.provideReturnValue(4);
        mock.provideReturnValue(String_t("fi"));
        mock.provideReturnValue(String_t("se"));
        mock.provideReturnValue(String_t("th"));
        mock.provideReturnValue(String_t("fo"));
        std::auto_ptr<afl::data::Value> p(testee.call(Segment().pushBackString("SNAPSHOTLS")));
        afl::data::Access aa(p.get());
        a.checkEqual("listSnapshots size", aa.getArraySize(), 4U);
        a.checkEqual("listSnapshots result 0", aa[0].toString(), "fi");
        a.checkEqual("listSnapshots result 1", aa[1].toString(), "se");
        a.checkEqual("listSnapshots result 2", aa[2].toString(), "th");
        a.checkEqual("listSnapshots result 3", aa[3].toString(), "fo");
    }
}

/* Test error cases */
AFL_TEST("server.interface.FileSnapshotServer:errors", a)
{
    FileSnapshotMock mock(a);
    server::interface::FileSnapshotServer testee(mock);

    Segment empty;
    AFL_CHECK_THROWS(a("empty"),                testee.call(empty),                                                                           std::exception);
    AFL_CHECK_THROWS(a("bad verb"),             testee.call(Segment().pushBackString("q")),                                                   std::exception);
    AFL_CHECK_THROWS(a("missing arg to add"),   testee.call(Segment().pushBackString("SNAPSHOTADD")),                                         std::exception);
    AFL_CHECK_THROWS(a("missing arg to cp"),    testee.call(Segment().pushBackString("SNAPSHOTCP").pushBackString("x")),                      std::exception);
    AFL_CHECK_THROWS(a("too many args to add"), testee.call(Segment().pushBackString("SNAPSHOTADD").pushBackString("X").pushBackString("Y")), std::exception);
}

/* Test roundtrip with FileSnapshotClient. */
AFL_TEST("server.interface.FileSnapshotServer:roundtrip", a)
{
    FileSnapshotMock mock(a);
    server::interface::FileSnapshotServer level1(mock);
    server::interface::FileSnapshotClient level2(level1);
    server::interface::FileSnapshotServer level3(level2);
    server::interface::FileSnapshotClient level4(level3);

    mock.expectCall("createSnapshot(x)");
    level4.createSnapshot("x");

    mock.expectCall("copySnapshot(i,j)");
    level4.copySnapshot("i", "j");

    mock.expectCall("removeSnapshot(s)");
    level4.removeSnapshot("s");

    mock.expectCall("listSnapshots()");
    mock.provideReturnValue(1);
    mock.provideReturnValue(String_t("e"));
    afl::data::StringList_t out;
    level4.listSnapshots(out);
    a.checkEqual("listSnapshots count",  out.size(), 1U);
    a.checkEqual("listSnapshots result", out[0], "e");
}
