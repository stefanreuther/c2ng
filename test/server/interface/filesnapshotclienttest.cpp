/**
  *  \file test/server/interface/filesnapshotclienttest.cpp
  *  \brief Test for server::interface::FileSnapshotClient
  */

#include "server/interface/filesnapshotclient.hpp"

#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"

using afl::data::Vector;
using afl::data::VectorValue;
using afl::data::Segment;

AFL_TEST("server.interface.FileSnapshotClient", a)
{
    afl::test::CommandHandler mock(a);
    server::interface::FileSnapshotClient testee(mock);

    // createSnapshot
    mock.expectCall("SNAPSHOTADD, nn");
    mock.provideNewResult(0);
    testee.createSnapshot("nn");

    // copySnapshot
    mock.expectCall("SNAPSHOTCP, ff, tt");
    mock.provideNewResult(0);
    testee.copySnapshot("ff", "tt");

    // removeSnapshot
    mock.expectCall("SNAPSHOTRM, xx");
    mock.provideNewResult(0);
    testee.removeSnapshot("xx");

    // listSnapshots
    mock.expectCall("SNAPSHOTLS");
    mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackString("p").pushBackString("q").pushBackString("r"))));
    afl::data::StringList_t out;
    testee.listSnapshots(out);
    a.checkEqual("num results", out.size(), 3U);
    a.checkEqual("result 0", out[0], "p");
    a.checkEqual("result 1", out[1], "q");
    a.checkEqual("result 2", out[2], "r");
}
