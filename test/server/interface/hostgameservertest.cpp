/**
  *  \file test/server/interface/hostgameservertest.cpp
  *  \brief Test for server::interface::HostGameServer
  */

#include "server/interface/hostgameserver.hpp"

#include "afl/data/access.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/hostgameclient.hpp"
#include <stdexcept>

using afl::data::Access;
using afl::data::Segment;
using afl::string::Format;
using server::interface::HostGame;
using server::interface::HostSchedule;
using server::interface::HostTool;
using server::Value_t;

namespace {
    class HostGameMock : public afl::test::CallReceiver, public HostGame {
     public:
        HostGameMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual int32_t createNewGame()
            {
                checkCall("createNewGame()");
                return consumeReturnValue<int>();
            }
        virtual int32_t cloneGame(int32_t gameId, afl::base::Optional<State> newState)
            {
                checkCall(Format("cloneGame(%d,%s)", gameId, newState.isValid() ? HostGame::formatState(*newState.get()) : String_t("-")));
                return consumeReturnValue<int>();
            }
        virtual void setType(int32_t gameId, Type type)
            { checkCall(Format("setType(%d,%s)", gameId, HostGame::formatType(type))); }
        virtual void setState(int32_t gameId, State state)
            { checkCall(Format("setState(%d,%s)", gameId, HostGame::formatState(state))); }
        virtual void setOwner(int32_t gameId, String_t user)
            { checkCall(Format("setOwner(%d,%s)", gameId, user)); }
        virtual void setName(int32_t gameId, String_t name)
            { checkCall(Format("setName(%d,%s)", gameId, name)); }
        virtual Info getInfo(int32_t gameId)
            {
                checkCall(Format("getInfo(%d)", gameId));
                return consumeReturnValue<Info>();
            }
        virtual void getInfos(const Filter& filter, bool verbose, std::vector<Info>& result)
            {
                checkCall(Format("getInfos(%s,%s,%s,%s,%s,%s,%s,%d,%s)")
                          << (filter.requiredState.isValid() ? HostGame::formatState(*filter.requiredState.get()) : String_t("-"))
                          << (filter.requiredType.isValid()  ? HostGame::formatType(*filter.requiredType.get())   : String_t("-"))
                          << filter.requiredUser.orElse("-")
                          << filter.requiredHost.orElse("-")
                          << filter.requiredTool.orElse("-")
                          << filter.requiredShipList.orElse("-")
                          << filter.requiredMaster.orElse("-")
                          << filter.requiredCopyOf.orElse(-1)
                          << (verbose ? "t" : "f"));
                int n = consumeReturnValue<int>();
                while (n-- > 0) {
                    result.push_back(consumeReturnValue<Info>());
                }
            }
        virtual void getGames(const Filter& filter, afl::data::IntegerList_t& result)
            {
                checkCall(Format("getGames(%s,%s,%s,%s,%s,%s,%s,%d)")
                          << (filter.requiredState.isValid() ? HostGame::formatState(*filter.requiredState.get()) : String_t("-"))
                          << (filter.requiredType.isValid()  ? HostGame::formatType(*filter.requiredType.get())   : String_t("-"))
                          << filter.requiredUser.orElse("-")
                          << filter.requiredHost.orElse("-")
                          << filter.requiredTool.orElse("-")
                          << filter.requiredShipList.orElse("-")
                          << filter.requiredMaster.orElse("-")
                          << filter.requiredCopyOf.orElse(-1));
                int n = consumeReturnValue<int>();
                while (n-- > 0) {
                    result.push_back(consumeReturnValue<int>());
                }
            }
        virtual void setConfig(int32_t gameId, const afl::data::StringList_t& keyValues)
            {
                String_t call = Format("setConfig(%d", gameId);
                for (size_t i = 0, n = keyValues.size(); i < n; ++i) {
                    call += ",";
                    call += keyValues[i];
                }
                call += ")";
                checkCall(call);
            }
        virtual String_t getConfig(int32_t gameId, String_t key)
            {
                checkCall(Format("getConfig(%d,%s)", gameId, key));
                return consumeReturnValue<String_t>();
            }
        virtual void getConfig(int32_t gameId, const afl::data::StringList_t& keys, afl::data::StringList_t& values)
            {
                String_t call = Format("getConfig(%d", gameId);
                for (size_t i = 0, n = keys.size(); i < n; ++i) {
                    call += ",";
                    call += keys[i];
                }
                call += ")";
                checkCall(call);

                int n = consumeReturnValue<int>();
                while (n-- > 0) {
                    values.push_back(consumeReturnValue<String_t>());
                }
            }
        virtual String_t getComputedValue(int32_t gameId, String_t key)
            {
                checkCall(Format("getComputedValue(%d,%s)", gameId, key));
                return consumeReturnValue<String_t>();
            }
        virtual State getState(int32_t gameId)
            {
                checkCall(Format("getState(%d)", gameId));
                return consumeReturnValue<State>();
            }
        virtual Type getType(int32_t gameId)
            {
                checkCall(Format("getType(%d)", gameId));
                return consumeReturnValue<Type>();
            }
        virtual String_t getOwner(int32_t gameId)
            {
                checkCall(Format("getOwner(%d)", gameId));
                return consumeReturnValue<String_t>();
            }
        virtual String_t getName(int32_t gameId)
            {
                checkCall(Format("getName(%d)", gameId));
                return consumeReturnValue<String_t>();
            }
        virtual String_t getDirectory(int32_t gameId)
            {
                checkCall(Format("getDirectory(%d)", gameId));
                return consumeReturnValue<String_t>();
            }
        virtual Permissions_t getPermissions(int32_t gameId, String_t userId)
            {
                checkCall(Format("getPermissions(%d,%s)", gameId, userId));
                return consumeReturnValue<Permissions_t>();
            }
        virtual bool addTool(int32_t gameId, String_t toolId)
            {
                checkCall(Format("addTool(%d,%s)", gameId, toolId));
                return consumeReturnValue<bool>();
            }
        virtual bool removeTool(int32_t gameId, String_t toolId)
            {
                checkCall(Format("removeTool(%d,%s)", gameId, toolId));
                return consumeReturnValue<bool>();
            }
        virtual void getTools(int32_t gameId, std::vector<server::interface::HostTool::Info>& result)
            {
                checkCall(Format("getTools(%d)", gameId));
                int n = consumeReturnValue<int>();
                while (n-- > 0) {
                    result.push_back(consumeReturnValue<server::interface::HostTool::Info>());
                }
            }
        virtual Totals getTotals()
            {
                checkCall("getTotals()");
                return consumeReturnValue<Totals>();
            }
        virtual VictoryCondition getVictoryCondition(int32_t gameId)
            {
                checkCall(Format("getVictoryCondition(%d)", gameId));
                return consumeReturnValue<VictoryCondition>();
            }
        virtual void updateGames(const afl::data::IntegerList_t& gameIds)
            {
                String_t call = "updateGames(";
                for (size_t i = 0, n = gameIds.size(); i < n; ++i) {
                    if (i != 0) {
                        call += ",";
                    }
                    call += Format("%d", gameIds[i]);
                }
                call += ")";
                checkCall(call);
            }
        virtual void resetToTurn(int32_t gameId, int turnNr)
            { checkCall(Format("resetToTurn(%d,%d)", gameId, turnNr)); }
    };

