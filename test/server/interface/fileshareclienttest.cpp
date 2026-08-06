/**
  *  \file test/server/interface/fileshareclienttest.cpp
  *  \brief Test for server::interface::FileShareClient
  */

#include "server/interface/fileshareclient.hpp"

#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Segment;
using afl::data::Vector;
using afl::data::VectorValue;

namespace {
    HashValue* makeInfo(String_t pathName, String_t owner, int seqNr)
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("path", server::makeStringValue(pathName));
        h->setNew("owner", server::makeStringValue(owner));
        h->setNew("seqNr", server::makeIntegerValue(seqNr));
        return new HashValue(h);
    }

    HashValue* makeGameResponse(String_t path, String_t name)
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("path", server::makeStringValue(path));
        h->setNew("name", server::makeStringValue(name));
        h->setNew("hostversion", server::makeStringValue("Host 2.0"));
        h->setNew("game", server::makeStringValue("7"));
        h->setNew("finished", server::makeStringValue("0"));
        h->setNew("nofilewarning", server::makeStringValue("1"));
        h->setNew("hosttime", server::makeStringValue("12324"));
        h->setNew("missing", new VectorValue(Vector::create(Segment().pushBackString("xyplan.dat"))));
        h->setNew("conflict", new VectorValue(Vector::create(Segment().pushBackInteger(3))));
        h->setNew("races", new VectorValue(Vector::create(Segment().pushBackInteger(1).pushBackString("Fed").pushBackInteger(3).pushBackString("Bird"))));
        return new HashValue(h);
    }
}

AFL_TEST("server.interface.FileShareClient", a)
{
    afl::test::CommandHandler mock(a);
    server::interface::FileShareClient testee(mock);

    // getSequenceNumber
    mock.expectCall("SHARESEQ, uu");
    mock.provideNewResult(server::makeIntegerValue(42));
    a.checkEqual("01. shareseq", testee.getSequenceNumber("uu"), 42);

    // listDirectories
    {
        Vector::Ref_t vv = Vector::create();
        vv->pushBackNew(makeInfo("u/foo/g", "1005", 50));
        vv->pushBackNew(makeInfo("u/bar/x/y", "1108", 51));
        mock.expectCall("SHARELS, aa");
        mock.provideNewResult(new VectorValue(vv));

        afl::container::PtrVector<server::interface::FileShare::Info> result;
        testee.listDirectories("aa", result);

        a.checkEqual("11. sharels size", result.size(), 2U);
        a.checkNonNull("12. sharels 0", result[0]);
        a.checkNonNull("13. sharels 1", result[1]);

        a.checkEqual("21. sharels 0 path",  result[0]->pathName, "u/foo/g");
        a.checkEqual("22. sharels 0 owner", result[0]->owner, "1005");
        a.checkEqual("23. sharels 0 seq",   result[0]->seqNr, 50);

        a.checkEqual("31. sharels 1 path",  result[1]->pathName, "u/bar/x/y");
        a.checkEqual("32. sharels 1 owner", result[1]->owner, "1108");
        a.checkEqual("33. sharels 1 seq",   result[1]->seqNr, 51);
    }

    // listGameInfo
    {
        Vector::Ref_t vv = Vector::create();
        vv->pushBackNew(makeGameResponse("u/foo/g", "shared one"));
        vv->pushBackNew(makeGameResponse("u/bar/x/y", "shared too"));
        mock.expectCall("SHARELSGAME, bb");
        mock.provideNewResult(new VectorValue(vv));

        afl::container::PtrVector<server::interface::FileGame::GameInfo> result;
        testee.listGameInfo("bb", result);

        a.checkEqual("51. sharelsgame size", result.size(), 2U);
        a.checkNonNull("52. sharelsgame 0", result[0]);
        a.checkNonNull("53. sharelsgame 1", result[1]);

        a.checkEqual("61. sharelsgame 0 path",  result[0]->pathName, "u/foo/g");
        a.checkEqual("62. sharelsgame 0 name", result[0]->gameName, "shared one");

        a.checkEqual("71. sharelsgame 1 path",  result[1]->pathName, "u/bar/x/y");
        a.checkEqual("72. sharelsgame 1 name", result[1]->gameName, "shared too");
    }

    mock.checkFinish();
}
