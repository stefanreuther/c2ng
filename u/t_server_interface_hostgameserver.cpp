/**
  *  \file u/t_server_interface_hostgameserver.cpp
  *  \brief Test for server::interface::HostGameServer
  */

#include "server/interface/hostgameserver.hpp"

#include <stdexcept>
#include "t_server_interface.hpp"
#include "afl/data/access.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/interface/hostgameclient.hpp"

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
void
TestServerInterfaceHostGameServer::testIt()
{
    HostGameMock mock("testIt");
    server::interface::HostGameServer testee(mock);

    // createNewGame
    mock.expectCall("createNewGame()");
    mock.provideReturnValue(72);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("NEWGAME")), 72);

    // cloneGame
    mock.expectCall("cloneGame(3,-)");
    mock.provideReturnValue(73);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("CLONEGAME").pushBackInteger(3)), 73);

    mock.expectCall("cloneGame(4,joining)");
    mock.provideReturnValue(74);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("CLONEGAME").pushBackInteger(4).pushBackString("joining")), 74);

    // setType/State/Owner/Name
    mock.expectCall("setType(17,public)");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("GAMESETTYPE").pushBackInteger(17).pushBackString("public")));

    mock.expectCall("setState(17,finished)");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("GAMESETSTATE").pushBackInteger(17).pushBackString("finished")));

    mock.expectCall("setOwner(17,1032)");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("GAMESETOWNER").pushBackInteger(17).pushBackString("1032")));

    mock.expectCall("setName(98,Eightynine)");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("GAMESETNAME").pushBackInteger(98).pushBackString("Eightynine")));

    // getInfo
    // - full data
    {
        mock.expectCall("getInfo(17)");
        mock.provideReturnValue(makeInfo());

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMESTAT").pushBackInteger(17)));
        Access a(p);
        TS_ASSERT_EQUALS(a("id").toInteger(), 42);
        TS_ASSERT_EQUALS(a("state").toString(), "running");
        TS_ASSERT_EQUALS(a("type").toString(), "unlisted");
        TS_ASSERT_EQUALS(a("name").toString(), "Answer");
        TS_ASSERT_EQUALS(a("description").toString(), "A Game");
        TS_ASSERT_EQUALS(a("difficulty").toInteger(), 96);
        TS_ASSERT_EQUALS(a("currentSchedule")("type").toInteger(), 1);
        TS_ASSERT_EQUALS(a("currentSchedule")("weekdays").toInteger(), 19);
        TS_ASSERT_EQUALS(a("currentSchedule")("daytime").toInteger(), 600);
        TS_ASSERT(a("currentSchedule")("condition").getValue() == 0);
        TS_ASSERT_EQUALS(a("slots").getArraySize(), 3U);
        TS_ASSERT_EQUALS(a("slots")[0].toString(), "dead");
        TS_ASSERT_EQUALS(a("slots")[1].toString(), "self");
        TS_ASSERT_EQUALS(a("slots")[2].toString(), "occupied");
        TS_ASSERT_EQUALS(a("turns").getArraySize(), 2U);
        TS_ASSERT_EQUALS(a("turns")[0].toInteger(), 16);
        TS_ASSERT_EQUALS(a("turns")[1].toInteger(), 1);
        TS_ASSERT_EQUALS(a("joinable").toInteger(), 1);
        TS_ASSERT_EQUALS(a("userPlays").toInteger(), 1);
        TS_ASSERT_EQUALS(a("scores").getArraySize(), 3U);
        TS_ASSERT_EQUALS(a("scores")[0].toInteger(), 12);
        TS_ASSERT_EQUALS(a("scores")[1].toInteger(), 167);
        TS_ASSERT_EQUALS(a("scores")[2].toInteger(), 150);
        TS_ASSERT_EQUALS(a("scoreName").toString(), "escore");
        TS_ASSERT_EQUALS(a("scoreDescription").toString(), "A Score");
        TS_ASSERT_EQUALS(a("minRankLevelToJoin").toInteger(), 10);
        TS_ASSERT_EQUALS(a("maxRankLevelToJoin").toInteger(), 11);
        TS_ASSERT_EQUALS(a("minRankPointsToJoin").toInteger(), 22);
        TS_ASSERT_EQUALS(a("maxRankPointsToJoin").toInteger(), 23);
        TS_ASSERT_EQUALS(a("host").toString(), "qhost");
        TS_ASSERT_EQUALS(a("hostDescription").toString(), "Quality Host");
        TS_ASSERT_EQUALS(a("hostKind").toString(), "qq");
        TS_ASSERT_EQUALS(a("shiplist").toString(), "default");
        TS_ASSERT_EQUALS(a("shiplistDescription").toString(), "Default List");
        TS_ASSERT_EQUALS(a("shiplistKind").toString(), "slk");
        TS_ASSERT_EQUALS(a("master").toString(), "qmaster");
        TS_ASSERT_EQUALS(a("masterDescription").toString(), "Quality Master");
        TS_ASSERT_EQUALS(a("masterKind").toString(), "mk");
        TS_ASSERT_EQUALS(a("turn").toInteger(), 3);
        TS_ASSERT_EQUALS(a("lastHostTime").toInteger(), 1961);
        TS_ASSERT_EQUALS(a("nextHostTime").toInteger(), 1989);
        TS_ASSERT_EQUALS(a("forum").toInteger(), 23);
        TS_ASSERT_EQUALS(a("userRank").toInteger(), 7);
        TS_ASSERT_EQUALS(a("otherRank").toInteger(), 8);
    }

    // - default (=minimal) data
    {
        mock.expectCall("getInfo(17)");
        mock.provideReturnValue(HostGame::Info());

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMESTAT").pushBackInteger(17)));
        Access a(p);
        TS_ASSERT_EQUALS(a("id").toInteger(), 0);
        TS_ASSERT_EQUALS(a("state").toString(), "preparing");
        TS_ASSERT_EQUALS(a("type").toString(), "private");
        TS_ASSERT_EQUALS(a("name").toString(), "");
        TS_ASSERT(a("description").getValue() == 0);
        TS_ASSERT(a("currentSchedule").getValue() == 0);
        TS_ASSERT(a("turns").getValue() == 0);
        TS_ASSERT(a("forum").getValue() == 0);
        TS_ASSERT(a("userRank").getValue() == 0);
        TS_ASSERT(a("otherRank").getValue() == 0);
    }

    // getInfos
    {
        mock.expectCall("getInfos(-,-,-,-,-,-,-,-1,f)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(makeInfo());
        mock.provideReturnValue(HostGame::Info());

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMELIST")));
        Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
        TS_ASSERT_EQUALS(a[0]("id").toInteger(), 42);
        TS_ASSERT_EQUALS(a[0]("state").toString(), "running");
        TS_ASSERT_EQUALS(a[0]("currentSchedule")("weekdays").toInteger(), 19);
        TS_ASSERT_EQUALS(a[1]("id").toInteger(), 0);
        TS_ASSERT_EQUALS(a[1]("state").toString(), "preparing");
        TS_ASSERT(a[1]("currentSchedule").getValue() == 0);
        TS_ASSERT_EQUALS(a[1]("currentSchedule")("weekdays").toInteger(), 0);
    }
    {
        mock.expectCall("getInfos(running,-,-,-,-,-,-,-1,t)");
        mock.provideReturnValue(0);
        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMELIST").pushBackString("STATE").pushBackString("running").pushBackString("VERBOSE")));
        Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 0U);
    }
    {
        mock.expectCall("getInfos(-,public,-,-,-,-,-,-1,f)");
        mock.provideReturnValue(0);
        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMELIST").pushBackString("TYPE").pushBackString("public")));
        Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 0U);
    }
    {
        mock.expectCall("getInfos(-,-,fred,-,-,-,-,-1,f)");
        mock.provideReturnValue(0);
        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMELIST").pushBackString("USER").pushBackString("fred")));
        Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 0U);
    }
    {
        mock.expectCall("getInfos(joining,unlisted,wilma,-,-,-,-,-1,t)");
        mock.provideReturnValue(0);
        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMELIST").pushBackString("USER").pushBackString("wilma").pushBackString("VERBOSE").
                                             pushBackString("TYPE").pushBackString("unlisted").pushBackString("STATE").pushBackString("joining")));
        Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 0U);
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
        Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 4U);
        TS_ASSERT_EQUALS(a[0].toInteger(), 89);
        TS_ASSERT_EQUALS(a[1].toInteger(), 32);
        TS_ASSERT_EQUALS(a[2].toInteger(), 16);
        TS_ASSERT_EQUALS(a[3].toInteger(), 8);
    }
    {
        mock.expectCall("getGames(finished,private,1030,-,-,-,-,-1)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(3);
        mock.provideReturnValue(5);

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMELIST").pushBackString("TYPE").pushBackString("private").
                                             pushBackString("STATE").pushBackString("finished").pushBackString("ID").pushBackString("USER").pushBackString("1030")));
        Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
        TS_ASSERT_EQUALS(a[0].toInteger(), 3);
        TS_ASSERT_EQUALS(a[1].toInteger(), 5);
    }

    // setConfig
    mock.expectCall("setConfig(8,one,a,other,b)");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("GAMESET").pushBackInteger(8).pushBackString("one").pushBackString("a").pushBackString("other").pushBackString("b")));

    mock.expectCall("setConfig(5)");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("GAMESET").pushBackInteger(5)));

    // getConfig [single]
    mock.expectCall("getConfig(14,kk)");
    mock.provideReturnValue(String_t("zz"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("GAMEGET").pushBackInteger(14).pushBackString("kk")), "zz");

    // getConfig [multi]
    {
        mock.expectCall("getConfig(19,ha,hu,hi)");
        mock.provideReturnValue(3);
        mock.provideReturnValue(String_t("bla"));
        mock.provideReturnValue(String_t("blu"));
        mock.provideReturnValue(String_t("bli"));

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMEMGET").pushBackInteger(19).pushBackString("ha").pushBackString("hu").pushBackString("hi")));
        Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 3U);
        TS_ASSERT_EQUALS(a[0].toString(), "bla");
        TS_ASSERT_EQUALS(a[1].toString(), "blu");
        TS_ASSERT_EQUALS(a[2].toString(), "bli");
    }

    // getComputedValue
    mock.expectCall("getComputedValue(8,ck)");
    mock.provideReturnValue(String_t("cv"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("GAMEGETCC").pushBackInteger(8).pushBackString("ck")), "cv");

    // getState
    mock.expectCall("getState(12)");
    mock.provideReturnValue(HostGame::Finished);
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("GAMEGETSTATE").pushBackInteger(12)), "finished");

    // getType
    mock.expectCall("getType(23)");
    mock.provideReturnValue(HostGame::PublicGame);
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("GAMEGETTYPE").pushBackInteger(23)), "public");

    // getOwner
    mock.expectCall("getOwner(92)");
    mock.provideReturnValue(String_t("u96"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("GAMEGETOWNER").pushBackInteger(92)), "u96");

    // getName
    mock.expectCall("getName(76)");
    mock.provideReturnValue(String_t("gg"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("GAMEGETNAME").pushBackInteger(76)), "gg");

    // getDirectory
    mock.expectCall("getDirectory(34)");
    mock.provideReturnValue(String_t("a/b/c"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("GAMEGETDIR").pushBackInteger(34)), "a/b/c");

    // getPermissions
    mock.expectCall("getPermissions(8,zz)");
    mock.provideReturnValue(HostGame::Permissions_t() + HostGame::UserIsOwner + HostGame::GameIsPublic);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("GAMECHECKPERM").pushBackInteger(8).pushBackString("zz")), 17);

    // addTool
    mock.expectCall("addTool(53,nt)");
    mock.provideReturnValue(true);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("GAMEADDTOOL").pushBackInteger(53).pushBackString("nt")), 1);

    // removeTool
    mock.expectCall("removeTool(57,ot)");
    mock.provideReturnValue(false);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("GAMERMTOOL").pushBackInteger(57).pushBackString("ot")), 0);

    // getTools
    {
        mock.expectCall("getTools(56)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(HostTool::Info("ii", "dd", "kk", true));
        mock.provideReturnValue(HostTool::Info("i2", "d2", "k2", false));

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMELSTOOLS").pushBackInteger(56)));
        Access a(p);
        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
        TS_ASSERT_EQUALS(a[0]("id").toString(),          "ii");
        TS_ASSERT_EQUALS(a[0]("description").toString(), "dd");
        TS_ASSERT_EQUALS(a[0]("kind").toString(),        "kk");
        TS_ASSERT_EQUALS(a[0]("default").toInteger(),    1);
        TS_ASSERT_EQUALS(a[1]("id").toString(),          "i2");
        TS_ASSERT_EQUALS(a[1]("description").toString(), "d2");
        TS_ASSERT_EQUALS(a[1]("kind").toString(),        "k2");
        TS_ASSERT_EQUALS(a[1]("default").toInteger(),    0);
    }

    // getTotals
    {
        mock.expectCall("getTotals()");
        mock.provideReturnValue(HostGame::Totals(9, 3, 4));

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMETOTALS")));
        Access a(p);

        TS_ASSERT_EQUALS(a("joining").toInteger(), 9);
        TS_ASSERT_EQUALS(a("running").toInteger(), 3);
        TS_ASSERT_EQUALS(a("finished").toInteger(), 4);
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
        Access a(p);

        TS_ASSERT_EQUALS(a("endCondition").toString(), "ee");
        TS_ASSERT_EQUALS(a("endTurn").toInteger(), 62);
        TS_ASSERT_EQUALS(a("endProbability").toInteger(), 5);
        TS_ASSERT_EQUALS(a("endScore").toInteger(), 99);
        TS_ASSERT_EQUALS(a("endScoreName").toString(), "esn");
        TS_ASSERT_EQUALS(a("endScoreDescription").toString(), "esd");
        TS_ASSERT_EQUALS(a("referee").toString(), "ref");
        TS_ASSERT_EQUALS(a("refereeDescription").toString(), "refd");
    }
    {
        mock.expectCall("getVictoryCondition(18)");
        mock.provideReturnValue(HostGame::VictoryCondition());

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("GAMEGETVC").pushBackInteger(18)));
        Access a(p);

        TS_ASSERT_EQUALS(a("endCondition").toString(), "");
        TS_ASSERT(a("endTurn").getValue() == 0);
        TS_ASSERT(a("endProbability").getValue() == 0);
        TS_ASSERT(a("endScore").getValue() == 0);
        TS_ASSERT(a("endScoreName").getValue() == 0);
        TS_ASSERT(a("endScoreDescription").getValue() == 0);
        TS_ASSERT(a("referee").getValue() == 0);
        TS_ASSERT(a("refereeDescription").getValue() == 0);
    }

    // updateGames
    mock.expectCall("updateGames(1,3,5)");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("GAMEUPDATE").pushBackInteger(1).pushBackInteger(3).pushBackInteger(5)));

    mock.expectCall("updateGames()");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("GAMEUPDATE")));

    // resetToTurn
    mock.expectCall("resetToTurn(7,22)");
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("GAMERESET").pushBackInteger(7).pushBackInteger(22)));

    // Variations
    mock.expectCall("createNewGame()");
    mock.provideReturnValue(99);
    TS_ASSERT_EQUALS(testee.callInt(Segment().pushBackString("newGame")), 99);

    mock.expectCall("getInfos(running,-,-,-,-,-,-,-1,t)");
    mock.provideReturnValue(0);
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("GAMELIST").pushBackString("state").pushBackString("running").pushBackString("verbose")));

    mock.expectCall("getInfos(running,-,-,-,-,-,-,-1,t)");
    mock.provideReturnValue(0);
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("GAMELIST").pushBackString("STATE").pushBackString("running").pushBackString("ID").pushBackString("VERBOSE")));

    mock.expectCall("getInfos(-,-,-,-,-,-,-,99,f)");
    mock.provideReturnValue(0);
    TS_ASSERT_THROWS_NOTHING(testee.callVoid(Segment().pushBackString("GAMELIST").pushBackString("COPYOF").pushBackInteger(99)));

    mock.checkFinish();
}

