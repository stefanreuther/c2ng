/**
  *  \file test/server/interface/hostgameclienttest.cpp
  *  \brief Test for server::interface::HostGameClient
  */

#include "server/interface/hostgameclient.hpp"

#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/except/invaliddataexception.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"

using afl::data::Vector;
using afl::data::VectorValue;
using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Segment;

namespace {
    server::Value_t* makeGameInfo()
    {
        // A (partial) schedule
        Hash::Ref_t sch = Hash::create();
        sch->setNew("type",      server::makeIntegerValue(1));        // WEEKLY
        sch->setNew("weekdays",  server::makeIntegerValue(19));
        sch->setNew("interval",  server::makeIntegerValue(6));
        sch->setNew("daytime",   server::makeIntegerValue(1400));

        // A game
        Hash::Ref_t h = Hash::create();
        h->setNew("id",                  server::makeIntegerValue(43));
        h->setNew("state",               server::makeStringValue("joining"));
        h->setNew("type",                server::makeStringValue("public"));
        h->setNew("name",                server::makeStringValue("The Name"));
        h->setNew("description",         server::makeStringValue("A test game"));
        h->setNew("difficulty",          server::makeIntegerValue(133));
        h->setNew("currentSchedule",     new HashValue(sch));
        h->setNew("slots",               new VectorValue(Vector::create(Segment().pushBackString("open").pushBackString("occupied").pushBackString("self"))));
        h->setNew("turns",               new VectorValue(Vector::create(Segment().pushBackInteger(0).pushBackInteger(2).pushBackInteger(1))));
        h->setNew("joinable",            server::makeIntegerValue(0));
        h->setNew("userPlays",           server::makeIntegerValue(1));
        h->setNew("scores",              new VectorValue(Vector::create(Segment().pushBackInteger(130).pushBackInteger(140).pushBackInteger(135))));
        h->setNew("scoreName",           server::makeStringValue("test"));
        h->setNew("scoreDescription",    server::makeStringValue("Test Score"));
        h->setNew("minRankLevelToJoin",  server::makeIntegerValue(3));
        h->setNew("maxRankLevelToJoin",  server::makeIntegerValue(4));
        h->setNew("minRankPointsToJoin", server::makeIntegerValue(5));
        h->setNew("maxRankPointsToJoin", server::makeIntegerValue(6));
        h->setNew("host",                server::makeStringValue("thost"));
        h->setNew("hostDescription",     server::makeStringValue("Tim Host"));
        h->setNew("hostKind",            server::makeStringValue("th"));
        h->setNew("shiplist",            server::makeStringValue("plist2"));
        h->setNew("shiplistDescription", server::makeStringValue("PList 2"));
        h->setNew("shiplistKind",        server::makeStringValue("plist"));
        h->setNew("master",              server::makeStringValue("xmaster"));
        h->setNew("masterDescription",   server::makeStringValue("Master X"));
        h->setNew("masterKind",          server::makeStringValue("mak"));
        h->setNew("turn",                server::makeIntegerValue(2));
        h->setNew("lastHostTime",        server::makeIntegerValue(15354520));
        h->setNew("nextHostTime",        server::makeIntegerValue(15356789));
        h->setNew("forum",               server::makeIntegerValue(65));
        h->setNew("userRank",            server::makeIntegerValue(3));
        h->setNew("otherRank",           server::makeIntegerValue(7));

        return new HashValue(h);
    }
}


