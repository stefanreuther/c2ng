/**
  *  \file test/server/interface/filegameclienttest.cpp
  *  \brief Test for server::interface::FileGameClient
  */

#include "server/interface/filegameclient.hpp"

#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::IntegerValue;
using afl::data::Segment;
using afl::data::StringValue;
using afl::data::Vector;
using afl::data::VectorValue;

namespace {
    Hash::Ref_t makeGameResponse(String_t path, String_t name)
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("path", new StringValue(path));
        h->setNew("name", new StringValue(name));
        h->setNew("hostversion", new StringValue("Host 2.0"));
        h->setNew("game", new StringValue("7"));
        h->setNew("finished", new StringValue("0"));
        h->setNew("hosttime", new StringValue("12324"));
        h->setNew("missing", new VectorValue(Vector::create(Segment().pushBackString("xyplan.dat"))));
        h->setNew("conflict", new VectorValue(Vector::create(Segment().pushBackInteger(3))));
        h->setNew("races", new VectorValue(Vector::create(Segment().pushBackInteger(1).pushBackString("Fed").pushBackInteger(3).pushBackString("Bird"))));
        return h;
    }

    Hash::Ref_t makeKeyResponse(String_t path, String_t label1, String_t label2)
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("path", new StringValue(path));
        h->setNew("file", new StringValue(path + "/fizz.bin"));
        h->setNew("reg", new StringValue("1"));
        h->setNew("key1", new StringValue(label1));
        h->setNew("key2", new StringValue(label2));
        return h;
    }

    Hash::Ref_t makeFullKeyResponse(Hash::Ref_t h, int useCount, String_t keyId)
    {
        h->setNew("useCount", new IntegerValue(useCount));
        h->setNew("id", new StringValue(keyId));
        return h;
    }
}

