/**
  *  \file u/t_server_interface_hostgame.cpp
  *  \brief Test for server::interface::HostGame
  */

#include "server/interface/hostgame.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceHostGame::testInterface()
{
    class Tester : public server::interface::HostGame {
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
    };
    Tester t;
}

/** Test "format" functions. */
void
TestServerInterfaceHostGame::testFormat()
{
    using server::interface::HostGame;

    TS_ASSERT_EQUALS(HostGame::formatType(HostGame::PublicGame),   "public");
    TS_ASSERT_EQUALS(HostGame::formatType(HostGame::PrivateGame),  "private");
    TS_ASSERT_EQUALS(HostGame::formatType(HostGame::UnlistedGame), "unlisted");
    TS_ASSERT_EQUALS(HostGame::formatType(HostGame::TestGame),     "test");

    TS_ASSERT_EQUALS(HostGame::formatState(HostGame::Preparing), "preparing");
    TS_ASSERT_EQUALS(HostGame::formatState(HostGame::Joining),   "joining");
    TS_ASSERT_EQUALS(HostGame::formatState(HostGame::Running),   "running");
    TS_ASSERT_EQUALS(HostGame::formatState(HostGame::Finished),  "finished");
    TS_ASSERT_EQUALS(HostGame::formatState(HostGame::Deleted),   "deleted");

    TS_ASSERT_EQUALS(HostGame::formatSlotState(HostGame::OpenSlot), "open");
    TS_ASSERT_EQUALS(HostGame::formatSlotState(HostGame::DeadSlot), "dead");
    TS_ASSERT_EQUALS(HostGame::formatSlotState(HostGame::SelfSlot), "self");
    TS_ASSERT_EQUALS(HostGame::formatSlotState(HostGame::OccupiedSlot), "occupied");
}

/** Test "parse" functions. */
void
TestServerInterfaceHostGame::testParse()
{
    using server::interface::HostGame;

    HostGame::Type type;
    TS_ASSERT(HostGame::parseType("public", type));
    TS_ASSERT_EQUALS(type, HostGame::PublicGame);
    TS_ASSERT(HostGame::parseType("private", type));
    TS_ASSERT_EQUALS(type, HostGame::PrivateGame);
    TS_ASSERT(HostGame::parseType("unlisted", type));
    TS_ASSERT_EQUALS(type, HostGame::UnlistedGame);
    TS_ASSERT(HostGame::parseType("test", type));
    TS_ASSERT_EQUALS(type, HostGame::TestGame);

    TS_ASSERT(!HostGame::parseType("TEST", type));
    TS_ASSERT(!HostGame::parseType("preparing", type));
    TS_ASSERT(!HostGame::parseType("pu", type));
    TS_ASSERT(!HostGame::parseType("", type));

    HostGame::State state;
    TS_ASSERT(HostGame::parseState("preparing", state));
    TS_ASSERT_EQUALS(state, HostGame::Preparing);
    TS_ASSERT(HostGame::parseState("joining", state));
    TS_ASSERT_EQUALS(state, HostGame::Joining);
    TS_ASSERT(HostGame::parseState("running", state));
    TS_ASSERT_EQUALS(state, HostGame::Running);
    TS_ASSERT(HostGame::parseState("finished", state));
    TS_ASSERT_EQUALS(state, HostGame::Finished);
    TS_ASSERT(HostGame::parseState("deleted", state));
    TS_ASSERT_EQUALS(state, HostGame::Deleted);

    TS_ASSERT(!HostGame::parseState("JOINING", state));
    TS_ASSERT(!HostGame::parseState("join", state));
    TS_ASSERT(!HostGame::parseState("public", state));
    TS_ASSERT(!HostGame::parseState("", state));

    HostGame::SlotState ss;
    TS_ASSERT(HostGame::parseSlotState("open", ss));
    TS_ASSERT_EQUALS(ss, HostGame::OpenSlot);
    TS_ASSERT(HostGame::parseSlotState("dead", ss));
    TS_ASSERT_EQUALS(ss, HostGame::DeadSlot);
    TS_ASSERT(HostGame::parseSlotState("self", ss));
    TS_ASSERT_EQUALS(ss, HostGame::SelfSlot);
    TS_ASSERT(HostGame::parseSlotState("occupied", ss));
    TS_ASSERT_EQUALS(ss, HostGame::OccupiedSlot);

    TS_ASSERT(!HostGame::parseSlotState("OPEN", ss));
    TS_ASSERT(!HostGame::parseSlotState("op", ss));
    TS_ASSERT(!HostGame::parseSlotState("", ss));
    TS_ASSERT(!HostGame::parseSlotState("foo", ss));
}

/** Interface test. */
void
TestServerInterfaceHostGame::testInit()
{
    using server::interface::HostGame;
    {
        HostGame::Totals t;
        TS_ASSERT_EQUALS(t.numJoiningGames, 0);
        TS_ASSERT_EQUALS(t.numRunningGames, 0);
        TS_ASSERT_EQUALS(t.numFinishedGames, 0);
    }
    {
        HostGame::Info i;
        TS_ASSERT_EQUALS(i.gameId, 0);
        TS_ASSERT_EQUALS(i.state, HostGame::Preparing);
        TS_ASSERT_EQUALS(i.type, HostGame::PrivateGame);
        TS_ASSERT_EQUALS(i.name, "");
        TS_ASSERT(!i.description.isValid());
        TS_ASSERT_EQUALS(i.difficulty, 0);
        TS_ASSERT(!i.currentSchedule.isValid());
        TS_ASSERT(!i.slotStates.isValid());
        TS_ASSERT(!i.turnStates.isValid());
        TS_ASSERT(!i.joinable.isValid());
        TS_ASSERT(!i.scores.isValid());
        TS_ASSERT(!i.scoreName.isValid());
        TS_ASSERT(!i.scoreDescription.isValid());
        TS_ASSERT_EQUALS(i.hostName, "");
        TS_ASSERT_EQUALS(i.hostDescription, "");
        TS_ASSERT_EQUALS(i.shipListName, "");
        TS_ASSERT_EQUALS(i.shipListDescription, "");
        TS_ASSERT(!i.masterName.isValid());
        TS_ASSERT(!i.masterDescription.isValid());
        TS_ASSERT_EQUALS(i.turnNumber, 0);
        TS_ASSERT(!i.lastHostTime.isValid());
        TS_ASSERT(!i.nextHostTime.isValid());
        TS_ASSERT(!i.forumId.isValid());
    }
    {
        HostGame::VictoryCondition v;
        TS_ASSERT_EQUALS(v.endCondition, "");
        TS_ASSERT(!v.endTurn.isValid());
        TS_ASSERT(!v.endProbability.isValid());
        TS_ASSERT(!v.endScore.isValid());
        TS_ASSERT(!v.endScoreName.isValid());
        TS_ASSERT(!v.endScoreDescription.isValid());
        TS_ASSERT(!v.referee.isValid());
        TS_ASSERT(!v.refereeDescription.isValid());
    }
}

