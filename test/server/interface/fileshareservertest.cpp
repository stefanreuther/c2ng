/**
  *  \file test/server/interface/fileshareservertest.cpp
  *  \brief Test for server::interface::FileShareServer
  */

#include "server/interface/fileshareserver.hpp"

#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/filegame.hpp"
#include "server/interface/fileshare.hpp"
#include "server/interface/fileshareclient.hpp"

using afl::data::Access;
using afl::data::Segment;
using afl::string::Format;
using server::interface::FileGame;
using server::interface::FileShare;

namespace {
    FileShare::Info makeInfo(String_t pathName, String_t owner, int seqNr)
    {
        FileShare::Info result;
        result.pathName = pathName;
        result.owner = owner;
        result.seqNr = seqNr;
        return result;
    }

    class FileShareMock : public FileShare, public afl::test::CallReceiver {
     public:
        FileShareMock(afl::test::Assert a)
            : CallReceiver(a)
            { }

        virtual int32_t getSequenceNumber(String_t userId)
            {
                checkCall(Format("getSequenceNumber(%s)", userId));
                return consumeReturnValue<int32_t>();
            }

        virtual void listDirectories(String_t userId, afl::container::PtrVector<Info>& result)
            {
                checkCall(Format("listDirectories(%s)", userId));
                int n = consumeReturnValue<int>();
                for (int i = 0; i < n; ++i) {
                    result.pushBackNew(new Info(consumeReturnValue<Info>()));
                }
            }

        virtual void listGameInfo(String_t userId, afl::container::PtrVector<FileGame::GameInfo>& result)
            {
                checkCall(Format("listGameInfo(%s)", userId));
                int n = consumeReturnValue<int>();
                for (int i = 0; i < n; ++i) {
                    result.pushBackNew(new FileGame::GameInfo(consumeReturnValue<FileGame::GameInfo>()));
                }
            }
    };
}

AFL_TEST("server.interface.FileShareServer:commands", a)
{
    FileShareMock mock(a);
    server::interface::FileShareServer testee(mock);

    // getSequenceNumber
    mock.expectCall("getSequenceNumber(uu)");
    mock.provideReturnValue<int32_t>(66);
    a.checkEqual("01. getSequenceNumber", testee.callInt(Segment().pushBackString("SHARESEQ").pushBackString("uu")), 66);

    // listDirectories
    mock.expectCall("listDirectories(u2)");
    mock.provideReturnValue(3);
    mock.provideReturnValue(makeInfo("u/one", "o1", 45));
    mock.provideReturnValue(makeInfo("u/two", "o2", 47));
    mock.provideReturnValue(makeInfo("u/three", "o3", 48));
    std::auto_ptr<afl::data::Value> dirResult(testee.call(Segment().pushBackString("SHARELS").pushBackString("u2")));
    a.checkEqual("11. length", Access(dirResult).getArraySize(), 3U);
    a.checkEqual("12a. path",  Access(dirResult)[0]("path").toString(), "u/one");
    a.checkEqual("12b. owner", Access(dirResult)[0]("owner").toString(), "o1");
    a.checkEqual("12c. seqNr", Access(dirResult)[0]("seqNr").toInteger(), 45);
    a.checkEqual("13a. path",  Access(dirResult)[1]("path").toString(), "u/two");
    a.checkEqual("13b. owner", Access(dirResult)[1]("owner").toString(), "o2");
    a.checkEqual("13c. seqNr", Access(dirResult)[1]("seqNr").toInteger(), 47);
    a.checkEqual("14a. path",  Access(dirResult)[2]("path").toString(), "u/three");
    a.checkEqual("14b. owner", Access(dirResult)[2]("owner").toString(), "o3");
    a.checkEqual("14c. seqNr", Access(dirResult)[2]("seqNr").toInteger(), 48);

    // listGameInfo
    FileGame::GameInfo gi;
    gi.pathName = "q/1";
    gi.gameName = "g1";
    gi.gameId = 99;
    gi.hostTime = 13579;
    gi.isFinished = false;
    gi.fileWarningDisabled = true;
    mock.expectCall("listGameInfo(uz)");
    mock.provideReturnValue(1);
    mock.provideReturnValue(gi);

    std::auto_ptr<afl::data::Value> gameResult(testee.call(Segment().pushBackString("SHARELSGAME").pushBackString("uz")));
    a.checkEqual("21. length", Access(gameResult).getArraySize(), 1U);
    a.checkEqual("22a. path",  Access(gameResult)[0]("path").toString(), "q/1");
    a.checkEqual("22b. name",  Access(gameResult)[0]("name").toString(), "g1");
    a.checkEqual("22c. game",  Access(gameResult)[0]("game").toInteger(), 99);
    a.checkEqual("22d. warn",  Access(gameResult)[0]("nofilewarning").toInteger(), 1);
}