    HostGame::Info makeInfo()
    {
        // Prepare fully-populated info
        HostSchedule::Schedule sch;
        sch.type     = HostSchedule::Weekly;
        sch.weekdays = 19;
        sch.daytime  = 600;

        std::vector<HostGame::SlotState> ss;
        ss.push_back(HostGame::DeadSlot);
        ss.push_back(HostGame::SelfSlot);
        ss.push_back(HostGame::OccupiedSlot);

        std::vector<int32_t> ts;
        ts.push_back(16);
        ts.push_back(1);

        std::vector<int32_t> sc;
        sc.push_back(12);
        sc.push_back(167);
        sc.push_back(150);

        HostGame::Info i;
        i.gameId              = 42;
        i.state               = HostGame::Running;
        i.type                = HostGame::UnlistedGame;
        i.name                = "Answer";
        i.description         = "A Game";
        i.difficulty          = 96;
        i.currentSchedule     = sch;
        i.slotStates          = ss;
        i.turnStates          = ts;
        i.joinable            = true;
        i.userPlays           = true;
        i.scores              = sc;
        i.scoreName           = "escore";
        i.scoreDescription    = "A Score";
        i.minRankLevelToJoin  = 10;
        i.maxRankLevelToJoin  = 11;
        i.minRankPointsToJoin = 22;
        i.maxRankPointsToJoin = 23;
        i.hostName            = "qhost";
        i.hostDescription     = "Quality Host";
        i.hostKind            = "qq";
        i.shipListName        = "default";
        i.shipListDescription = "Default List";
        i.shipListKind        = "slk";
        i.masterName          = "qmaster";
        i.masterDescription   = "Quality Master";
        i.masterKind          = "mk";
        i.turnNumber          = 3;
        i.lastHostTime        = 1961;
        i.nextHostTime        = 1989;
        i.forumId             = 23;
        i.userRank            = 7;
        i.otherRank           = 8;

        return i;
    }
}