/** Test simple commands. */
AFL_TEST("server.interface.HostGameClient:commands", a)
{
    using server::interface::HostGame;
    afl::test::CommandHandler mock(a);
    server::interface::HostGameClient testee(mock);

    // NEWGAME
    mock.expectCall("NEWGAME");
    mock.provideNewResult(server::makeIntegerValue(12));
    a.checkEqual("01. createNewGame", testee.createNewGame(), 12);

    // CLONEGAME
    mock.expectCall("CLONEGAME, 2");
    mock.provideNewResult(server::makeIntegerValue(9));
    a.checkEqual("11. cloneGame", testee.cloneGame(2, afl::base::Nothing), 9);
    mock.expectCall("CLONEGAME, 7, joining");
    mock.provideNewResult(server::makeIntegerValue(10));
    a.checkEqual("12. cloneGame", testee.cloneGame(7, HostGame::Joining), 10);

    // GAMESETTYPE
    mock.expectCall("GAMESETTYPE, 10, unlisted");
    mock.provideNewResult(0);
    testee.setType(10, HostGame::UnlistedGame);

    // GAMESETSTATE
    mock.expectCall("GAMESETSTATE, 10, running");
    mock.provideNewResult(0);
    testee.setState(10, HostGame::Running);

    // GAMESETOWNER
    mock.expectCall("GAMESETOWNER, 7, 1001");
    mock.provideNewResult(0);
    testee.setOwner(7, "1001");

    // GAMESETNAME
    mock.expectCall("GAMESETNAME, 5, Game Five");
    mock.provideNewResult(0);
    testee.setName(5, "Game Five");

    // GAMELIST ID
    {
        afl::data::IntegerList_t result;
        mock.expectCall("GAMELIST, ID");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackInteger(2).pushBackInteger(3).pushBackInteger(5))));
        HostGame::Filter filter;
        filter.requiredState = afl::base::Nothing;
        filter.requiredType = afl::base::Nothing;
        filter.requiredUser = afl::base::Nothing;
        AFL_CHECK_SUCCEEDS(a("21. getGames"), testee.getGames(HostGame::Filter(), result));
        a.checkEqual("22. size", result.size(), 3U);
        a.checkEqual("23. result", result[0], 2);
        a.checkEqual("24. result", result[1], 3);
        a.checkEqual("25. result", result[2], 5);
    }
    {
        afl::data::IntegerList_t result;
        mock.expectCall("GAMELIST, STATE, running, ID");
        mock.provideNewResult(new VectorValue(Vector::create()));
        HostGame::Filter filter;
        filter.requiredState = HostGame::Running;
        AFL_CHECK_SUCCEEDS(a("26. getGames"), testee.getGames(filter, result));
        a.checkEqual("27. size", result.size(), 0U);
    }
    {
        afl::data::IntegerList_t result;
        mock.expectCall("GAMELIST, TYPE, public, ID");
        mock.provideNewResult(new VectorValue(Vector::create()));
        HostGame::Filter filter;
        filter.requiredType = HostGame::PublicGame;
        AFL_CHECK_SUCCEEDS(a("28. getGames"), testee.getGames(filter, result));
        a.checkEqual("29. size", result.size(), 0U);
    }
    {
        afl::data::IntegerList_t result;
        mock.expectCall("GAMELIST, USER, 1030, ID");
        mock.provideNewResult(new VectorValue(Vector::create()));
        HostGame::Filter filter;
        filter.requiredUser = String_t("1030");
        AFL_CHECK_SUCCEEDS(a("30. getGames"), testee.getGames(filter, result));
        a.checkEqual("31. size", result.size(), 0U);
    }
    {
        afl::data::IntegerList_t result;
        mock.expectCall("GAMELIST, STATE, joining, TYPE, unlisted, USER, 1015, ID");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackInteger(42))));
        HostGame::Filter filter;
        filter.requiredState = HostGame::Joining;
        filter.requiredType = HostGame::UnlistedGame;
        filter.requiredUser = String_t("1015");
        AFL_CHECK_SUCCEEDS(a("32. getGames"), testee.getGames(filter, result));
        a.checkEqual("33. size", result.size(), 1U);
        a.checkEqual("34. result", result[0], 42);
    }

    // GAMESET
    {
        afl::data::StringList_t kv;
        kv.push_back("master");
        kv.push_back("zeus");
        kv.push_back("host");
        kv.push_back("phost2");
        mock.expectCall("GAMESET, 8, master, zeus, host, phost2");
        mock.provideNewResult(0);
        AFL_CHECK_SUCCEEDS(a("41. setConfig"), testee.setConfig(8, kv));
    }

    // GAMEGET
    mock.expectCall("GAMEGET, 7, master");
    mock.provideNewResult(server::makeStringValue("pmaster"));
    a.checkEqual("51. getConfig", testee.getConfig(7, "master"), "pmaster");

    // GAMEMGET [FIXME: needed?]
    {
        afl::data::StringList_t keys;
        keys.push_back("k1");
        keys.push_back("k2");

        afl::data::StringList_t values;

        mock.expectCall("GAMEMGET, 6, k1, k2");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackString("first").pushBackString("second"))));
        AFL_CHECK_SUCCEEDS(a("61. getConfig"), testee.getConfig(6, keys, values));
        a.checkEqual("62. size", values.size(), 2U);
        a.checkEqual("63. result", values[0], "first");
        a.checkEqual("64. result", values[1], "second");
    }

    // GAMEGETCC
    mock.expectCall("GAMEGETCC, 19, difficulty");
    mock.provideNewResult(server::makeIntegerValue(99));
    a.checkEqual("71. getComputedValue", testee.getComputedValue(19, "difficulty"), "99");

    // GAMEGETSTATE
    mock.expectCall("GAMEGETSTATE, 1");
    mock.provideNewResult(server::makeStringValue("joining"));
    a.checkEqual("81. getState", testee.getState(1), HostGame::Joining);

    // GAMEGETTYPE
    mock.expectCall("GAMEGETTYPE, 18");
    mock.provideNewResult(server::makeStringValue("private"));
    a.checkEqual("91. getType", testee.getType(18), HostGame::PrivateGame);

    // GAMEGETOWNER
    mock.expectCall("GAMEGETOWNER, 65");
    mock.provideNewResult(server::makeStringValue("1106"));
    a.checkEqual("101. getOwner", testee.getOwner(65), "1106");

    // GAMEGETNAME
    mock.expectCall("GAMEGETNAME, 8");
    mock.provideNewResult(server::makeStringValue("Eight"));
    a.checkEqual("111. getName", testee.getName(8), "Eight");

    // GAMEGETDIR
    mock.expectCall("GAMEGETDIR, 7");
    mock.provideNewResult(server::makeStringValue("g/777"));
    a.checkEqual("121. getDirectory", testee.getDirectory(7), "g/777");

    // GAMECHECKPERM
    mock.expectCall("GAMECHECKPERM, 9, anon");
    mock.provideNewResult(server::makeIntegerValue(5));
    a.checkEqual("131. getPermissions", testee.getPermissions(9, "anon"), (HostGame::Permissions_t() + HostGame::UserIsOwner + HostGame::UserIsActive));

    // GAMEADDTOOL
    mock.expectCall("GAMEADDTOOL, 3, explmap");
    mock.provideNewResult(server::makeIntegerValue(0));
    a.checkEqual("141. addTool", testee.addTool(3, "explmap"), false);

    // GAMERMTOOL
    mock.expectCall("GAMERMTOOL, 3, wrap");
    mock.provideNewResult(server::makeIntegerValue(1));
    a.checkEqual("151. removeTool", testee.removeTool(3, "wrap"), true);

    // GAMETOTALS
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("joining", server::makeIntegerValue(12));
        h->setNew("running", server::makeIntegerValue(105));
        h->setNew("finished", server::makeIntegerValue(230));
        mock.expectCall("GAMETOTALS");
        mock.provideNewResult(new HashValue(h));

        HostGame::Totals t = testee.getTotals();
        a.checkEqual("161. numJoiningGames", t.numJoiningGames, 12);
        a.checkEqual("162. numRunningGames", t.numRunningGames, 105);
        a.checkEqual("163. numFinishedGames", t.numFinishedGames, 230);
    }

    // GAMEUPDATE [FIXME: needed?]
    {
        afl::data::IntegerList_t ids;
        ids.push_back(32);
        ids.push_back(16);
        ids.push_back(8);
        mock.expectCall("GAMEUPDATE, 32, 16, 8");
        mock.provideNewResult(0);
        AFL_CHECK_SUCCEEDS(a("171. updateGames"), testee.updateGames(ids));
    }

    // GAMERESET
    mock.expectCall("GAMERESET, 55, 13");
    mock.provideNewResult(0);   // does not matter
    AFL_CHECK_SUCCEEDS(a("181. resetToTurn"), testee.resetToTurn(55, 13));

    mock.checkFinish();
}

