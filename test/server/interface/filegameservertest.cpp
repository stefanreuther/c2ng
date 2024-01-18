/**
  *  \file test/server/interface/filegameservertest.cpp
  *  \brief Test for server::interface::FileGameServer
  */

#include "server/interface/filegameserver.hpp"

#include "afl/data/access.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/filegameclient.hpp"
#include <stdexcept>

using afl::string::Format;
using afl::data::Access;
using afl::data::Segment;
using afl::data::Value;
using server::interface::FileGame;

namespace {
    class FileGameMock : public server::interface::FileGame, public afl::test::CallReceiver {
     public:
        FileGameMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        void getGameInfo(String_t path, GameInfo& result)
            {
                checkCall(Format("getGameInfo(%s)", path));
                result = consumeReturnValue<GameInfo>();
            }
        void listGameInfo(String_t path, afl::container::PtrVector<GameInfo>& result)
            {
                checkCall(Format("listGameInfo(%s)", path));
                size_t n = consumeReturnValue<size_t>();
                while (n-- > 0) {
                    result.pushBackNew(consumeReturnValue<GameInfo*>());
                }
            }
        void getKeyInfo(String_t path, KeyInfo& result)
            {
                checkCall(Format("getKeyInfo(%s)", path));
                result = consumeReturnValue<KeyInfo>();
            }
        void listKeyInfo(String_t path, const Filter& filter, afl::container::PtrVector<KeyInfo>& result)
            {
                checkCall(Format("listKeyInfo(%s,%s,%d)", path, filter.keyId.orElse("-"), int(filter.unique)));
                size_t n = consumeReturnValue<size_t>();
                while (n-- > 0) {
                    result.pushBackNew(consumeReturnValue<KeyInfo*>());
                }
            }
    };
}