/** Test general cases. */
AFL_TEST("server.interface.HostGameServer:commands", a)
{
    HostGameMock mock(a);
    server::interface::HostGameServer testee(mock);

    // createNewGame
    mock.expectCall("createNewGame()");
    mock.provideReturnValue(72);
    a.checkEqual("01. newgame", testee.callInt(Segment().pushBackString("NEWGAME")), 72);

    // cloneGame
    mock.expectCall("cloneGame(3,-)");
    mock.provideReturnValue(73);
    a.checkEqual("11. clonegame", testee.callInt(Segment().pushBackString("CLONEGAME").pushBackInteger(3)), 73);

    mock.expectCall("cloneGame(4,joining)");
    mock.provideReturnValue(74);
    a.checkEqual("21. clonegame", testee.callInt(Segment().pushBackString("CLONEGAME").pushBackInteger(4).pushBackString("joining")), 74);

    // setType/State/Owner/Name
    mock.expectCall("setType(17,public)");
    AFL_CHECK_SUCCEEDS(a("31. gamesettype"), testee.callVoid(Segment().pushBackString("GAMESETTYPE").pushBackInteger(17).pushBackString("public")));

    mock.expectCall("setState(17,finished)");
    AFL_CHECK_SUCCEEDS(a("41. gamesetstate"), testee.callVoid(Segment().pushBackString("GAMESETSTATE").pushBackInteger(17).pushBackString("finished")));

    mock.expectCall("setOwner(17,1032)");
    AFL_CHECK_SUCCEEDS(a("51. gamesetowner"), testee.callVoid(Segment().pushBackString("GAMESETOWNER").pushBackInteger(17).pushBackString("1032")));

    mock.expectCall("setName(98,Eightynine)");
    AFL_CHECK_SUCCEEDS(a("61. gamesetname"), testee.callVoid(Segment().pushBackString("GAMESETNAME").pushBackInteger(98).pushBackString("Eightynine")));

    // getInfo
    // - full data
    {
        mock.expectCall("getInfo(17)");
        mock.provideReturnValue(makeInfo());

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMESTAT").pushBackInteger(17)));
        Access ap(p);
        a.checkEqual("71. id",                   ap("id").toInteger(), 42);
        a.checkEqual("72. state",                ap("state").toString(), "running");
        a.checkEqual("73. type",                 ap("type").toString(), "unlisted");
        a.checkEqual("74. name",                 ap("name").toString(), "Answer");
        a.checkEqual("75. description",          ap("description").toString(), "A Game");
        a.checkEqual("76. difficulty",           ap("difficulty").toInteger(), 96);
        a.checkEqual("77. currentSchedule",      ap("currentSchedule")("type").toInteger(), 1);
        a.checkEqual("78. currentSchedule",      ap("currentSchedule")("weekdays").toInteger(), 19);
        a.checkEqual("79. currentSchedule",      ap("currentSchedule")("daytime").toInteger(), 600);
        a.checkNull ("80. currentSchedule",      ap("currentSchedule")("condition").getValue());
        a.checkEqual("81. slots",                ap("slots").getArraySize(), 3U);
        a.checkEqual("82. slots",                ap("slots")[0].toString(), "dead");
        a.checkEqual("83. slots",                ap("slots")[1].toString(), "self");
        a.checkEqual("84. slots",                ap("slots")[2].toString(), "occupied");
        a.checkEqual("85. turns",                ap("turns").getArraySize(), 2U);
        a.checkEqual("86. turns",                ap("turns")[0].toInteger(), 16);
        a.checkEqual("87. turns",                ap("turns")[1].toInteger(), 1);
        a.checkEqual("88. joinable",             ap("joinable").toInteger(), 1);
        a.checkEqual("89. userPlays",            ap("userPlays").toInteger(), 1);
        a.checkEqual("90. scores",               ap("scores").getArraySize(), 3U);
        a.checkEqual("91. scores",               ap("scores")[0].toInteger(), 12);
        a.checkEqual("92. scores",               ap("scores")[1].toInteger(), 167);
        a.checkEqual("93. scores",               ap("scores")[2].toInteger(), 150);
        a.checkEqual("94. scoreName",            ap("scoreName").toString(), "escore");
        a.checkEqual("95. scoreDescription",     ap("scoreDescription").toString(), "A Score");
        a.checkEqual("96. minRankLevelToJoin",   ap("minRankLevelToJoin").toInteger(), 10);
        a.checkEqual("97. maxRankLevelToJoin",   ap("maxRankLevelToJoin").toInteger(), 11);
        a.checkEqual("98. minRankPointsToJoin",  ap("minRankPointsToJoin").toInteger(), 22);
        a.checkEqual("99. maxRankPointsToJoin",  ap("maxRankPointsToJoin").toInteger(), 23);
        a.checkEqual("100. host",                ap("host").toString(), "qhost");
        a.checkEqual("101. hostDescription",     ap("hostDescription").toString(), "Quality Host");
        a.checkEqual("102. hostKind",            ap("hostKind").toString(), "qq");
        a.checkEqual("103. shiplist",            ap("shiplist").toString(), "default");
        a.checkEqual("104. shiplistDescription", ap("shiplistDescription").toString(), "Default List");
        a.checkEqual("105. shiplistKind",        ap("shiplistKind").toString(), "slk");
        a.checkEqual("106. master",              ap("master").toString(), "qmaster");
        a.checkEqual("107. masterDescription",   ap("masterDescription").toString(), "Quality Master");
        a.checkEqual("108. masterKind",          ap("masterKind").toString(), "mk");
        a.checkEqual("109. turn",                ap("turn").toInteger(), 3);
        a.checkEqual("110. lastHostTime",        ap("lastHostTime").toInteger(), 1961);
        a.checkEqual("111. nextHostTime",        ap("nextHostTime").toInteger(), 1989);
        a.checkEqual("112. forum",               ap("forum").toInteger(), 23);
        a.checkEqual("113. userRank",            ap("userRank").toInteger(), 7);
        a.checkEqual("114. otherRank",           ap("otherRank").toInteger(), 8);
    }

    // - default (=minimal) data
    {
        mock.expectCall("getInfo(17)");
        mock.provideReturnValue(HostGame::Info());

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMESTAT").pushBackInteger(17)));
        Access ap(p);
        a.checkEqual("121. id",              ap("id").toInteger(), 0);
        a.checkEqual("122. state",           ap("state").toString(), "preparing");
        a.checkEqual("123. type",            ap("type").toString(), "private");
        a.checkEqual("124. name",            ap("name").toString(), "");
        a.checkNull ("125. description",     ap("description").getValue());
        a.checkNull ("126. currentSchedule", ap("currentSchedule").getValue());
        a.checkNull ("127. turns",           ap("turns").getValue());
        a.checkNull ("128. forum",           ap("forum").getValue());
        a.checkNull ("129. userRank",        ap("userRank").getValue());
        a.checkNull ("130. otherRank",       ap("otherRank").getValue());
    }

    // getInfos
    {
        mock.expectCall("getInfos(-,-,-,-,-,-,-,-1,f)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(makeInfo());
        mock.provideReturnValue(HostGame::Info());

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMELIST")));
        Access ap(p);
        a.checkEqual("131. getArraySize", ap.getArraySize(), 2U);
        a.checkEqual("132. id",              ap[0]("id").toInteger(), 42);
        a.checkEqual("133. state",           ap[0]("state").toString(), "running");
        a.checkEqual("134. currentSchedule", ap[0]("currentSchedule")("weekdays").toInteger(), 19);
        a.checkEqual("135. id",              ap[1]("id").toInteger(), 0);
        a.checkEqual("136. state",           ap[1]("state").toString(), "preparing");
        a.checkNull ("137. currentSchedule", ap[1]("currentSchedule").getValue());
        a.checkEqual("138. currentSchedule", ap[1]("currentSchedule")("weekdays").toInteger(), 0);
    }
    {
        mock.expectCall("getInfos(running,-,-,-,-,-,-,-1,t)");
        mock.provideReturnValue(0);
        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMELIST").pushBackString("STATE").pushBackString("running").pushBackString("VERBOSE")));
        Access ap(p);
        a.checkEqual("139. getArraySize", ap.getArraySize(), 0U);
    }
    {
        mock.expectCall("getInfos(-,public,-,-,-,-,-,-1,f)");
        mock.provideReturnValue(0);
        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMELIST").pushBackString("TYPE").pushBackString("public")));
        Access ap(p);
        a.checkEqual("140. getArraySize", ap.getArraySize(), 0U);
    }
    {
        mock.expectCall("getInfos(-,-,fred,-,-,-,-,-1,f)");
        mock.provideReturnValue(0);
        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMELIST").pushBackString("USER").pushBackString("fred")));
        Access ap(p);
        a.checkEqual("141. getArraySize", ap.getArraySize(), 0U);
    }
    {
        mock.expectCall("getInfos(joining,unlisted,wilma,-,-,-,-,-1,t)");
        mock.provideReturnValue(0);
        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMELIST").pushBackString("USER").pushBackString("wilma").pushBackString("VERBOSE").
                                             pushBackString("TYPE").pushBackString("unlisted").pushBackString("STATE").pushBackString("joining")));
        Access ap(p);
        a.checkEqual("142. getArraySize", ap.getArraySize(), 0U);
    }

    // getGames
    {
        mock.expectCall("getGames(-,-,-,-,-,-,-,-1)");
        mock.provideReturnValue(4);
        mock.provideReturnValue(89);
        mock.provideReturnValue(32);
        mock.provideReturnValue(16);
        mock.provideReturnValue(8);

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMELIST").pushBackString("ID")));
        Access ap(p);
        a.checkEqual("151. getArraySize", ap.getArraySize(), 4U);
        a.checkEqual("152. result", ap[0].toInteger(), 89);
        a.checkEqual("153. result", ap[1].toInteger(), 32);
        a.checkEqual("154. result", ap[2].toInteger(), 16);
        a.checkEqual("155. result", ap[3].toInteger(), 8);
    }
    {
        mock.expectCall("getGames(finished,private,1030,-,-,-,-,-1)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(3);
        mock.provideReturnValue(5);

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMELIST").pushBackString("TYPE").pushBackString("private").
                                             pushBackString("STATE").pushBackString("finished").pushBackString("ID").pushBackString("USER").pushBackString("1030")));
        Access ap(p);
        a.checkEqual("161. getArraySize", ap.getArraySize(), 2U);
        a.checkEqual("162. result", ap[0].toInteger(), 3);
        a.checkEqual("163. result", ap[1].toInteger(), 5);
    }

    // setConfig
    mock.expectCall("setConfig(8,one,a,other,b)");
    AFL_CHECK_SUCCEEDS(a("171. gameget"), testee.callVoid(Segment().pushBackString("GAMESET").pushBackInteger(8).pushBackString("one").pushBackString("a").pushBackString("other").pushBackString("b")));

    mock.expectCall("setConfig(5)");
    AFL_CHECK_SUCCEEDS(a("181. gameset"), testee.callVoid(Segment().pushBackString("GAMESET").pushBackInteger(5)));

    // getConfig [single]
    mock.expectCall("getConfig(14,kk)");
    mock.provideReturnValue(String_t("zz"));
    a.checkEqual("191. gameget", testee.callString(Segment().pushBackString("GAMEGET").pushBackInteger(14).pushBackString("kk")), "zz");

    // getConfig [multi]
    {
        mock.expectCall("getConfig(19,ha,hu,hi)");
        mock.provideReturnValue(3);
        mock.provideReturnValue(String_t("bla"));
        mock.provideReturnValue(String_t("blu"));
        mock.provideReturnValue(String_t("bli"));

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMEMGET").pushBackInteger(19).pushBackString("ha").pushBackString("hu").pushBackString("hi")));
        Access ap(p);
        a.checkEqual("201. getArraySize", ap.getArraySize(), 3U);
        a.checkEqual("202. result", ap[0].toString(), "bla");
        a.checkEqual("203. result", ap[1].toString(), "blu");
        a.checkEqual("204. result", ap[2].toString(), "bli");
    }

    // getComputedValue
    mock.expectCall("getComputedValue(8,ck)");
    mock.provideReturnValue(String_t("cv"));
    a.checkEqual("211. gamegetcc", testee.callString(Segment().pushBackString("GAMEGETCC").pushBackInteger(8).pushBackString("ck")), "cv");

    // getState
    mock.expectCall("getState(12)");
    mock.provideReturnValue(HostGame::Finished);
    a.checkEqual("221. gamegetstate", testee.callString(Segment().pushBackString("GAMEGETSTATE").pushBackInteger(12)), "finished");

    // getType
    mock.expectCall("getType(23)");
    mock.provideReturnValue(HostGame::PublicGame);
    a.checkEqual("231. gamegettype", testee.callString(Segment().pushBackString("GAMEGETTYPE").pushBackInteger(23)), "public");

    // getOwner
    mock.expectCall("getOwner(92)");
    mock.provideReturnValue(String_t("u96"));
    a.checkEqual("241. gamegetowner", testee.callString(Segment().pushBackString("GAMEGETOWNER").pushBackInteger(92)), "u96");

    // getName
    mock.expectCall("getName(76)");
    mock.provideReturnValue(String_t("gg"));
    a.checkEqual("251. gamegetname", testee.callString(Segment().pushBackString("GAMEGETNAME").pushBackInteger(76)), "gg");

    // getDirectory
    mock.expectCall("getDirectory(34)");
    mock.provideReturnValue(String_t("a/b/c"));
    a.checkEqual("261. gamegetdir", testee.callString(Segment().pushBackString("GAMEGETDIR").pushBackInteger(34)), "a/b/c");

    // getPermissions
    mock.expectCall("getPermissions(8,zz)");
    mock.provideReturnValue(HostGame::Permissions_t() + HostGame::UserIsOwner + HostGame::GameIsPublic);
    a.checkEqual("271. gamecheckperm", testee.callInt(Segment().pushBackString("GAMECHECKPERM").pushBackInteger(8).pushBackString("zz")), 17);

    // addTool
    mock.expectCall("addTool(53,nt)");
    mock.provideReturnValue(true);
    a.checkEqual("281. gameaddtool", testee.callInt(Segment().pushBackString("GAMEADDTOOL").pushBackInteger(53).pushBackString("nt")), 1);

    // removeTool
    mock.expectCall("removeTool(57,ot)");
    mock.provideReturnValue(false);
    a.checkEqual("291. gamermtool", testee.callInt(Segment().pushBackString("GAMERMTOOL").pushBackInteger(57).pushBackString("ot")), 0);

    // getTools
    {
        mock.expectCall("getTools(56)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(HostTool::Info("ii", "dd", "kk", true));
        mock.provideReturnValue(HostTool::Info("i2", "d2", "k2", false));

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMELSTOOLS").pushBackInteger(56)));
        Access ap(p);
        a.checkEqual("301. getArraySize", ap.getArraySize(), 2U);
        a.checkEqual("302. id",           ap[0]("id").toString(),          "ii");
        a.checkEqual("303. description",  ap[0]("description").toString(), "dd");
        a.checkEqual("304. kind",         ap[0]("kind").toString(),        "kk");
        a.checkEqual("305. default",      ap[0]("default").toInteger(),    1);
        a.checkEqual("306. id",           ap[1]("id").toString(),          "i2");
        a.checkEqual("307. description",  ap[1]("description").toString(), "d2");
        a.checkEqual("308. kind",         ap[1]("kind").toString(),        "k2");
        a.checkEqual("309. default",      ap[1]("default").toInteger(),    0);
    }

    // getTotals
    {
        mock.expectCall("getTotals()");
        mock.provideReturnValue(HostGame::Totals(9, 3, 4));

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMETOTALS")));
        Access ap(p);

        a.checkEqual("311. joining", ap("joining").toInteger(), 9);
        a.checkEqual("312. running", ap("running").toInteger(), 3);
        a.checkEqual("313. finished", ap("finished").toInteger(), 4);
    }

    // getVictoryCondition
    {
        HostGame::VictoryCondition vc;
        vc.endCondition = "ee";
        vc.endTurn = 62;
        vc.endProbability = 5;
        vc.endScore = 99;
        vc.endScoreName = "esn";
        vc.endScoreDescription = "esd";
        vc.referee = "ref";
        vc.refereeDescription = "refd";

        mock.expectCall("getVictoryCondition(18)");
        mock.provideReturnValue(vc);

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMEGETVC").pushBackInteger(18)));
        Access ap(p);

        a.checkEqual("321. endcondition",        ap("endCondition").toString(), "ee");
        a.checkEqual("322. endturn",             ap("endTurn").toInteger(), 62);
        a.checkEqual("323. endprobability",      ap("endProbability").toInteger(), 5);
        a.checkEqual("324. endscore",            ap("endScore").toInteger(), 99);
        a.checkEqual("325. endscorename",        ap("endScoreName").toString(), "esn");
        a.checkEqual("326. endscoredescription", ap("endScoreDescription").toString(), "esd");
        a.checkEqual("327. referee",             ap("referee").toString(), "ref");
        a.checkEqual("328. refereedescription",  ap("refereeDescription").toString(), "refd");
    }
    {
        mock.expectCall("getVictoryCondition(18)");
        mock.provideReturnValue(HostGame::VictoryCondition());

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMEGETVC").pushBackInteger(18)));
        Access ap(p);

        a.checkEqual("331. endcondition",       ap("endCondition").toString(), "");
        a.checkNull("332. endturn",             ap("endTurn").getValue());
        a.checkNull("333. endprobability",      ap("endProbability").getValue());
        a.checkNull("334. endscore",            ap("endScore").getValue());
        a.checkNull("335. endscorename",        ap("endScoreName").getValue());
        a.checkNull("336. endscoredescription", ap("endScoreDescription").getValue());
        a.checkNull("337. referee",             ap("referee").getValue());
        a.checkNull("338. refereedescription",  ap("refereeDescription").getValue());
    }

    // updateGames
    mock.expectCall("updateGames(1,3,5)");
    AFL_CHECK_SUCCEEDS(a("341. gameupdate"), testee.callVoid(Segment().pushBackString("GAMEUPDATE").pushBackInteger(1).pushBackInteger(3).pushBackInteger(5)));

    mock.expectCall("updateGames()");
    AFL_CHECK_SUCCEEDS(a("351. gameupdate"), testee.callVoid(Segment().pushBackString("GAMEUPDATE")));

    // resetToTurn
    mock.expectCall("resetToTurn(7,22)");
    AFL_CHECK_SUCCEEDS(a("361. gamereset"), testee.callVoid(Segment().pushBackString("GAMERESET").pushBackInteger(7).pushBackInteger(22)));

    // Variations
    mock.expectCall("createNewGame()");
    mock.provideReturnValue(99);
    a.checkEqual("371. newgame", testee.callInt(Segment().pushBackString("newGame")), 99);

    mock.expectCall("getInfos(running,-,-,-,-,-,-,-1,t)");
    mock.provideReturnValue(0);
    AFL_CHECK_SUCCEEDS(a("381. gamelist"), testee.callVoid(Segment().pushBackString("GAMELIST").pushBackString("state").pushBackString("running").pushBackString("verbose")));

    mock.expectCall("getInfos(running,-,-,-,-,-,-,-1,t)");
    mock.provideReturnValue(0);
    AFL_CHECK_SUCCEEDS(a("391. gamelist"), testee.callVoid(Segment().pushBackString("GAMELIST").pushBackString("STATE").pushBackString("running").pushBackString("ID").pushBackString("VERBOSE")));

    mock.expectCall("getInfos(-,-,-,-,-,-,-,99,f)");
    mock.provideReturnValue(0);
    AFL_CHECK_SUCCEEDS(a("401. gamelist"), testee.callVoid(Segment().pushBackString("GAMELIST").pushBackString("COPYOF").pushBackInteger(99)));

    mock.checkFinish();
}

