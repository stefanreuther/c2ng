/**
  *  \file test/server/interface/hostgametest.cpp
  *  \brief Test for server::interface::HostGame
  */

#include "server/interface/hostgame.hpp"
#include "afl/test/testrunner.hpp"

using server::interface::HostGame;

/** Interface test. */
AFL_TEST_NOARG("server.interface.HostGame:interface")
{
    class Tester : public HostGame {
     public:
        virtual int32_t createNewGame()
            { return 0; }
        virtual int32_t cloneGame(int32_t /*gameId*/, afl::base::Optional<State> /*newState*/)
            { return 0; }
        virtual void setType(int32_t /*gameId*/, Type /*type*/)
            { }
        virtual void setState(int32_t /*gameId*/, State /*state*/)
            { }
        virtual void setOwner(int32_t /*gameId*/, String_t /*user*/)
            { }
        virtual void setName(int32_t /*gameId*/, String_t /*name*/)
            { }
        virtual Info getInfo(int32_t /*gameId*/)
            { return Info(); }
        virtual void getInfos(const Filter& /*filter*/, bool /*verbose*/, std::vector<Info>& /*result*/)
            { }
        virtual void getGames(const Filter& /*filter*/, std::vector<int32_t>& /*result*/)
            { }
        virtual void setConfig(int32_t /*gameId*/, const afl::data::StringList_t& /*keyValues*/)
            { }
        virtual String_t getConfig(int32_t /*gameId*/, String_t /*key*/)
            { return String_t(); }
        virtual void getConfig(int32_t /*gameId*/, const afl::data::StringList_t& /*keys*/, afl::data::StringList_t& /*values*/)
            { }
        virtual String_t getComputedValue(int32_t /*gameId*/, String_t /*key*/)
            { return String_t(); }
        virtual State getState(int32_t /*gameId*/)
            { return State(); }
        virtual Type getType(int32_t /*gameId*/)
            { return Type(); }
        virtual String_t getOwner(int32_t /*gameId*/)
            { return String_t(); }
        virtual String_t getName(int32_t /*gameId*/)
            { return String_t(); }
        virtual String_t getDirectory(int32_t /*gameId*/)
            { return String_t(); }
        virtual afl::bits::SmallSet<Permission> getPermissions(int32_t /*gameId*/, String_t /*userId*/)
            { return afl::bits::SmallSet<Permission>(); }
        virtual bool addTool(int32_t /*gameId*/, String_t /*toolId*/)
            { return false; }
        virtual bool removeTool(int32_t /*gameId*/, String_t /*toolId*/)
            { return false; }
        virtual void getTools(int32_t /*gameId*/, std::vector<server::interface::HostTool::Info>& /*result*/)
            { }
        virtual Totals getTotals()
            { return Totals(); }
        virtual VictoryCondition getVictoryCondition(int32_t /*gameId*/)
            { return VictoryCondition(); }
        virtual void updateGames(const afl::data::IntegerList_t& /*gameIds*/)
            { }
        virtual void resetToTurn(int32_t /*gameId*/, int /*turnNr*/)
            { }
    };
    Tester t;
}

/** Test "format" functions. */
AFL_TEST("server.interface.HostGame:formatType", a)
{
    a.checkEqual("01", HostGame::formatType(HostGame::PublicGame),   "public");
    a.checkEqual("02", HostGame::formatType(HostGame::PrivateGame),  "private");
    a.checkEqual("03", HostGame::formatType(HostGame::UnlistedGame), "unlisted");
    a.checkEqual("04", HostGame::formatType(HostGame::TestGame),     "test");
}

AFL_TEST("server.interface.HostGame:formatState", a)
{
    a.checkEqual("01", HostGame::formatState(HostGame::Preparing), "preparing");
    a.checkEqual("02", HostGame::formatState(HostGame::Joining),   "joining");
    a.checkEqual("03", HostGame::formatState(HostGame::Running),   "running");
    a.checkEqual("04", HostGame::formatState(HostGame::Finished),  "finished");
    a.checkEqual("05", HostGame::formatState(HostGame::Deleted),   "deleted");
}

AFL_TEST("server.interface.HostGame:formatSlotState", a)
{
    a.checkEqual("01", HostGame::formatSlotState(HostGame::OpenSlot), "open");
    a.checkEqual("02", HostGame::formatSlotState(HostGame::DeadSlot), "dead");
    a.checkEqual("03", HostGame::formatSlotState(HostGame::SelfSlot), "self");
    a.checkEqual("04", HostGame::formatSlotState(HostGame::OccupiedSlot), "occupied");
}