/** Test GAMESTAT/GAMELIST. */
AFL_TEST("server.interface.HostGameClient:getInfo", a)
{
    using server::interface::HostGame;
    using server::interface::HostSchedule;
    afl::test::CommandHandler mock(a);
    server::interface::HostGameClient testee(mock);

    // Minimum answer from GAMESTAT
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("id", server::makeIntegerValue(7));
        h->setNew("state", server::makeStringValue("running"));
        h->setNew("type",  server::makeStringValue("private"));
        mock.expectCall("GAMESTAT, 7");
        mock.provideNewResult(new HashValue(h));
        HostGame::Info i = testee.getInfo(7);

        a.checkEqual("01. gameId",           i.gameId, 7);
        a.checkEqual("02. state",            i.state, HostGame::Running);
        a.checkEqual("03. type",             i.type, HostGame::PrivateGame);
        a.checkEqual("04. name",             i.name, "");
        a.check     ("05. description",     !i.description.isValid());
        a.check     ("06. currentSchedule", !i.currentSchedule.isValid());
        a.check     ("07. scoreName",       !i.scoreName.isValid());
        a.check     ("08. masterName",      !i.masterName.isValid());
    }

    // Full answer from GAMESTAT
    {
        mock.expectCall("GAMESTAT, 42");
        mock.provideNewResult(makeGameInfo());
        HostGame::Info i = testee.getInfo(42);

        a.checkEqual("11. gameId",      i.gameId, 43); // value as provided by mock, deliberately different from parameter
        a.checkEqual("12. state",       i.state, HostGame::Joining);
        a.checkEqual("13. type",        i.type, HostGame::PublicGame);
        a.checkEqual("14. name",        i.name, "The Name");
        a.check     ("15. description", i.description.isSame(String_t("A test game")));
        a.checkEqual("16. difficulty",  i.difficulty, 133);

        a.check("21. currentSchedule", i.currentSchedule.isValid());
        a.check("22. currentSchedule", i.currentSchedule.get()->type.isSame(HostSchedule::Weekly));

        a.check     ("31. slotStates", i.slotStates.isValid());
        a.checkEqual("32. slotStates", i.slotStates.get()->size(), 3U);
        a.checkEqual("33. slotStates", (*i.slotStates.get())[0], HostGame::OpenSlot);
        a.checkEqual("34. slotStates", (*i.slotStates.get())[1], HostGame::OccupiedSlot);
        a.checkEqual("35. slotStates", (*i.slotStates.get())[2], HostGame::SelfSlot);

        a.check     ("41. turnStates", i.turnStates.isValid());
        a.checkEqual("42. turnStates", i.turnStates.get()->size(), 3U);
        a.checkEqual("43. turnStates", (*i.turnStates.get())[0], 0);
        a.checkEqual("44. turnStates", (*i.turnStates.get())[1], 2);
        a.checkEqual("45. turnStates", (*i.turnStates.get())[2], 1);

        a.check("51. joinable", i.joinable.isSame(false));
        a.check("52. userPlays", i.userPlays.isSame(true));

        a.check     ("61. scores", i.scores.isValid());
        a.checkEqual("62. scores", i.scores.get()->size(), 3U);
        a.checkEqual("63. scores", (*i.scores.get())[0], 130);
        a.checkEqual("64. scores", (*i.scores.get())[1], 140);
        a.checkEqual("65. scores", (*i.scores.get())[2], 135);

        a.check     ("71. scoreName",           i.scoreName.isSame(String_t("test")));
        a.check     ("72. scoreDescription",    i.scoreDescription.isSame(String_t("Test Score")));
        a.checkEqual("73. minRankLevelToJoin",  i.minRankLevelToJoin.orElse(-1), 3);
        a.checkEqual("74. maxRankLevelToJoin",  i.maxRankLevelToJoin.orElse(-1), 4);
        a.checkEqual("75. minRankPointsToJoin", i.minRankPointsToJoin.orElse(-1), 5);
        a.checkEqual("76. maxRankPointsToJoin", i.maxRankPointsToJoin.orElse(-1), 6);
        a.checkEqual("77. hostName",            i.hostName, "thost");
        a.checkEqual("78. hostDescription",     i.hostDescription, "Tim Host");
        a.checkEqual("79. hostKind",            i.hostKind, "th");
        a.checkEqual("80. shipListName",        i.shipListName, "plist2");
        a.checkEqual("81. shipListDescription", i.shipListDescription, "PList 2");
        a.checkEqual("82. shipListKind",        i.shipListKind, "plist");
        a.check     ("83. masterName",          i.masterName.isSame(String_t("xmaster")));
        a.check     ("84. masterDescription",   i.masterDescription.isSame(String_t("Master X")));
        a.check     ("85. masterKind",          i.masterKind.isSame(String_t("mak")));

        a.checkEqual("91. turnNumber",   i.turnNumber, 2);
        a.check     ("92. lastHostTime", i.lastHostTime.isSame(15354520));
        a.check     ("93. nextHostTime", i.nextHostTime.isSame(15356789));
        a.check     ("94. forumId",      i.forumId.isSame(65));
        a.check     ("95. userRank",     i.userRank.isSame(3));
        a.check     ("96. otherRank",    i.otherRank.isSame(7));
    }

    // Full answer from GAMELIST
    {
        mock.expectCall("GAMELIST");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackNew(makeGameInfo()))));
        std::vector<HostGame::Info> infos;
        testee.getInfos(HostGame::Filter(), false, infos);
        a.checkEqual("101. size",   infos.size(), 1U);
        a.checkEqual("102. gameId", infos[0].gameId, 43);
        a.checkEqual("103. state",  infos[0].state, HostGame::Joining);
    }

    // Variations of GAMELIST
    {
        std::vector<HostGame::Info> infos;

        mock.expectCall("GAMELIST, VERBOSE");
        mock.provideNewResult(0);
        testee.getInfos(HostGame::Filter(), true, infos);

        mock.expectCall("GAMELIST, STATE, running");
        mock.provideNewResult(0);
        HostGame::Filter f1;
        f1.requiredState = HostGame::Running;
        testee.getInfos(f1, false, infos);

        mock.expectCall("GAMELIST, TYPE, unlisted");
        mock.provideNewResult(0);
        HostGame::Filter f2;
        f2.requiredType = HostGame::UnlistedGame;
        testee.getInfos(f2, false, infos);

        mock.expectCall("GAMELIST, USER, u32");
        mock.provideNewResult(0);
        HostGame::Filter f3;
        f3.requiredUser = String_t("u32");
        testee.getInfos(f3, false, infos);

        mock.expectCall("GAMELIST, STATE, joining, TYPE, public, USER, 1003, VERBOSE");
        mock.provideNewResult(0);
        HostGame::Filter f4;
        f4.requiredState = HostGame::Joining;
        f4.requiredType = HostGame::PublicGame;
        f4.requiredUser = String_t("1003");
        testee.getInfos(f4, true, infos);

        mock.expectCall("GAMELIST, USER, 1003, HOST, qhost, TOOL, multitool, SHIPLIST, list, MASTER, pmaster, VERBOSE");
        mock.provideNewResult(0);
        HostGame::Filter f5;
        f5.requiredUser = String_t("1003");
        f5.requiredHost = String_t("qhost");
        f5.requiredTool = String_t("multitool");
        f5.requiredShipList = String_t("list");
        f5.requiredMaster = String_t("pmaster");
        testee.getInfos(f5, true, infos);

        mock.expectCall("GAMELIST, COPYOF, 7");
        mock.provideNewResult(0);
        HostGame::Filter f6;
        f6.requiredCopyOf = 7;
        testee.getInfos(f6, false, infos);

        a.checkEqual("111. size", infos.size(), 0U);
    }

    mock.checkFinish();
}