/** Simple test. */
AFL_TEST("server.interface.FileGameServer:commands", a)
{
    FileGameMock mock(a);
    server::interface::FileGameServer testee(mock);

    // getGameInfo
    {
        FileGame::GameInfo gi;
        gi.pathName = "p";
        gi.gameName = "g";
        gi.hostVersion = "Gh 3";
        gi.gameId = 99;
        gi.hostTime = 13579;
        gi.isFinished = false;
        gi.slots.push_back(FileGame::Slot_t(2, "Liz"));
        gi.slots.push_back(FileGame::Slot_t(9, "Bot"));
        gi.missingFiles.push_back("race.nm");
        gi.conflictSlots.push_back(2);
        gi.conflictSlots.push_back(3);
        gi.conflictSlots.push_back(5);

        mock.expectCall("getGameInfo(pp)");
        mock.provideReturnValue<FileGame::GameInfo>(gi);

        std::auto_ptr<Value> p(testee.call(Segment().pushBackString("STATGAME").pushBackString("pp")));
        a.checkNonNull("01. STATGAME", p.get());

        Access ap(p);
        a.checkEqual("11. path",        ap("path").toString(), "p");
        a.checkEqual("12. name",        ap("name").toString(), "g");
        a.checkEqual("13. hostversion", ap("hostversion").toString(), "Gh 3");
        a.checkEqual("14. game",        ap("game").toInteger(), 99);
        a.checkEqual("15. hosttime",    ap("hosttime").toInteger(), 13579);
        a.checkEqual("16. finished",    ap("finished").toInteger(), 0);
        a.checkEqual("17. races",       ap("races").getArraySize(), 4U);
        a.checkEqual("18. races",       ap("races")[0].toInteger(), 2);
        a.checkEqual("19. races",       ap("races")[1].toString(), "Liz");
        a.checkEqual("20. races",       ap("races")[2].toInteger(), 9);
        a.checkEqual("21. races",       ap("races")[3].toString(), "Bot");
        a.checkEqual("22. missing",     ap("missing").getArraySize(), 1U);
        a.checkEqual("23. missing",     ap("missing")[0].toString(), "race.nm");
        a.checkEqual("24. conflict",    ap("conflict").getArraySize(), 3U);
        a.checkEqual("25. conflict",    ap("conflict")[0].toInteger(), 2);
        a.checkEqual("26. conflict",    ap("conflict")[1].toInteger(), 3);
        a.checkEqual("27. conflict",    ap("conflict")[2].toInteger(), 5);
        mock.checkFinish();
    }

    // listGameInfo
    {
        std::auto_ptr<FileGame::GameInfo> gi;
        mock.provideReturnValue<size_t>(2);

        gi.reset(new FileGame::GameInfo());
        gi->pathName = "q/1";
        gi->gameName = "g1";
        gi->gameId = 99;
        gi->hostTime = 13579;
        gi->isFinished = false;
        mock.provideReturnValue<FileGame::GameInfo*>(gi.release());

        gi.reset(new FileGame::GameInfo());
        gi->pathName = "q/2";
        gi->gameName = "g2";
        gi->gameId = 77;
        gi->hostTime = 0;
        gi->isFinished = true;
        mock.provideReturnValue<FileGame::GameInfo*>(gi.release());

        mock.expectCall("listGameInfo(q)");

        std::auto_ptr<Value> p(testee.call(Segment().pushBackString("LSGAME").pushBackString("q")));
        a.checkNonNull("31. LSGAME", p.get());

        Access ap(p);
        a.checkEqual("41. getArraySize", ap.getArraySize(), 2U);
        a.checkEqual("42. path",     ap[0]("path").toString(), "q/1");
        a.checkEqual("43. finished", ap[0]("finished").toInteger(), 0);
        a.checkEqual("44. path",     ap[1]("path").toString(), "q/2");
        a.checkEqual("45. finished", ap[1]("finished").toInteger(), 1);

        mock.checkFinish();
    }

    // getKeyInfo (classic)
    {
        FileGame::KeyInfo ki;
        ki.pathName = "a/k";
        ki.fileName = "a/k/keyfile";
        ki.isRegistered = true;
        ki.label1 = "L1";
        ki.label2 = "L2";

        mock.expectCall("getKeyInfo(a/k)");
        mock.provideReturnValue<FileGame::KeyInfo>(ki);

        std::auto_ptr<Value> p(testee.call(Segment().pushBackString("STATREG").pushBackString("a/k")));
        a.checkNonNull("51. STATREG", p.get());

        Access ap(p);
        a.checkEqual("61. path",     ap("path").toString(), "a/k");
        a.checkEqual("62. file",     ap("file").toString(), "a/k/keyfile");
        a.checkEqual("63. reg",      ap("reg").toInteger(), 1);
        a.checkEqual("64. key1",     ap("key1").toString(), "L1");
        a.checkEqual("65. key2",     ap("key2").toString(), "L2");
        a.check     ("66. useCount", ap("useCount").isNull());
        a.check     ("67. id",       ap("id").isNull());

        mock.checkFinish();
    }

    // getKeyInfo (full)
    {
        FileGame::KeyInfo ki;
        ki.pathName = "a/k";
        ki.fileName = "a/k/keyfile";
        ki.isRegistered = true;
        ki.label1 = "L1";
        ki.label2 = "L2";
        ki.useCount = 32;
        ki.keyId = String_t("ididid");

        mock.expectCall("getKeyInfo(a/k)");
        mock.provideReturnValue<FileGame::KeyInfo>(ki);

        std::auto_ptr<Value> p(testee.call(Segment().pushBackString("STATREG").pushBackString("a/k")));
        a.checkNonNull("71. STATREG", p.get());

        Access ap(p);
        a.checkEqual("81. path",     ap("path").toString(), "a/k");
        a.checkEqual("82. file",     ap("file").toString(), "a/k/keyfile");
        a.checkEqual("83. reg",      ap("reg").toInteger(), 1);
        a.checkEqual("84. key1",     ap("key1").toString(), "L1");
        a.checkEqual("85. key2",     ap("key2").toString(), "L2");
        a.checkEqual("86. useCount", ap("useCount").toInteger(), 32);
        a.checkEqual("87. id",       ap("id").toString(), "ididid");

        mock.checkFinish();
    }

    // listKeyInfo
    {
        std::auto_ptr<FileGame::KeyInfo> ki;
        mock.provideReturnValue<size_t>(3);

        ki.reset(new FileGame::KeyInfo());
        ki->pathName = "r/p1";
        ki->isRegistered = true;
        mock.provideReturnValue<FileGame::KeyInfo*>(ki.release());

        ki.reset(new FileGame::KeyInfo());
        ki->pathName = "r/p2";
        ki->isRegistered = true;
        mock.provideReturnValue<FileGame::KeyInfo*>(ki.release());

        ki.reset(new FileGame::KeyInfo());
        ki->pathName = "r/sw";
        ki->isRegistered = false;
        mock.provideReturnValue<FileGame::KeyInfo*>(ki.release());

        mock.expectCall("listKeyInfo(r,-,0)");

        std::auto_ptr<Value> p(testee.call(Segment().pushBackString("LSREG").pushBackString("r")));
        a.checkNonNull("91. LSREG", p.get());

        Access ap(p);
        a.checkEqual("101. getArraySize", ap.getArraySize(), 3U);
        a.checkEqual("102. path", ap[0]("path").toString(), "r/p1");
        a.checkEqual("103. reg",  ap[0]("reg").toInteger(), true);
        a.checkEqual("104. path", ap[1]("path").toString(), "r/p2");
        a.checkEqual("105. reg",  ap[1]("reg").toInteger(), true);
        a.checkEqual("106. path", ap[2]("path").toString(), "r/sw");
        a.checkEqual("107. reg",  ap[2]("reg").toInteger(), false);

        mock.checkFinish();
    }

    // listKeyInfo with options
    {
        mock.provideReturnValue<size_t>(0);
        mock.expectCall("listKeyInfo(r,kid,0)");

        std::auto_ptr<Value> p(testee.call(Segment().pushBackString("LSREG").pushBackString("r").pushBackString("ID").pushBackString("kid")));
        a.checkNonNull("111. LSREG", p.get());
        mock.checkFinish();
    }

    // listKeyInfo with options
    {
        mock.provideReturnValue<size_t>(0);
        mock.expectCall("listKeyInfo(r,-,1)");

        std::auto_ptr<Value> p(testee.call(Segment().pushBackString("LSREG").pushBackString("r").pushBackString("UNIQ")));
        a.checkNonNull("121. LSREG", p.get());
        mock.checkFinish();
    }

    // Variants
    mock.expectCall("listKeyInfo(zz,-,0)");
    mock.provideReturnValue<size_t>(0);
    testee.callVoid(Segment().pushBackString("lsreg").pushBackString("zz"));
    mock.checkFinish();
}