/** Test "parse" functions. */
AFL_TEST("server.interface.HostGame:parseType", a)
{
    HostGame::Type type;
    a.check("01", HostGame::parseType("public", type));
    a.checkEqual("02", type, HostGame::PublicGame);
    a.check("03", HostGame::parseType("private", type));
    a.checkEqual("04", type, HostGame::PrivateGame);
    a.check("05", HostGame::parseType("unlisted", type));
    a.checkEqual("06", type, HostGame::UnlistedGame);
    a.check("07", HostGame::parseType("test", type));
    a.checkEqual("08", type, HostGame::TestGame);

    a.check("11", !HostGame::parseType("TEST", type));
    a.check("12", !HostGame::parseType("preparing", type));
    a.check("13", !HostGame::parseType("pu", type));
    a.check("14", !HostGame::parseType("", type));
}

AFL_TEST("server.interface.HostGame:parseState", a)
{
    HostGame::State state;
    a.check("21", HostGame::parseState("preparing", state));
    a.checkEqual("22", state, HostGame::Preparing);
    a.check("23", HostGame::parseState("joining", state));
    a.checkEqual("24", state, HostGame::Joining);
    a.check("25", HostGame::parseState("running", state));
    a.checkEqual("26", state, HostGame::Running);
    a.check("27", HostGame::parseState("finished", state));
    a.checkEqual("28", state, HostGame::Finished);
    a.check("29", HostGame::parseState("deleted", state));
    a.checkEqual("30", state, HostGame::Deleted);

    a.check("31", !HostGame::parseState("JOINING", state));
    a.check("32", !HostGame::parseState("join", state));
    a.check("33", !HostGame::parseState("public", state));
    a.check("34", !HostGame::parseState("", state));
}

AFL_TEST("server.interface.HostGame:parseSlotState", a)
{
    HostGame::SlotState ss;
    a.check("41", HostGame::parseSlotState("open", ss));
    a.checkEqual("42", ss, HostGame::OpenSlot);
    a.check("43", HostGame::parseSlotState("dead", ss));
    a.checkEqual("44", ss, HostGame::DeadSlot);
    a.check("45", HostGame::parseSlotState("self", ss));
    a.checkEqual("46", ss, HostGame::SelfSlot);
    a.check("47", HostGame::parseSlotState("occupied", ss));
    a.checkEqual("48", ss, HostGame::OccupiedSlot);

    a.check("51", !HostGame::parseSlotState("OPEN", ss));
    a.check("52", !HostGame::parseSlotState("op", ss));
    a.check("53", !HostGame::parseSlotState("", ss));
    a.check("54", !HostGame::parseSlotState("foo", ss));
}

/** Interface test. */
AFL_TEST("server.interface.HostGame:init:Totals", a)
{
    HostGame::Totals t;
    a.checkEqual("01", t.numJoiningGames, 0);
    a.checkEqual("02", t.numRunningGames, 0);
    a.checkEqual("03", t.numFinishedGames, 0);
}

AFL_TEST("server.interface.HostGame:init:Info", a)
{
    HostGame::Info i;
    a.checkEqual("01", i.gameId, 0);
    a.checkEqual("02", i.state, HostGame::Preparing);
    a.checkEqual("03", i.type, HostGame::PrivateGame);
    a.checkEqual("04", i.name, "");
    a.check     ("05", !i.description.isValid());
    a.checkEqual("06", i.difficulty, 0);
    a.check     ("07", !i.currentSchedule.isValid());
    a.check     ("08", !i.slotStates.isValid());
    a.check     ("09", !i.turnStates.isValid());
    a.check     ("10", !i.joinable.isValid());
    a.check     ("11", !i.userPlays.isValid());
    a.check     ("12", !i.scores.isValid());
    a.check     ("13", !i.scoreName.isValid());
    a.check     ("14", !i.scoreDescription.isValid());
    a.checkEqual("15", i.hostName, "");
    a.checkEqual("16", i.hostDescription, "");
    a.checkEqual("17", i.hostKind, "");
    a.checkEqual("18", i.shipListName, "");
    a.checkEqual("19", i.shipListDescription, "");
    a.checkEqual("20", i.shipListKind, "");
    a.check     ("21", !i.masterName.isValid());
    a.check     ("22", !i.masterDescription.isValid());
    a.checkEqual("23", i.turnNumber, 0);
    a.check     ("24", !i.lastHostTime.isValid());
    a.check     ("25", !i.nextHostTime.isValid());
    a.check     ("25", !i.forumId.isValid());
    a.check     ("26", !i.userRank.isValid());
    a.check     ("27", !i.otherRank.isValid());
}

AFL_TEST("server.interface.HostGame:init:VictoryCondition", a)
{
    HostGame::VictoryCondition v;
    a.checkEqual("01. endCondition", v.endCondition, "");
    a.check     ("02", !v.endTurn.isValid());
    a.check     ("03", !v.endProbability.isValid());
    a.check     ("04", !v.endScore.isValid());
    a.check     ("05", !v.endScoreName.isValid());
    a.check     ("06", !v.endScoreDescription.isValid());
    a.check     ("07", !v.referee.isValid());
    a.check     ("08", !v.refereeDescription.isValid());
}