AFL_TEST("server.interface.FileShareServer:error", a)
{
    FileShareMock mock(a);
    server::interface::FileShareServer testee(mock);

    // Too few args
    AFL_CHECK_THROWS(a("01. missing"), testee.call(Segment().pushBackString("SHARESEQ")), std::exception);
    AFL_CHECK_THROWS(a("02. missing"), testee.call(Segment().pushBackString("SHARELS")), std::exception);
    AFL_CHECK_THROWS(a("03. missing"), testee.call(Segment().pushBackString("SHARELSGAME")), std::exception);

    // Too many args
    AFL_CHECK_THROWS(a("11. too many"), testee.call(Segment().pushBackString("SHARESEQ").pushBackString("u").pushBackString("x")), std::exception);
    AFL_CHECK_THROWS(a("12. too many"), testee.call(Segment().pushBackString("SHARELS").pushBackString("u").pushBackString("x")), std::exception);
    AFL_CHECK_THROWS(a("13. too many"), testee.call(Segment().pushBackString("SHARELSGAME").pushBackString("u").pushBackString("x")), std::exception);

    // Bad name
    AFL_CHECK_THROWS(a("21. bad name"), testee.call(Segment().pushBackString("SHAREIT").pushBackString("uu")), std::exception);
}

AFL_TEST("server.interface.FileShareServer:roundtrip", a)
{
    FileShareMock mock(a);
    server::interface::FileShareServer level1(mock);
    server::interface::FileShareClient level2(level1);
    server::interface::FileShareServer level3(level2);
    server::interface::FileShareClient level4(level3);

    // getSequenceNumber
    mock.expectCall("getSequenceNumber(mnop)");
    mock.provideReturnValue<int32_t>(72);
    a.checkEqual("01. getSequenceNumber", level4.getSequenceNumber("mnop"), 72);

    // listDirectories
    mock.expectCall("listDirectories(qqq)");
    mock.provideReturnValue(2);
    mock.provideReturnValue(makeInfo("u/one", "w1", 145));
    mock.provideReturnValue(makeInfo("u/two", "x2", 147));

    afl::container::PtrVector<FileShare::Info> dirResult;
    level4.listDirectories("qqq", dirResult);

    a.checkEqual("11. length", dirResult.size(), 2U);
    a.checkEqual("12a. path",  dirResult[0]->pathName, "u/one");
    a.checkEqual("12b. owner", dirResult[0]->owner, "w1");
    a.checkEqual("12c. seqNr", dirResult[0]->seqNr, 145);
    a.checkEqual("13a. path",  dirResult[1]->pathName, "u/two");
    a.checkEqual("13b. owner", dirResult[1]->owner, "x2");
    a.checkEqual("13c. seqNr", dirResult[1]->seqNr, 147);

    // listGameInfo
    FileGame::GameInfo gi;
    gi.pathName = "q/1";
    gi.gameName = "g1";
    gi.gameId = 99;
    gi.hostTime = 13579;
    gi.isFinished = false;
    gi.fileWarningDisabled = true;
    mock.expectCall("listGameInfo(uz)");
    mock.provideReturnValue(1);
    mock.provideReturnValue(gi);

    afl::container::PtrVector<FileGame::GameInfo> gameResult;
    level4.listGameInfo("uz", gameResult);
    a.checkEqual("21. length", gameResult.size(), 1U);
    a.checkEqual("22a. path",  gameResult[0]->pathName, "q/1");
    a.checkEqual("22b. name",  gameResult[0]->gameName, "g1");
    a.checkEqual("22c. game",  gameResult[0]->gameId, 99);
    a.checkEqual("22d. warn",  gameResult[0]->fileWarningDisabled, true);
}