/** Test GAMELSTOOLS. */
AFL_TEST("server.interface.HostGameClient:getTools", a)
{
    using server::interface::HostGame;
    using server::interface::HostTool;
    afl::test::CommandHandler mock(a);
    server::interface::HostGameClient testee(mock);

    // Empty answer
    {
        mock.expectCall("GAMELSTOOLS, 12");
        mock.provideNewResult(new VectorValue(Vector::create()));
        std::vector<HostTool::Info> infos;
        AFL_CHECK_SUCCEEDS(a("01. getTools"), testee.getTools(12, infos));
        a.checkEqual("02. size", infos.size(), 0U);
    }

    // Nonempty answer
    {
        Hash::Ref_t ha = Hash::create();
        ha->setNew("id",           server::makeStringValue("a"));
        ha->setNew("description",  server::makeStringValue("apple"));
        ha->setNew("kind",         server::makeStringValue("fruit"));
        ha->setNew("default",      server::makeIntegerValue(0));

        Hash::Ref_t hb = Hash::create();
        hb->setNew("id",           server::makeStringValue("b"));
        hb->setNew("description",  server::makeStringValue("bread"));
        hb->setNew("kind",         server::makeStringValue("staple"));
        hb->setNew("default",      server::makeIntegerValue(1));

        Vector::Ref_t v = Vector::create();
        v->pushBackNew(new HashValue(ha));
        v->pushBackNew(0);
        v->pushBackNew(new HashValue(hb));

        mock.expectCall("GAMELSTOOLS, 39");
        mock.provideNewResult(new VectorValue(v));
        std::vector<HostTool::Info> infos;
        AFL_CHECK_SUCCEEDS(a("11. getTools"), testee.getTools(39, infos));
        a.checkEqual("12. size", infos.size(), 3U);

        // First
        a.checkEqual("21. id",          infos[0].id, "a");
        a.checkEqual("22. description", infos[0].description, "apple");
        a.checkEqual("23. kind",        infos[0].kind, "fruit");
        a.checkEqual("24. isDefault",   infos[0].isDefault, false);

        // Second, default deserialisation for missing members
        a.checkEqual("31. id",          infos[1].id, "");
        a.checkEqual("32. description", infos[1].description, "");
        a.checkEqual("33. kind",        infos[1].kind, "");
        a.checkEqual("34. isDefault",   infos[1].isDefault, false);

        // Last
        a.checkEqual("41. id",          infos[2].id, "b");
        a.checkEqual("42. description", infos[2].description, "bread");
        a.checkEqual("43. kind",        infos[2].kind, "staple");
        a.checkEqual("44. isDefault",   infos[2].isDefault, true);
    }

    mock.checkFinish();
}