/** Test errors. */
AFL_TEST("server.interface.HostGameServer:errors", a)
{
    HostGameMock mock(a);
    server::interface::HostGameServer testee(mock);

    // Number of parameters
    Segment empty;
    AFL_CHECK_THROWS(a("01. no verb"),        testee.callVoid(empty), std::exception);
    AFL_CHECK_THROWS(a("02. too many args"),  testee.callVoid(Segment().pushBackString("NEWGAME").pushBackInteger(3)), std::exception);
    AFL_CHECK_THROWS(a("03. missing arg"),    testee.callVoid(Segment().pushBackString("CLONEGAME")), std::exception);
    AFL_CHECK_THROWS(a("04. missing option"), testee.callVoid(Segment().pushBackString("GAMELIST").pushBackString("STATE")), std::exception);
    AFL_CHECK_THROWS(a("05. missing option"), testee.callVoid(Segment().pushBackString("GAMELIST").pushBackString("TYPE")), std::exception);
    AFL_CHECK_THROWS(a("06. missing option"), testee.callVoid(Segment().pushBackString("GAMELIST").pushBackString("USER")), std::exception);
    AFL_CHECK_THROWS(a("07. missing arg"),    testee.callVoid(Segment().pushBackString("GAMEMGET")), std::exception);
    AFL_CHECK_THROWS(a("08. too many args"),  testee.callVoid(Segment().pushBackString("GAMETOTALS").pushBackInteger(9)), std::exception);
    AFL_CHECK_THROWS(a("09. missing arg"),    testee.callVoid(Segment().pushBackString("GAMERESET").pushBackInteger(7)), std::exception);

    // Bad commands or keywords
    AFL_CHECK_THROWS(a("11. bad verb"),   testee.callVoid(Segment().pushBackString("")), std::exception);
    AFL_CHECK_THROWS(a("12. bad verb"),   testee.callVoid(Segment().pushBackString("HI")), std::exception);
    AFL_CHECK_THROWS(a("13. bad option"), testee.callVoid(Segment().pushBackString("GAMELIST").pushBackString("FUN")), std::exception);

    // Bad status, type, etc
    AFL_CHECK_THROWS(a("21. bad status"), testee.callInt(Segment().pushBackString("CLONEGAME").pushBackInteger(4).pushBackString("JOINING")), std::exception);
    AFL_CHECK_THROWS(a("22. bad status"), testee.callInt(Segment().pushBackString("CLONEGAME").pushBackInteger(4).pushBackString("")), std::exception);
    AFL_CHECK_THROWS(a("23. bad status"), testee.callInt(Segment().pushBackString("CLONEGAME").pushBackInteger(4).pushBackString("x")), std::exception);
    AFL_CHECK_THROWS(a("24. bad type"),   testee.callVoid(Segment().pushBackString("GAMESETTYPE").pushBackInteger(3).pushBackString("ha")), std::exception);
    AFL_CHECK_THROWS(a("25. bad status"), testee.callVoid(Segment().pushBackString("GAMESETSTATE").pushBackInteger(3).pushBackString("hu")), std::exception);
    AFL_CHECK_THROWS(a("26. bad status"), testee.callVoid(Segment().pushBackString("GAMELIST").pushBackString("STATE").pushBackString("jumping")), std::exception);
    AFL_CHECK_THROWS(a("27. bad status"), testee.callVoid(Segment().pushBackString("GAMELIST").pushBackString("STATE").pushBackString("RUNNING")), std::exception);
    AFL_CHECK_THROWS(a("28. bad type"),   testee.callVoid(Segment().pushBackString("GAMELIST").pushBackString("TYPE").pushBackString("typing")), std::exception);
    AFL_CHECK_THROWS(a("29. bad id"),     testee.callVoid(Segment().pushBackString("GAMEMGET").pushBackString("hu")), std::exception);
    AFL_CHECK_THROWS(a("30. bad turn"),   testee.callVoid(Segment().pushBackString("GAMERESET").pushBackInteger(7).pushBackString("asdljlad")), std::exception);

    mock.checkFinish();
}