/** Test errors. */
void
TestServerInterfaceHostGameServer::testErrors()
{
    HostGameMock mock("testErrors");
    server::interface::HostGameServer testee(mock);

    // Number of parameters
    Segment empty;
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("NEWGAME").pushBackInteger(3)), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("CLONEGAME")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GAMELIST").pushBackString("STATE")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GAMELIST").pushBackString("TYPE")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GAMELIST").pushBackString("USER")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GAMEMGET")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GAMETOTALS").pushBackInteger(9)), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GAMERESET").pushBackInteger(7)), std::exception);

    // Bad commands or keywords
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("HI")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GAMELIST").pushBackString("FUN")), std::exception);

    // Bad status, type, etc
    TS_ASSERT_THROWS(testee.callInt(Segment().pushBackString("CLONEGAME").pushBackInteger(4).pushBackString("JOINING")), std::exception);
    TS_ASSERT_THROWS(testee.callInt(Segment().pushBackString("CLONEGAME").pushBackInteger(4).pushBackString("")), std::exception);
    TS_ASSERT_THROWS(testee.callInt(Segment().pushBackString("CLONEGAME").pushBackInteger(4).pushBackString("x")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GAMESETTYPE").pushBackInteger(3).pushBackString("ha")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GAMESETSTATE").pushBackInteger(3).pushBackString("hu")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GAMELIST").pushBackString("STATE").pushBackString("jumping")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GAMELIST").pushBackString("STATE").pushBackString("RUNNING")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GAMELIST").pushBackString("TYPE").pushBackString("typing")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GAMEMGET").pushBackString("hu")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GAMERESET").pushBackInteger(7).pushBackString("asdljlad")), std::exception);

    mock.checkFinish();
}