AFL_TEST("server.interface.FileGameClient", a)
{
    using server::interface::FileGame;

    afl::test::CommandHandler mock(a);
    server::interface::FileGameClient testee(mock);

    // getGameInfo - null answer
    {
        mock.expectCall("STATGAME, a/b");
        mock.provideNewResult(0);

        FileGame::GameInfo gi;
        AFL_CHECK_SUCCEEDS(a("01. getGameInfo"), testee.getGameInfo("a/b", gi));
        a.checkEqual("02. gameName",      gi.gameName, "");
        a.checkEqual("03. pathName",      gi.pathName, "");
        a.checkEqual("04. gameId",        gi.gameId, 0);
        a.checkEqual("05. missingFiles",  gi.missingFiles.size(), 0U);
        a.checkEqual("06. conflictSlots", gi.conflictSlots.size(), 0U);
        a.checkEqual("07. slots",         gi.slots.size(), 0U);
        a.checkEqual("08. isFinished",    gi.isFinished, false);
    }

    // getGameInfo - real answer
    {
        mock.expectCall("STATGAME, x/y/z");
        mock.provideNewResult(new HashValue(makeGameResponse("x/y/z/a", "Game A")));

        FileGame::GameInfo gi;
        AFL_CHECK_SUCCEEDS(a("11. getGameInfo"), testee.getGameInfo("x/y/z", gi));
        a.checkEqual("12. gameName",      gi.gameName, "Game A");
        a.checkEqual("13. pathName",      gi.pathName, "x/y/z/a");
        a.checkEqual("14. hostVersion",   gi.hostVersion, "Host 2.0");
        a.checkEqual("15. gameId",        gi.gameId, 7);
        a.checkEqual("16. missingFiles",  gi.missingFiles.size(), 1U);
        a.checkEqual("17. missingFiles",  gi.missingFiles[0], "xyplan.dat");
        a.checkEqual("18. conflictSlots", gi.conflictSlots.size(), 1U);
        a.checkEqual("19. conflictSlots", gi.conflictSlots[0], 3);
        a.checkEqual("20. slots",         gi.slots.size(), 2U);
        a.checkEqual("21. slots",         gi.slots[0].first, 1);
        a.checkEqual("22. slots",         gi.slots[0].second, "Fed");
        a.checkEqual("23. slots",         gi.slots[1].first, 3);
        a.checkEqual("24. slots",         gi.slots[1].second, "Bird");
        a.checkEqual("25. isFinished",    gi.isFinished, false);
    }

    // getGameInfo - answer with bogus value (must not crash)
    {
        Hash::Ref_t h = makeGameResponse("x/y/z/a", "Game A");
        h->setNew("game", new StringValue("blub"));
        mock.expectCall("STATGAME, x/y/z");
        mock.provideNewResult(new HashValue(h));

        FileGame::GameInfo gi;
        AFL_CHECK_SUCCEEDS(a("31. getGameInfo"), testee.getGameInfo("x/y/z", gi));
        a.checkEqual("32. gameName",      gi.gameName, "Game A");
        a.checkEqual("33. pathName",      gi.pathName, "x/y/z/a");
        a.checkEqual("34. hostVersion",   gi.hostVersion, "Host 2.0");
        a.checkEqual("35. gameId",        gi.gameId, 0);
        a.checkEqual("36. missingFiles",  gi.missingFiles.size(), 1U);
        a.checkEqual("37. conflictSlots", gi.conflictSlots.size(), 1U);
        a.checkEqual("38. slots",         gi.slots.size(), 2U);
        a.checkEqual("39. isFinished",    gi.isFinished, false);
    }

    // listGameInfo - null answer
    {
        mock.expectCall("LSGAME, a/b");
        mock.provideNewResult(0);

        afl::container::PtrVector<FileGame::GameInfo> result;
        AFL_CHECK_SUCCEEDS(a("41. listGameInfo"), testee.listGameInfo("a/b", result));
        a.checkEqual("42. size", result.size(), 0U);
    }

    // listGameInfo - real answer
    {
        mock.expectCall("LSGAME, z");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().
                                                               pushBackNew(new HashValue(makeGameResponse("z/1", "Game One"))).
                                                               pushBackNew(new HashValue(makeGameResponse("z/2", "Game Two"))).
                                                               pushBackNew(new HashValue(makeGameResponse("z/3/a", "Game Three A"))))));

        afl::container::PtrVector<FileGame::GameInfo> result;
        AFL_CHECK_SUCCEEDS(a("51. listGameInfo"), testee.listGameInfo("z", result));
        a.checkEqual  ("52. size",     result.size(), 3U);
        a.checkNonNull("53. result",   result[0]);
        a.checkEqual  ("54. gameName", result[0]->gameName, "Game One");
        a.checkEqual  ("55. pathName", result[0]->pathName, "z/1");
        a.checkNonNull("56. result",   result[1]);
        a.checkEqual  ("57. gameName", result[1]->gameName, "Game Two");
        a.checkEqual  ("58. pathName", result[1]->pathName, "z/2");
        a.checkNonNull("59. result",   result[2]);
        a.checkEqual  ("60. gameName", result[2]->gameName, "Game Three A");
        a.checkEqual  ("61. pathName", result[2]->pathName, "z/3/a");
    }

    // listGameInfo - mixed answer (produces empty game)
    {
        mock.expectCall("LSGAME, zq");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().
                                                               pushBackNew(0).
                                                               pushBackNew(new HashValue(makeGameResponse("zq/qq", "Q"))))));

        afl::container::PtrVector<FileGame::GameInfo> result;
        AFL_CHECK_SUCCEEDS(a("71. listGameInfo"), testee.listGameInfo("zq", result));
        a.checkEqual  ("72. size",     result.size(), 2U);
        a.checkNonNull("73. result",   result[0]);
        a.checkEqual  ("74. gameName", result[0]->gameName, "");
        a.checkEqual  ("75. pathName", result[0]->pathName, "");
        a.checkNonNull("76. result",   result[1]);
        a.checkEqual  ("77. gameName", result[1]->gameName, "Q");
        a.checkEqual  ("78. pathName", result[1]->pathName, "zq/qq");
    }

    // getKeyInfo - null answer
    {
        mock.expectCall("STATREG, r");
        mock.provideNewResult(0);

        FileGame::KeyInfo result;
        AFL_CHECK_SUCCEEDS(a("81. getKeyInfo"), testee.getKeyInfo("r", result));
        a.checkEqual("82. pathName",     result.pathName, "");
        a.checkEqual("83. fileName",     result.fileName, "");
        a.checkEqual("84. isRegistered", result.isRegistered, false);
        a.checkEqual("85. label1",       result.label1, "");
        a.checkEqual("86. label2",       result.label2, "");
        a.check("87. useCount",         !result.useCount.isValid());
        a.check("88. keyId",            !result.keyId.isValid());
    }

    // getKeyInfo - real answer
    {
        mock.expectCall("STATREG, r2");
        mock.provideNewResult(new HashValue(makeKeyResponse("r2", "Name", "Address")));

        FileGame::KeyInfo result;
        AFL_CHECK_SUCCEEDS(a("91. getKeyInfo"), testee.getKeyInfo("r2", result));
        a.checkEqual("92. pathName",     result.pathName, "r2");
        a.checkEqual("93. fileName",     result.fileName, "r2/fizz.bin");
        a.checkEqual("94. isRegistered", result.isRegistered, true);
        a.checkEqual("95. label1",       result.label1, "Name");
        a.checkEqual("96. label2",       result.label2, "Address");
        a.check("97. useCount",         !result.useCount.isValid());
        a.check("98. keyId",            !result.keyId.isValid());
    }

    // getKeyInfo - full answer
    {
        mock.expectCall("STATREG, r2");
        mock.provideNewResult(new HashValue(makeFullKeyResponse(makeKeyResponse("r2", "Name", "Address"), 17, "a1b2c3d4")));

        FileGame::KeyInfo result;
        AFL_CHECK_SUCCEEDS(a("101. getKeyInfo"), testee.getKeyInfo("r2", result));
        a.checkEqual("102. pathName",     result.pathName, "r2");
        a.checkEqual("103. fileName",     result.fileName, "r2/fizz.bin");
        a.checkEqual("104. isRegistered", result.isRegistered, true);
        a.checkEqual("105. label1",       result.label1, "Name");
        a.checkEqual("106. label2",       result.label2, "Address");
        a.checkEqual("107. useCount",     result.useCount.orElse(-1), 17);
        a.checkEqual("108. keyId",        result.keyId.orElse("-"), "a1b2c3d4");
    }

    // getKeyInfo - answer with bogus value (must not crash)
    {
        Hash::Ref_t h = makeKeyResponse("r2", "Name", "Address");
        h->setNew("reg", new StringValue("wut?"));
        mock.expectCall("STATREG, r2");
        mock.provideNewResult(new HashValue(h));

        FileGame::KeyInfo result;
        AFL_CHECK_SUCCEEDS(a("111. getKeyInfo"), testee.getKeyInfo("r2", result));
        a.checkEqual("112. pathName",     result.pathName, "r2");
        a.checkEqual("113. fileName",     result.fileName, "r2/fizz.bin");
        a.checkEqual("114. isRegistered", result.isRegistered, false);
        a.checkEqual("115. label1",       result.label1, "Name");
        a.checkEqual("116. label2",       result.label2, "Address");
    }

    // listKeyInfo - null answer
    {
        mock.expectCall("LSREG, r3");
        mock.provideNewResult(0);

        afl::container::PtrVector<FileGame::KeyInfo> result;
        AFL_CHECK_SUCCEEDS(a("121. listKeyInfo"), testee.listKeyInfo("r3", FileGame::Filter(), result));
        a.checkEqual("122. size", result.size(), 0U);
    }

    // listKeyInfo - real answer
    {
        mock.expectCall("LSREG, z");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().
                                                               pushBackNew(new HashValue(makeKeyResponse("z/1", "Key One", "Adr 1"))).
                                                               pushBackNew(new HashValue(makeKeyResponse("z/2", "Key Two", "Adr 2"))).
                                                               pushBackNew(new HashValue(makeKeyResponse("z/3/a", "Key Three A", "Adr 3a"))))));

        afl::container::PtrVector<FileGame::KeyInfo> result;
        AFL_CHECK_SUCCEEDS(a("131. listKeyInfo"), testee.listKeyInfo("z", FileGame::Filter(), result));
        a.checkEqual  ("132. size", result.size(), 3U);
        a.checkNonNull("133. result",   result[0]);
        a.checkEqual  ("134. label1",   result[0]->label1, "Key One");
        a.checkEqual  ("135. pathName", result[0]->pathName, "z/1");
        a.checkNonNull("136. result",   result[1]);
        a.checkEqual  ("137. label1",   result[1]->label1, "Key Two");
        a.checkEqual  ("138. pathName", result[1]->pathName, "z/2");
        a.checkNonNull("139. result",   result[2]);
        a.checkEqual  ("140. label1",   result[2]->label1, "Key Three A");
        a.checkEqual  ("141. pathName", result[2]->pathName, "z/3/a");
    }

    // listKeyInfo - options
    {
        mock.expectCall("LSREG, r3, ID, f5g6h7");
        mock.provideNewResult(0);

        FileGame::Filter f;
        f.keyId = "f5g6h7";
        afl::container::PtrVector<FileGame::KeyInfo> result;
        AFL_CHECK_SUCCEEDS(a("151. listKeyInfo"), testee.listKeyInfo("r3", f, result));
        a.checkEqual("152. size", result.size(), 0U);
    }

    // listKeyInfo - options
    {
        mock.expectCall("LSREG, r3, UNIQ");
        mock.provideNewResult(0);

        FileGame::Filter f;
        f.unique = true;
        afl::container::PtrVector<FileGame::KeyInfo> result;
        AFL_CHECK_SUCCEEDS(a("161. listKeyInfo"), testee.listKeyInfo("r3", f, result));
        a.checkEqual("162. size", result.size(), 0U);
    }
}