/** Test errors. */
AFL_TEST("server.interface.FileGameServer:errors", a)
{
    FileGameMock mock(a);
    server::interface::FileGameServer testee(mock);

    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    AFL_CHECK_THROWS(a("01. empty"),        testee.callVoid(empty), std::exception);
    AFL_CHECK_THROWS(a("02. bad verb"),     testee.callVoid(Segment().pushBackString("BADCMD")), std::exception);
    AFL_CHECK_THROWS(a("03. missing args"), testee.callVoid(Segment().pushBackString("LSREG")), std::exception);
    AFL_CHECK_THROWS(a("04. bad option"),   testee.callVoid(Segment().pushBackString("LSREG").pushBackString("a").pushBackString("b")), std::exception);
    AFL_CHECK_THROWS(a("05. missing args"), testee.callVoid(Segment().pushBackString("LSREG").pushBackString("a").pushBackString("ID")), std::exception);

    // ComposableCommandHandler personality
    interpreter::Arguments args(empty, 0, 0);
    std::auto_ptr<afl::data::Value> p;
    a.checkEqual("11. bad verb", testee.handleCommand("huhu", args, p), false);
}

/** Test roundtrip behaviour. */
AFL_TEST("server.interface.FileGameServer:roundtrip", a)
{
    FileGameMock mock(a);
    server::interface::FileGameServer level1(mock);
    server::interface::FileGameClient level2(level1);
    server::interface::FileGameServer level3(level2);
    server::interface::FileGameClient level4(level3);

    // getGameInfo
    {
        FileGame::GameInfo gi;
        gi.pathName = "p";
        gi.gameName = "g";
        gi.hostVersion = "HV 2.0";
        gi.gameId = 99;
        gi.hostTime = 13579;
        gi.isFinished = false;
        gi.slots.push_back(FileGame::Slot_t(2, "Liz"));
        gi.missingFiles.push_back("race.nm");
        gi.conflictSlots.push_back(5);

        mock.expectCall("getGameInfo(pp)");
        mock.provideReturnValue<FileGame::GameInfo>(gi);

        FileGame::GameInfo out;
        AFL_CHECK_SUCCEEDS(a("01. getGameInfo"), level4.getGameInfo("pp", out));
        a.checkEqual("02. pathName",      out.pathName, "p");
        a.checkEqual("03. gameName",      out.gameName, "g");
        a.checkEqual("04. hostVersion",   out.hostVersion, "HV 2.0");
        a.checkEqual("05. gameId",        out.gameId, 99);
        a.checkEqual("06. hostTime",      out.hostTime, 13579);
        a.checkEqual("07. isFinished",    out.isFinished, false);
        a.checkEqual("08. slots",         out.slots.size(), 1U);
        a.checkEqual("09. slots",         out.slots[0].first, 2);
        a.checkEqual("10. slots",         out.slots[0].second, "Liz");
        a.checkEqual("11. missingFiles",  out.missingFiles.size(), 1U);
        a.checkEqual("12. missingFiles",  out.missingFiles[0], "race.nm");
        a.checkEqual("13. conflictSlots", out.conflictSlots.size(), 1U);
        a.checkEqual("14. conflictSlots", out.conflictSlots[0], 5);
        mock.checkFinish();
    }

    // listGameInfo
    {
        std::auto_ptr<FileGame::GameInfo> gi;
        mock.provideReturnValue<size_t>(1);

        gi.reset(new FileGame::GameInfo());
        gi->pathName = "q/1";
        gi->gameName = "g1";
        gi->gameId = 99;
        gi->hostTime = 13579;
        gi->isFinished = false;
        mock.provideReturnValue<FileGame::GameInfo*>(gi.release());

        mock.expectCall("listGameInfo(q)");

        afl::container::PtrVector<FileGame::GameInfo> out;
        AFL_CHECK_SUCCEEDS(a("21. listGameInfo"), level4.listGameInfo("q", out));

        a.checkEqual("31. size",     out.size(), 1U);
        a.checkNonNull("32. out",    out[0]);
        a.checkEqual("33. pathName", out[0]->pathName, "q/1");
        a.checkEqual("34. gameId",   out[0]->gameId, 99);

        mock.checkFinish();
    }

    // getKeyInfo (classic)
    {
        FileGame::KeyInfo ki;
        ki.pathName = "e/k";
        ki.fileName = "e/k/keyfile";
        ki.isRegistered = true;
        ki.label1 = "e1";
        ki.label2 = "e2";

        mock.expectCall("getKeyInfo(e/k)");
        mock.provideReturnValue<FileGame::KeyInfo>(ki);

        FileGame::KeyInfo out;
        AFL_CHECK_SUCCEEDS(a("41. getKeyInfo"), level4.getKeyInfo("e/k", out));
        a.checkEqual("42. pathName",     out.pathName, "e/k");
        a.checkEqual("43. fileName",     out.fileName, "e/k/keyfile");
        a.checkEqual("44. isRegistered", out.isRegistered, true);
        a.checkEqual("45. label1",       out.label1, "e1");
        a.checkEqual("46. label2",       out.label2, "e2");
        a.check     ("47. useCount",    !out.useCount.isValid());
        a.check     ("48. keyId",       !out.keyId.isValid());

        mock.checkFinish();
    }

    // getKeyInfo (full)
    {
        FileGame::KeyInfo ki;
        ki.pathName = "e/k";
        ki.fileName = "e/k/keyfile";
        ki.isRegistered = true;
        ki.label1 = "e1";
        ki.label2 = "e2";
        ki.useCount = 44;
        ki.keyId = "kid";

        mock.expectCall("getKeyInfo(e/k)");
        mock.provideReturnValue<FileGame::KeyInfo>(ki);

        FileGame::KeyInfo out;
        AFL_CHECK_SUCCEEDS(a("51. getKeyInfo"), level4.getKeyInfo("e/k", out));
        a.checkEqual("52. pathName",     out.pathName, "e/k");
        a.checkEqual("53. fileName",     out.fileName, "e/k/keyfile");
        a.checkEqual("54. isRegistered", out.isRegistered, true);
        a.checkEqual("55. label1",       out.label1, "e1");
        a.checkEqual("56. label2",       out.label2, "e2");
        a.checkEqual("57. useCount",     out.useCount.orElse(-1), 44);
        a.checkEqual("58. keyId",        out.keyId.orElse(""), "kid");

        mock.checkFinish();
    }

    // listKeyInfo
    {
        std::auto_ptr<FileGame::KeyInfo> ki;
        mock.provideReturnValue<size_t>(2);

        ki.reset(new FileGame::KeyInfo());
        ki->pathName = "r/p1";
        ki->isRegistered = true;
        mock.provideReturnValue<FileGame::KeyInfo*>(ki.release());

        ki.reset(new FileGame::KeyInfo());
        ki->pathName = "r/sw";
        ki->isRegistered = false;
        mock.provideReturnValue<FileGame::KeyInfo*>(ki.release());

        mock.expectCall("listKeyInfo(r,-,0)");

        afl::container::PtrVector<FileGame::KeyInfo> out;
        AFL_CHECK_SUCCEEDS(a("61. listKeyInfo"), level4.listKeyInfo("r", FileGame::Filter(), out));

        a.checkEqual  ("71. size", out.size(), 2U);
        a.checkNonNull("72. result",       out[0]);
        a.checkEqual  ("73. pathName",     out[0]->pathName, "r/p1");
        a.checkEqual  ("74. isRegistered", out[0]->isRegistered, true);
        a.checkNonNull("75. result",       out[1]);
        a.checkEqual  ("76. pathName",     out[1]->pathName, "r/sw");
        a.checkEqual  ("77. isRegistered", out[1]->isRegistered, false);

        mock.checkFinish();
    }

    // listKeyInfo with options
    {
        mock.provideReturnValue<size_t>(0);
        mock.expectCall("listKeyInfo(r,kkkk,1)");

        FileGame::Filter f;
        f.keyId = "kkkk";
        f.unique = true;
        afl::container::PtrVector<FileGame::KeyInfo> out;
        AFL_CHECK_SUCCEEDS(a("81. listKeyInfo"), level4.listKeyInfo("r", f, out));

        a.checkEqual("91. size", out.size(), 0U);
        mock.checkFinish();
    }
}