/** Test roundtrip with client. */
void
TestServerInterfaceHostGameServer::testRoundtrip()
{
    HostGameMock mock("testRoundtrip");
    server::interface::HostGameServer level1(mock);
    server::interface::HostGameClient level2(level1);
    server::interface::HostGameServer level3(level2);
    server::interface::HostGameClient level4(level3);

    // createNewGame
    mock.expectCall("createNewGame()");
    mock.provideReturnValue(72);
    TS_ASSERT_EQUALS(level4.createNewGame(), 72);

    // cloneGame
    mock.expectCall("cloneGame(3,-)");
    mock.provideReturnValue(73);
    TS_ASSERT_EQUALS(level4.cloneGame(3, afl::base::Nothing), 73);

    mock.expectCall("cloneGame(4,joining)");
    mock.provideReturnValue(74);
    TS_ASSERT_EQUALS(level4.cloneGame(4, HostGame::Joining), 74);

    // setType/State/Owner/Name
    mock.expectCall("setType(17,public)");
    TS_ASSERT_THROWS_NOTHING(level4.setType(17, HostGame::PublicGame));

    mock.expectCall("setState(17,finished)");
    TS_ASSERT_THROWS_NOTHING(level4.setState(17, HostGame::Finished));

    mock.expectCall("setOwner(17,1032)");
    TS_ASSERT_THROWS_NOTHING(level4.setOwner(17, "1032"));

    mock.expectCall("setName(98,Eightynine)");
    TS_ASSERT_THROWS_NOTHING(level4.setName(98, "Eightynine"));

    // getInfo
    // - full data
    {
        mock.expectCall("getInfo(17)");
        mock.provideReturnValue(makeInfo());

        HostGame::Info i = level4.getInfo(17);
        TS_ASSERT_EQUALS(i.gameId, 42);
        TS_ASSERT_EQUALS(i.state, HostGame::Running);
        TS_ASSERT_EQUALS(i.type, HostGame::UnlistedGame);
        TS_ASSERT_EQUALS(i.name, "Answer");
        TS_ASSERT(i.description.isSame(String_t("A Game")));
        TS_ASSERT_EQUALS(i.difficulty, 96);
        TS_ASSERT(i.currentSchedule.isValid());
        TS_ASSERT(i.currentSchedule.get()->type.isSame(HostSchedule::Weekly));
        TS_ASSERT(i.currentSchedule.get()->weekdays.isSame(19));
        TS_ASSERT(i.currentSchedule.get()->daytime.isSame(600));
        TS_ASSERT(!i.currentSchedule.get()->condition.isValid());
        TS_ASSERT(i.slotStates.isValid());
        TS_ASSERT_EQUALS(i.slotStates.get()->size(), 3U);
        TS_ASSERT_EQUALS(i.slotStates.get()->at(0), HostGame::DeadSlot);
        TS_ASSERT_EQUALS(i.slotStates.get()->at(1), HostGame::SelfSlot);
        TS_ASSERT_EQUALS(i.slotStates.get()->at(2), HostGame::OccupiedSlot);
        TS_ASSERT(i.turnStates.isValid());
        TS_ASSERT_EQUALS(i.turnStates.get()->size(), 2U);
        TS_ASSERT_EQUALS(i.turnStates.get()->at(0), 16);
        TS_ASSERT_EQUALS(i.turnStates.get()->at(1), 1);
        TS_ASSERT(i.joinable.isSame(true));
        TS_ASSERT(i.userPlays.isSame(true));
        TS_ASSERT(i.scores.isValid());
        TS_ASSERT_EQUALS(i.scores.get()->size(), 3U);
        TS_ASSERT_EQUALS(i.scores.get()->at(0), 12);
        TS_ASSERT_EQUALS(i.scores.get()->at(1), 167);
        TS_ASSERT_EQUALS(i.scores.get()->at(2), 150);
        TS_ASSERT(i.scoreName.isSame(String_t("escore")));
        TS_ASSERT(i.scoreDescription.isSame(String_t("A Score")));
        TS_ASSERT_EQUALS(i.hostName, "qhost");
        TS_ASSERT_EQUALS(i.hostDescription, "Quality Host");
        TS_ASSERT_EQUALS(i.hostKind, "qq");
        TS_ASSERT_EQUALS(i.shipListName, "default");
        TS_ASSERT_EQUALS(i.shipListDescription, "Default List");
        TS_ASSERT_EQUALS(i.shipListKind, "slk");
        TS_ASSERT(i.masterName.isSame(String_t("qmaster")));
        TS_ASSERT(i.masterDescription.isSame(String_t("Quality Master")));
        TS_ASSERT(i.masterKind.isSame(String_t("mk")));
        TS_ASSERT_EQUALS(i.turnNumber, 3);
        TS_ASSERT(i.lastHostTime.isSame(1961));
        TS_ASSERT(i.nextHostTime.isSame(1989));
        TS_ASSERT(i.forumId.isSame(23));
        TS_ASSERT(i.userRank.isSame(7));
        TS_ASSERT(i.otherRank.isSame(8));
    }

    // - default (=minimal) data
    {
        mock.expectCall("getInfo(17)");
        mock.provideReturnValue(HostGame::Info());

        HostGame::Info i = level4.getInfo(17);
        TS_ASSERT_EQUALS(i.gameId, 0);
        TS_ASSERT_EQUALS(i.state, HostGame::Preparing);
        TS_ASSERT_EQUALS(i.type, HostGame::PrivateGame);
        TS_ASSERT_EQUALS(i.name, "");
        TS_ASSERT(!i.description.isValid());
        TS_ASSERT(!i.currentSchedule.isValid());
        TS_ASSERT(!i.turnStates.isValid());
        TS_ASSERT(!i.forumId.isValid());
        TS_ASSERT(!i.userRank.isValid());
        TS_ASSERT(!i.otherRank.isValid());
    }

    // getInfos
    {
        mock.expectCall("getInfos(-,-,-,-,-,-,-,-1,f)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(makeInfo());
        mock.provideReturnValue(HostGame::Info());

        std::vector<HostGame::Info> a;
        level4.getInfos(HostGame::Filter(), false, a);
        TS_ASSERT_EQUALS(a.size(), 2U);

        TS_ASSERT_EQUALS(a[0].gameId, 42);
        TS_ASSERT_EQUALS(a[0].state, HostGame::Running);
        TS_ASSERT(a[0].currentSchedule.isValid());
        TS_ASSERT(a[0].currentSchedule.get()->weekdays.isSame(19));

        TS_ASSERT_EQUALS(a[1].gameId, 0);
        TS_ASSERT_EQUALS(a[1].state, HostGame::Preparing);
        TS_ASSERT(!a[1].currentSchedule.isValid());
    }

    {
        std::vector<HostGame::Info> a;

        mock.expectCall("getInfos(running,-,-,-,-,-,-,-1,t)");
        mock.provideReturnValue(0);
        HostGame::Filter f1;
        f1.requiredState = HostGame::Running;
        TS_ASSERT_THROWS_NOTHING(level4.getInfos(f1, true, a));

        mock.expectCall("getInfos(-,public,-,-,-,-,-,-1,f)");
        mock.provideReturnValue(0);
        HostGame::Filter f2;
        f2.requiredType = HostGame::PublicGame;
        TS_ASSERT_THROWS_NOTHING(level4.getInfos(f2, false, a));

        mock.expectCall("getInfos(-,-,fred,-,-,-,-,-1,f)");
        mock.provideReturnValue(0);
        HostGame::Filter f3;
        f3.requiredUser = String_t("fred");
        TS_ASSERT_THROWS_NOTHING(level4.getInfos(f3, false, a));

        mock.expectCall("getInfos(joining,unlisted,wilma,-,-,-,-,-1,t)");
        mock.provideReturnValue(0);
        HostGame::Filter f4;
        f4.requiredState = HostGame::Joining;
        f4.requiredType = HostGame::UnlistedGame;
        f4.requiredUser = String_t("wilma");
        TS_ASSERT_THROWS_NOTHING(level4.getInfos(f4, true, a));

        mock.expectCall("getInfos(-,-,1003,qhost,multitool,list,pmaster,-1,t)");
        mock.provideReturnValue(0);
        HostGame::Filter f5;
        f5.requiredUser = String_t("1003");
        f5.requiredHost = String_t("qhost");
        f5.requiredTool = String_t("multitool");
        f5.requiredShipList = String_t("list");
        f5.requiredMaster = String_t("pmaster");
        TS_ASSERT_THROWS_NOTHING(level4.getInfos(f5, true, a));

        TS_ASSERT_EQUALS(a.size(), 0U);
    }

    // getGames
    {
        mock.expectCall("getGames(-,-,-,-,-,-,-,-1)");
        mock.provideReturnValue(4);
        mock.provideReturnValue(89);
        mock.provideReturnValue(32);
        mock.provideReturnValue(16);
        mock.provideReturnValue(8);

        afl::data::IntegerList_t a;
        TS_ASSERT_THROWS_NOTHING(level4.getGames(HostGame::Filter(), a));

        TS_ASSERT_EQUALS(a.size(), 4U);
        TS_ASSERT_EQUALS(a[0], 89);
        TS_ASSERT_EQUALS(a[1], 32);
        TS_ASSERT_EQUALS(a[2], 16);
        TS_ASSERT_EQUALS(a[3], 8);
    }
    {
        mock.expectCall("getGames(finished,private,1030,-,-,-,-,-1)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(3);
        mock.provideReturnValue(5);

        afl::data::IntegerList_t a;
        HostGame::Filter filter;
        filter.requiredState = HostGame::Finished;
        filter.requiredType = HostGame::PrivateGame;
        filter.requiredUser = String_t("1030");
        TS_ASSERT_THROWS_NOTHING(level4.getGames(filter, a));

        TS_ASSERT_EQUALS(a.size(), 2U);
        TS_ASSERT_EQUALS(a[0], 3);
        TS_ASSERT_EQUALS(a[1], 5);
    }

    // setConfig
    {
        afl::data::StringList_t s;
        s.push_back("one");
        s.push_back("a");
        s.push_back("other");
        s.push_back("b");

        mock.expectCall("setConfig(8,one,a,other,b)");
        TS_ASSERT_THROWS_NOTHING(level4.setConfig(8, s));
    }
    {
        mock.expectCall("setConfig(5)");
        TS_ASSERT_THROWS_NOTHING(level4.setConfig(5, afl::data::StringList_t()));
    }

    // getConfig [single]
    mock.expectCall("getConfig(14,kk)");
    mock.provideReturnValue(String_t("zz"));
    TS_ASSERT_EQUALS(level4.getConfig(14, "kk"), "zz");

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
        TS_ASSERT_THROWS_NOTHING(level4.getConfig(19, in, out));

        TS_ASSERT_EQUALS(out.size(), 3U);
        TS_ASSERT_EQUALS(out[0], "bla");
        TS_ASSERT_EQUALS(out[1], "blu");
        TS_ASSERT_EQUALS(out[2], "bli");
    }

    // getComputedValue
    mock.expectCall("getComputedValue(8,ck)");
    mock.provideReturnValue(String_t("cv"));
    TS_ASSERT_EQUALS(level4.getComputedValue(8, "ck"), "cv");

    // getState
    mock.expectCall("getState(12)");
    mock.provideReturnValue(HostGame::Finished);
    TS_ASSERT_EQUALS(level4.getState(12), HostGame::Finished);

    // getType
    mock.expectCall("getType(23)");
    mock.provideReturnValue(HostGame::PublicGame);
    TS_ASSERT_EQUALS(level4.getType(23), HostGame::PublicGame);

    // getOwner
    mock.expectCall("getOwner(92)");
    mock.provideReturnValue(String_t("u96"));
    TS_ASSERT_EQUALS(level4.getOwner(92), "u96");

     // getName
    mock.expectCall("getName(76)");
    mock.provideReturnValue(String_t("gg"));
    TS_ASSERT_EQUALS(level4.getName(76), "gg");

    // getDirectory
    mock.expectCall("getDirectory(34)");
    mock.provideReturnValue(String_t("a/b/c"));
    TS_ASSERT_EQUALS(level4.getDirectory(34), "a/b/c");

    // getPermissions
    {
        mock.expectCall("getPermissions(8,zz)");
        mock.provideReturnValue(HostGame::Permissions_t() + HostGame::UserIsOwner + HostGame::GameIsPublic);

        HostGame::Permissions_t p = level4.getPermissions(8, "zz");
        TS_ASSERT(p.contains(HostGame::UserIsOwner));
        TS_ASSERT(p.contains(HostGame::GameIsPublic));
        TS_ASSERT(!p.contains(HostGame::UserIsPrimary));
        TS_ASSERT(!p.contains(HostGame::UserIsActive));
        TS_ASSERT(!p.contains(HostGame::UserIsInactive));
    }

    // addTool
    mock.expectCall("addTool(53,nt)");
    mock.provideReturnValue(true);
    TS_ASSERT_EQUALS(level4.addTool(53, "nt"), true);

    // removeTool
    mock.expectCall("removeTool(57,ot)");
    mock.provideReturnValue(false);
    TS_ASSERT_EQUALS(level4.removeTool(57, "ot"), false);

    // getTools
    {
        mock.expectCall("getTools(56)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(HostTool::Info("ii", "dd", "kk", true));
        mock.provideReturnValue(HostTool::Info("i2", "d2", "k2", false));

        std::vector<HostTool::Info> a;
        TS_ASSERT_THROWS_NOTHING(level4.getTools(56, a));

        TS_ASSERT_EQUALS(a.size(), 2U);
        TS_ASSERT_EQUALS(a[0].id,          "ii");
        TS_ASSERT_EQUALS(a[0].description, "dd");
        TS_ASSERT_EQUALS(a[0].kind,        "kk");
        TS_ASSERT_EQUALS(a[0].isDefault,   true);
        TS_ASSERT_EQUALS(a[1].id,          "i2");
        TS_ASSERT_EQUALS(a[1].description, "d2");
        TS_ASSERT_EQUALS(a[1].kind,        "k2");
        TS_ASSERT_EQUALS(a[1].isDefault,   false);
    }

    // getTotals
    {
        mock.expectCall("getTotals()");
        mock.provideReturnValue(HostGame::Totals(9, 3, 4));

        HostGame::Totals t = level4.getTotals();
        TS_ASSERT_EQUALS(t.numJoiningGames, 9);
        TS_ASSERT_EQUALS(t.numRunningGames, 3);
        TS_ASSERT_EQUALS(t.numFinishedGames, 4);
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

        HostGame::VictoryCondition a = level4.getVictoryCondition(18);
        TS_ASSERT_EQUALS(a.endCondition, "ee");
        TS_ASSERT(a.endTurn.isSame(62));
        TS_ASSERT(a.endProbability.isSame(5));
        TS_ASSERT(a.endScore.isSame(99));
        TS_ASSERT(a.endScoreName.isSame(String_t("esn")));
        TS_ASSERT(a.endScoreDescription.isSame(String_t("esd")));
        TS_ASSERT(a.referee.isSame(String_t("ref")));
        TS_ASSERT(a.refereeDescription.isSame(String_t("refd")));
    }
    {
        mock.expectCall("getVictoryCondition(18)");
        mock.provideReturnValue(HostGame::VictoryCondition());

        HostGame::VictoryCondition a = level4.getVictoryCondition(18);
        TS_ASSERT_EQUALS(a.endCondition, "");
        TS_ASSERT(!a.endTurn.isValid());
        TS_ASSERT(!a.endProbability.isValid());
        TS_ASSERT(!a.endScore.isValid());
        TS_ASSERT(!a.endScoreName.isValid());
        TS_ASSERT(!a.endScoreDescription.isValid());
        TS_ASSERT(!a.referee.isValid());
        TS_ASSERT(!a.refereeDescription.isValid());
    }

    // updateGames
    {
        afl::data::IntegerList_t is;
        is.push_back(1);
        is.push_back(3);
        is.push_back(5);

        mock.expectCall("updateGames(1,3,5)");
        TS_ASSERT_THROWS_NOTHING(level4.updateGames(is));
    }
    {
        mock.expectCall("updateGames()");
        TS_ASSERT_THROWS_NOTHING(level4.updateGames(afl::data::IntegerList_t()));
    }

    // resetToTurn
    mock.expectCall("resetToTurn(22,12)");
    TS_ASSERT_THROWS_NOTHING(level4.resetToTurn(22,12));

    mock.checkFinish();
}