/** Test roundtrip with client. */
AFL_TEST("server.interface.HostGameServer:roundtrip", a)
{
    HostGameMock mock(a);
    server::interface::HostGameServer level1(mock);
    server::interface::HostGameClient level2(level1);
    server::interface::HostGameServer level3(level2);
    server::interface::HostGameClient level4(level3);

    // createNewGame
    mock.expectCall("createNewGame()");
    mock.provideReturnValue(72);
    a.checkEqual("01. createNewGame", level4.createNewGame(), 72);

    // cloneGame
    mock.expectCall("cloneGame(3,-)");
    mock.provideReturnValue(73);
    a.checkEqual("11. cloneGame", level4.cloneGame(3, afl::base::Nothing), 73);

    mock.expectCall("cloneGame(4,joining)");
    mock.provideReturnValue(74);
    a.checkEqual("21. cloneGame", level4.cloneGame(4, HostGame::Joining), 74);

    // setType/State/Owner/Name
    mock.expectCall("setType(17,public)");
    AFL_CHECK_SUCCEEDS(a("31. setType"), level4.setType(17, HostGame::PublicGame));

    mock.expectCall("setState(17,finished)");
    AFL_CHECK_SUCCEEDS(a("41. setState"), level4.setState(17, HostGame::Finished));

    mock.expectCall("setOwner(17,1032)");
    AFL_CHECK_SUCCEEDS(a("51. setOwner"), level4.setOwner(17, "1032"));

    mock.expectCall("setName(98,Eightynine)");
    AFL_CHECK_SUCCEEDS(a("61. setName"), level4.setName(98, "Eightynine"));

    // getInfo
    // - full data
    {
        mock.expectCall("getInfo(17)");
        mock.provideReturnValue(makeInfo());

        HostGame::Info i = level4.getInfo(17);
        a.checkEqual("71. gameId",               i.gameId, 42);
        a.checkEqual("72. state",                i.state, HostGame::Running);
        a.checkEqual("73. type",                 i.type, HostGame::UnlistedGame);
        a.checkEqual("74. name",                 i.name, "Answer");
        a.check     ("75. description",          i.description.isSame(String_t("A Game")));
        a.checkEqual("76. difficulty",           i.difficulty, 96);
        a.check     ("77. currentSchedule",      i.currentSchedule.isValid());
        a.check     ("78. currentSchedule",      i.currentSchedule.get()->type.isSame(HostSchedule::Weekly));
        a.check     ("79. currentSchedule",      i.currentSchedule.get()->weekdays.isSame(19));
        a.check     ("80. currentSchedule",      i.currentSchedule.get()->daytime.isSame(600));
        a.check     ("81. currentSchedule",     !i.currentSchedule.get()->condition.isValid());
        a.check     ("82. slotStates",           i.slotStates.isValid());
        a.checkEqual("83. slotStates",           i.slotStates.get()->size(), 3U);
        a.checkEqual("84. slotStates",           i.slotStates.get()->at(0), HostGame::DeadSlot);
        a.checkEqual("85. slotStates",           i.slotStates.get()->at(1), HostGame::SelfSlot);
        a.checkEqual("86. slotStates",           i.slotStates.get()->at(2), HostGame::OccupiedSlot);
        a.check     ("87. turnStates",           i.turnStates.isValid());
        a.checkEqual("88. turnStates",           i.turnStates.get()->size(), 2U);
        a.checkEqual("89. turnStates",           i.turnStates.get()->at(0), 16);
        a.checkEqual("90. turnStates",           i.turnStates.get()->at(1), 1);
        a.check     ("91. joinable",             i.joinable.isSame(true));
        a.check     ("92. userPlays",            i.userPlays.isSame(true));
        a.check     ("93. scores",               i.scores.isValid());
        a.checkEqual("94. scores",               i.scores.get()->size(), 3U);
        a.checkEqual("95. scores",               i.scores.get()->at(0), 12);
        a.checkEqual("96. scores",               i.scores.get()->at(1), 167);
        a.checkEqual("97. scores",               i.scores.get()->at(2), 150);
        a.check     ("98. scoreName",            i.scoreName.isSame(String_t("escore")));
        a.check     ("99. scoreDescription",     i.scoreDescription.isSame(String_t("A Score")));
        a.checkEqual("100. hostName",            i.hostName, "qhost");
        a.checkEqual("101. hostDescription",     i.hostDescription, "Quality Host");
        a.checkEqual("102. hostKind",            i.hostKind, "qq");
        a.checkEqual("103. shipListName",        i.shipListName, "default");
        a.checkEqual("104. shipListDescription", i.shipListDescription, "Default List");
        a.checkEqual("105. shipListKind",        i.shipListKind, "slk");
        a.check     ("106. masterName",          i.masterName.isSame(String_t("qmaster")));
        a.check     ("107. masterDescription",   i.masterDescription.isSame(String_t("Quality Master")));
        a.check     ("108. masterKind",          i.masterKind.isSame(String_t("mk")));
        a.checkEqual("109. turnNumber",          i.turnNumber, 3);
        a.check     ("110. lastHostTime",        i.lastHostTime.isSame(1961));
        a.check     ("111. nextHostTime",        i.nextHostTime.isSame(1989));
        a.check     ("112. forumId",             i.forumId.isSame(23));
        a.check     ("113. userRank",            i.userRank.isSame(7));
        a.check     ("114. otherRank",           i.otherRank.isSame(8));
    }

    // - default (=minimal) data
    {
        mock.expectCall("getInfo(17)");
        mock.provideReturnValue(HostGame::Info());

        HostGame::Info i = level4.getInfo(17);
        a.checkEqual("121. gameId",      i.gameId, 0);
        a.checkEqual("122. state",       i.state, HostGame::Preparing);
        a.checkEqual("123. type",        i.type, HostGame::PrivateGame);
        a.checkEqual("124. name",        i.name, "");
        a.check("125. description",     !i.description.isValid());
        a.check("126. currentSchedule", !i.currentSchedule.isValid());
        a.check("127. turnStates",      !i.turnStates.isValid());
        a.check("128. forumId",         !i.forumId.isValid());
        a.check("129. userRank",        !i.userRank.isValid());
        a.check("130. otherRank",       !i.otherRank.isValid());
    }

    // getInfos
    {
        mock.expectCall("getInfos(-,-,-,-,-,-,-,-1,f)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(makeInfo());
        mock.provideReturnValue(HostGame::Info());

        std::vector<HostGame::Info> aa;
        level4.getInfos(HostGame::Filter(), false, aa);
        a.checkEqual("131. size", aa.size(), 2U);

        a.checkEqual("141. gameId", aa[0].gameId, 42);
        a.checkEqual("142. state",  aa[0].state, HostGame::Running);
        a.check("143. currentSchedule", aa[0].currentSchedule.isValid());
        a.check("144. currentSchedule", aa[0].currentSchedule.get()->weekdays.isSame(19));

        a.checkEqual("151. gameId", aa[1].gameId, 0);
        a.checkEqual("152. state",  aa[1].state, HostGame::Preparing);
        a.check("153. currentSchedule", !aa[1].currentSchedule.isValid());
    }

    {
        std::vector<HostGame::Info> aa;

        mock.expectCall("getInfos(running,-,-,-,-,-,-,-1,t)");
        mock.provideReturnValue(0);
        HostGame::Filter f1;
        f1.requiredState = HostGame::Running;
        AFL_CHECK_SUCCEEDS(a("161. getInfos"), level4.getInfos(f1, true, aa));

        mock.expectCall("getInfos(-,public,-,-,-,-,-,-1,f)");
        mock.provideReturnValue(0);
        HostGame::Filter f2;
        f2.requiredType = HostGame::PublicGame;
        AFL_CHECK_SUCCEEDS(a("171. getInfos"), level4.getInfos(f2, false, aa));

        mock.expectCall("getInfos(-,-,fred,-,-,-,-,-1,f)");
        mock.provideReturnValue(0);
        HostGame::Filter f3;
        f3.requiredUser = String_t("fred");
        AFL_CHECK_SUCCEEDS(a("181. getInfos"), level4.getInfos(f3, false, aa));

        mock.expectCall("getInfos(joining,unlisted,wilma,-,-,-,-,-1,t)");
        mock.provideReturnValue(0);
        HostGame::Filter f4;
        f4.requiredState = HostGame::Joining;
        f4.requiredType = HostGame::UnlistedGame;
        f4.requiredUser = String_t("wilma");
        AFL_CHECK_SUCCEEDS(a("191. getInfos"), level4.getInfos(f4, true, aa));

        mock.expectCall("getInfos(-,-,1003,qhost,multitool,list,pmaster,-1,t)");
        mock.provideReturnValue(0);
        HostGame::Filter f5;
        f5.requiredUser = String_t("1003");
        f5.requiredHost = String_t("qhost");
        f5.requiredTool = String_t("multitool");
        f5.requiredShipList = String_t("list");
        f5.requiredMaster = String_t("pmaster");
        AFL_CHECK_SUCCEEDS(a("201. getInfos"), level4.getInfos(f5, true, aa));

        a.checkEqual("211. size", aa.size(), 0U);
    }

    // getGames
    {
        mock.expectCall("getGames(-,-,-,-,-,-,-,-1)");
        mock.provideReturnValue(4);
        mock.provideReturnValue(89);
        mock.provideReturnValue(32);
        mock.provideReturnValue(16);
        mock.provideReturnValue(8);

        afl::data::IntegerList_t aa;
        AFL_CHECK_SUCCEEDS(a("221. getGames"), level4.getGames(HostGame::Filter(), aa));

        a.checkEqual("231. size", aa.size(), 4U);
        a.checkEqual("232. result", aa[0], 89);
        a.checkEqual("233. result", aa[1], 32);
        a.checkEqual("234. result", aa[2], 16);
        a.checkEqual("235. result", aa[3], 8);
    }
    {
        mock.expectCall("getGames(finished,private,1030,-,-,-,-,-1)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(3);
        mock.provideReturnValue(5);

        afl::data::IntegerList_t aa;
        HostGame::Filter filter;
        filter.requiredState = HostGame::Finished;
        filter.requiredType = HostGame::PrivateGame;
        filter.requiredUser = String_t("1030");
        AFL_CHECK_SUCCEEDS(a("241. getGames"), level4.getGames(filter, aa));

        a.checkEqual("251. size", aa.size(), 2U);
        a.checkEqual("252. result", aa[0], 3);
        a.checkEqual("253. result", aa[1], 5);
    }

    // setConfig
    {
        afl::data::StringList_t s;
        s.push_back("one");
        s.push_back("a");
        s.push_back("other");
        s.push_back("b");

        mock.expectCall("setConfig(8,one,a,other,b)");
        AFL_CHECK_SUCCEEDS(a("261. setConfig"), level4.setConfig(8, s));
    }
    {
        mock.expectCall("setConfig(5)");
        AFL_CHECK_SUCCEEDS(a("262. setConfig"), level4.setConfig(5, afl::data::StringList_t()));
    }

    // getConfig [single]
    mock.expectCall("getConfig(14,kk)");
    mock.provideReturnValue(String_t("zz"));
    a.checkEqual("271. getConfig", level4.getConfig(14, "kk"), "zz");

    // getConfig [multi]
    {
        mock.expectCall("getConfig(19,ha,hu,hi)");
        mock.provideReturnValue(3);
        mock.provideReturnValue(String_t("bla"));
        mock.provideReturnValue(String_t("blu"));
        mock.provideReturnValue(String_t("bli"));

        afl::data::StringList_t in;
        in.push_back("ha");
        in.push_back("hu");
        in.push_back("hi");

        afl::data::StringList_t out;
        AFL_CHECK_SUCCEEDS(a("281. getConfig"), level4.getConfig(19, in, out));

        a.checkEqual("291. size", out.size(), 3U);
        a.checkEqual("292. result", out[0], "bla");
        a.checkEqual("293. result", out[1], "blu");
        a.checkEqual("294. result", out[2], "bli");
    }

    // getComputedValue
    mock.expectCall("getComputedValue(8,ck)");
    mock.provideReturnValue(String_t("cv"));
    a.checkEqual("301. getComputedValue", level4.getComputedValue(8, "ck"), "cv");

    // getState
    mock.expectCall("getState(12)");
    mock.provideReturnValue(HostGame::Finished);
    a.checkEqual("311. getState", level4.getState(12), HostGame::Finished);

    // getType
    mock.expectCall("getType(23)");
    mock.provideReturnValue(HostGame::PublicGame);
    a.checkEqual("321. getType", level4.getType(23), HostGame::PublicGame);

    // getOwner
    mock.expectCall("getOwner(92)");
    mock.provideReturnValue(String_t("u96"));
    a.checkEqual("331. getOwner", level4.getOwner(92), "u96");

     // getName
    mock.expectCall("getName(76)");
    mock.provideReturnValue(String_t("gg"));
    a.checkEqual("341. getName", level4.getName(76), "gg");

    // getDirectory
    mock.expectCall("getDirectory(34)");
    mock.provideReturnValue(String_t("a/b/c"));
    a.checkEqual("351. getDirectory", level4.getDirectory(34), "a/b/c");

    // getPermissions
    {
        mock.expectCall("getPermissions(8,zz)");
        mock.provideReturnValue(HostGame::Permissions_t() + HostGame::UserIsOwner + HostGame::GameIsPublic);

        HostGame::Permissions_t p = level4.getPermissions(8, "zz");
        a.check("361. UserIsOwner",     p.contains(HostGame::UserIsOwner));
        a.check("362. GameIsPublic",    p.contains(HostGame::GameIsPublic));
        a.check("363. UserIsPrimary",  !p.contains(HostGame::UserIsPrimary));
        a.check("364. UserIsActive",   !p.contains(HostGame::UserIsActive));
        a.check("365. UserIsInactive", !p.contains(HostGame::UserIsInactive));
    }

    // addTool
    mock.expectCall("addTool(53,nt)");
    mock.provideReturnValue(true);
    a.checkEqual("371. addTool", level4.addTool(53, "nt"), true);

    // removeTool
    mock.expectCall("removeTool(57,ot)");
    mock.provideReturnValue(false);
    a.checkEqual("381. removeTool", level4.removeTool(57, "ot"), false);

    // getTools
    {
        mock.expectCall("getTools(56)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(HostTool::Info("ii", "dd", "kk", true));
        mock.provideReturnValue(HostTool::Info("i2", "d2", "k2", false));

        std::vector<HostTool::Info> aa;
        AFL_CHECK_SUCCEEDS(a("391. getTools"), level4.getTools(56, aa));

        a.checkEqual("401. size", aa.size(), 2U);
        a.checkEqual("402. id",          aa[0].id,          "ii");
        a.checkEqual("403. description", aa[0].description, "dd");
        a.checkEqual("404. kind",        aa[0].kind,        "kk");
        a.checkEqual("405. isDefault",   aa[0].isDefault,   true);
        a.checkEqual("406. id",          aa[1].id,          "i2");
        a.checkEqual("407. description", aa[1].description, "d2");
        a.checkEqual("408. kind",        aa[1].kind,        "k2");
        a.checkEqual("409. isDefault",   aa[1].isDefault,   false);
    }

    // getTotals
    {
        mock.expectCall("getTotals()");
        mock.provideReturnValue(HostGame::Totals(9, 3, 4));

        HostGame::Totals t = level4.getTotals();
        a.checkEqual("411. numJoiningGames", t.numJoiningGames, 9);
        a.checkEqual("412. numRunningGames", t.numRunningGames, 3);
        a.checkEqual("413. numFinishedGames", t.numFinishedGames, 4);
    }

    // getVictoryCondition
    {
        HostGame::VictoryCondition vc;
        vc.endCondition = "ee";
        vc.endTurn = 62;
        vc.endProbability = 5;
        vc.endScore = 99;
        vc.endScoreName = "esn";
        vc.endScoreDescription = "esd";
        vc.referee = "ref";
        vc.refereeDescription = "refd";

        mock.expectCall("getVictoryCondition(18)");
        mock.provideReturnValue(vc);

        HostGame::VictoryCondition aa = level4.getVictoryCondition(18);
        a.checkEqual("421. endCondition",   aa.endCondition, "ee");
        a.check("422. endTurn",             aa.endTurn.isSame(62));
        a.check("423. endProbability",      aa.endProbability.isSame(5));
        a.check("424. endScore",            aa.endScore.isSame(99));
        a.check("425. endScoreName",        aa.endScoreName.isSame(String_t("esn")));
        a.check("426. endScoreDescription", aa.endScoreDescription.isSame(String_t("esd")));
        a.check("427. referee",             aa.referee.isSame(String_t("ref")));
        a.check("428. refereeDescription",  aa.refereeDescription.isSame(String_t("refd")));
    }
    {
        mock.expectCall("getVictoryCondition(18)");
        mock.provideReturnValue(HostGame::VictoryCondition());

        HostGame::VictoryCondition aa = level4.getVictoryCondition(18);
        a.checkEqual("431. endCondition",    aa.endCondition, "");
        a.check("432. endTurn",             !aa.endTurn.isValid());
        a.check("433. endProbability",      !aa.endProbability.isValid());
        a.check("434. endScore",            !aa.endScore.isValid());
        a.check("435. endScoreName",        !aa.endScoreName.isValid());
        a.check("436. endScoreDescription", !aa.endScoreDescription.isValid());
        a.check("437. referee",             !aa.referee.isValid());
        a.check("438. refereeDescription",  !aa.refereeDescription.isValid());
    }

    // updateGames
    {
        afl::data::IntegerList_t is;
        is.push_back(1);
        is.push_back(3);
        is.push_back(5);

        mock.expectCall("updateGames(1,3,5)");
        AFL_CHECK_SUCCEEDS(a("441. updateGames"), level4.updateGames(is));
    }
    {
        mock.expectCall("updateGames()");
        AFL_CHECK_SUCCEEDS(a("442. updateGames"), level4.updateGames(afl::data::IntegerList_t()));
    }

    // resetToTurn
    mock.expectCall("resetToTurn(22,12)");
    AFL_CHECK_SUCCEEDS(a("451. resetToTurn"), level4.resetToTurn(22,12));

    mock.checkFinish();
}