/** Test GAMEGETVC. */
AFL_TEST("server.interface.HostGameClient:getVictoryCondition", a)
{
    using server::interface::HostGame;
    afl::test::CommandHandler mock(a);
    server::interface::HostGameClient testee(mock);

    // Null answer
    {
        mock.expectCall("GAMEGETVC, 89");
        mock.provideNewResult(0);
        HostGame::VictoryCondition vc = testee.getVictoryCondition(89);

        a.checkEqual("01. endCondition",    vc.endCondition, String_t());
        a.check("02. endTurn",             !vc.endTurn.isValid());
        a.check("03. endProbability",      !vc.endProbability.isValid());
        a.check("04. endScore",            !vc.endScore.isValid());
        a.check("05. endScoreName",        !vc.endScoreName.isValid());
        a.check("06. endScoreDescription", !vc.endScoreDescription.isValid());
        a.check("07. referee",             !vc.referee.isValid());
        a.check("08. refereeDescription",  !vc.refereeDescription.isValid());
    }

    // Full answer
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("endCondition",        server::makeStringValue("turn"));
        h->setNew("endTurn",             server::makeIntegerValue(90));
        h->setNew("endProbability",      server::makeIntegerValue(5));
        h->setNew("endScore",            server::makeIntegerValue(15000));
        h->setNew("endScoreName",        server::makeStringValue("Ultra Score"));
        h->setNew("endScoreDescription", server::makeStringValue("Best Ever!"));
        h->setNew("referee",             server::makeStringValue("Bibi"));
        h->setNew("refereeDescription",  server::makeStringValue("Info..."));
        mock.expectCall("GAMEGETVC, 76");
        mock.provideNewResult(new HashValue(h));

        HostGame::VictoryCondition vc = testee.getVictoryCondition(76);

        a.checkEqual("11. endCondition",   vc.endCondition, "turn");
        a.check("12. endTurn",             vc.endTurn.isSame(90));
        a.check("13. endProbability",      vc.endProbability.isSame(5));
        a.check("14. endScore",            vc.endScore.isSame(15000));
        a.check("15. endScoreName",        vc.endScoreName.isSame(String_t("Ultra Score")));
        a.check("16. endScoreDescription", vc.endScoreDescription.isSame(String_t("Best Ever!")));
        a.check("17. referee",             vc.referee.isSame(String_t("Bibi")));
        a.check("18. refereeDescription",  vc.refereeDescription.isSame(String_t("Info...")));
    }

    mock.checkFinish();
}

/** Test errors. */
AFL_TEST("server.interface.HostGameClient:errors", a)
{
    using server::interface::HostGame;
    afl::test::CommandHandler mock(a);
    server::interface::HostGameClient testee(mock);

    // GAMEGETSTATE
    mock.expectCall("GAMEGETSTATE, 7");
    mock.provideNewResult(server::makeStringValue("thinking"));
    AFL_CHECK_THROWS(a("01. bad state"), testee.getState(7), afl::except::InvalidDataException);

    // GAMEGETTYPE
    mock.expectCall("GAMEGETTYPE, 12");
    mock.provideNewResult(server::makeStringValue("fun"));
    AFL_CHECK_THROWS(a("11. bad type"), testee.getType(12), afl::except::InvalidDataException);

    // GAMESTAT with empty result (means: state/type don't decode)
    mock.expectCall("GAMESTAT, 9");
    mock.provideNewResult(0);
    AFL_CHECK_THROWS(a("21. null stat"), testee.getInfo(9), afl::except::InvalidDataException);

    // GAMESTAT with invalid state
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("id",                  server::makeIntegerValue(43));
        h->setNew("state",               server::makeStringValue("fighting"));
        h->setNew("type",                server::makeStringValue("public"));
        mock.expectCall("GAMESTAT, 2");
        mock.provideNewResult(new HashValue(h));
        AFL_CHECK_THROWS(a("31. bad state"), testee.getInfo(2), afl::except::InvalidDataException);
    }

    // GAMESTAT with invalid type
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("id",                  server::makeIntegerValue(43));
        h->setNew("state",               server::makeStringValue("finished"));
        h->setNew("type",                server::makeStringValue("boring"));
        mock.expectCall("GAMESTAT, 3");
        mock.provideNewResult(new HashValue(h));
        AFL_CHECK_THROWS(a("41. bad type"), testee.getInfo(3), afl::except::InvalidDataException);
    }

    // GAMESTAT with invalid slot state
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("id",                  server::makeIntegerValue(43));
        h->setNew("state",               server::makeStringValue("finished"));
        h->setNew("type",                server::makeStringValue("public"));
        h->setNew("slots", new VectorValue(Vector::create(Segment().pushBackString("meh"))));
        mock.expectCall("GAMESTAT, 4");
        mock.provideNewResult(new HashValue(h));
        AFL_CHECK_THROWS(a("51. bad slot state"), testee.getInfo(4), afl::except::InvalidDataException);
    }
}
