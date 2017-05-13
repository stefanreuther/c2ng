/**
  *  \file u/t_game_spec_mission.cpp
  *  \brief Test for game::spec::Mission
  */

#include "game/spec/mission.hpp"

#include "t_game_spec.hpp"

/** Test setters/getters. */
void
TestGameSpecMission::testData()
{
    using game::spec::Mission;

    // Verify default state
    Mission testee(42, "7,Bistromathic");
    TS_ASSERT_EQUALS(testee.getNumber(), 42);
    TS_ASSERT(testee.getRaceMask().contains(7));
    TS_ASSERT(testee.getFlags().empty());
    TS_ASSERT_EQUALS(testee.getName(), "Bistromathic");
    TS_ASSERT_EQUALS(testee.getShortName(), "Bistrom");
    TS_ASSERT_EQUALS(testee.getHotkey(), '\0');

    TS_ASSERT_EQUALS(testee.getParameterType(game::InterceptParameter), Mission::NoParameter);
    TS_ASSERT_EQUALS(testee.getParameterType(game::TowParameter), Mission::NoParameter);
    TS_ASSERT(testee.getParameterFlags(game::InterceptParameter).empty());
    TS_ASSERT(testee.getParameterFlags(game::TowParameter).empty());
    TS_ASSERT_EQUALS(testee.getParameterName(game::InterceptParameter), "Intercept");
    TS_ASSERT_EQUALS(testee.getParameterName(game::TowParameter), "Tow");

    TS_ASSERT_EQUALS(testee.getConditionExpression(), "");
    TS_ASSERT_EQUALS(testee.getWarningExpression(), "");
    TS_ASSERT_EQUALS(testee.getLabelExpression(), "");
    TS_ASSERT_EQUALS(testee.getSetCommand(), "");

    // Set everything
    // Note: we cannot change the number!
    testee.setRaceMask(game::PlayerSet_t(3));
    testee.setFlags(Mission::FlagSet_t(Mission::RegisteredMission));
    testee.setName("Big Whoop");
    testee.setShortName("bg whp");
    testee.setHotkey('w');
    testee.setParameterType(game::InterceptParameter, Mission::PlanetParameter);
    testee.setParameterType(game::TowParameter, Mission::HereParameter);
    testee.setParameterFlags(game::InterceptParameter, Mission::ParameterFlagSet_t(Mission::OwnParameter));
    testee.setParameterFlags(game::TowParameter, Mission::ParameterFlagSet_t(Mission::NotThisParameter));
    testee.setParameterName(game::InterceptParameter, "own planet");
    testee.setParameterName(game::TowParameter, "other ship here");
    testee.setConditionExpression("cond?");
    testee.setWarningExpression("warn?");
    testee.setLabelExpression("label?");
    testee.setSetCommand("set!");

    // Verify
    TS_ASSERT_EQUALS(testee.getNumber(), 42);
    TS_ASSERT(!testee.getRaceMask().contains(7));
    TS_ASSERT(testee.getRaceMask().contains(3));
    TS_ASSERT(testee.getFlags() == Mission::FlagSet_t(Mission::RegisteredMission));
    TS_ASSERT_EQUALS(testee.getName(), "Big Whoop");
    TS_ASSERT_EQUALS(testee.getShortName(), "bg whp");
    TS_ASSERT_EQUALS(testee.getHotkey(), 'w');

    TS_ASSERT_EQUALS(testee.getParameterType(game::InterceptParameter), Mission::PlanetParameter);
    TS_ASSERT_EQUALS(testee.getParameterType(game::TowParameter), Mission::HereParameter);
    TS_ASSERT(testee.getParameterFlags(game::InterceptParameter).contains(Mission::OwnParameter));
    TS_ASSERT(testee.getParameterFlags(game::TowParameter).contains(Mission::NotThisParameter));
    TS_ASSERT_EQUALS(testee.getParameterName(game::InterceptParameter), "own planet");
    TS_ASSERT_EQUALS(testee.getParameterName(game::TowParameter), "other ship here");

    TS_ASSERT_EQUALS(testee.getConditionExpression(), "cond?");
    TS_ASSERT_EQUALS(testee.getWarningExpression(), "warn?");
    TS_ASSERT_EQUALS(testee.getLabelExpression(), "label?");
    TS_ASSERT_EQUALS(testee.getSetCommand(), "set!");
}

/** Test constructor. */
void
TestGameSpecMission::testConstruct()
{
    using game::spec::Mission;

    // Name and hotkey
    TS_ASSERT_EQUALS(Mission(42, ",hi mom").getName(), "hi mom");
    TS_ASSERT_EQUALS(Mission(42, ",~hi mom").getHotkey(), 'h');
    TS_ASSERT_EQUALS(Mission(42, ",hi ~Mom").getHotkey(), 'm');
    TS_ASSERT_EQUALS(Mission(42, "this is mostly ignored,hi mom").getName(), "hi mom");

    // Races
    {
        Mission m(42, "-7,hi mom");
        TS_ASSERT(m.getRaceMask().contains(1));
        TS_ASSERT(!m.getRaceMask().contains(7));
    }
    {
        Mission m(42, "+7,hi mom");
        TS_ASSERT(!m.getRaceMask().contains(1));
        TS_ASSERT(m.getRaceMask().contains(7));
    }

    // Flags
    {
        Mission m(42, "r,hi mom");
        TS_ASSERT(m.getFlags().contains(Mission::RegisteredMission));
        TS_ASSERT(m.hasFlag(Mission::RegisteredMission));
        TS_ASSERT(!m.getFlags().contains(Mission::WaypointMission));
        TS_ASSERT(!m.hasFlag(Mission::WaypointMission));
    }
    {
        Mission m(42, "i,hi mom");
        TS_ASSERT(!m.getFlags().contains(Mission::RegisteredMission));
        TS_ASSERT(!m.hasFlag(Mission::RegisteredMission));
        TS_ASSERT(m.getFlags().contains(Mission::WaypointMission));
        TS_ASSERT(m.hasFlag(Mission::WaypointMission));
    }
    {
        Mission m(42, "ri,hi mom");
        TS_ASSERT(m.getFlags().contains(Mission::RegisteredMission));
        TS_ASSERT(m.getFlags().contains(Mission::WaypointMission));
    }

    // Parameter assignment
    {
        Mission m(42, "n#,hi mom");
        TS_ASSERT_EQUALS(m.getParameterType(game::InterceptParameter), Mission::NoParameter);
        TS_ASSERT_EQUALS(m.getParameterType(game::TowParameter), Mission::IntegerParameter);
    }
    {
        Mission m(42, "n*,hi mom");
        TS_ASSERT_EQUALS(m.getParameterType(game::InterceptParameter), Mission::IntegerParameter);
        TS_ASSERT_EQUALS(m.getParameterType(game::TowParameter), Mission::NoParameter);
    }
    {
        Mission m(42, "n*#,hi mom");
        TS_ASSERT_EQUALS(m.getParameterType(game::InterceptParameter), Mission::IntegerParameter);
        TS_ASSERT_EQUALS(m.getParameterType(game::TowParameter), Mission::IntegerParameter);
    }

    // Other parameter types
    TS_ASSERT_EQUALS(Mission(42,"p#,hi mom").getParameterType(game::TowParameter), Mission::PlanetParameter);
    TS_ASSERT_EQUALS(Mission(42,"s#,hi mom").getParameterType(game::TowParameter), Mission::ShipParameter);
    TS_ASSERT_EQUALS(Mission(42,"h#,hi mom").getParameterType(game::TowParameter), Mission::HereParameter);
    TS_ASSERT_EQUALS(Mission(42,"b#,hi mom").getParameterType(game::TowParameter), Mission::BaseParameter);
    TS_ASSERT_EQUALS(Mission(42,"y#,hi mom").getParameterType(game::TowParameter), Mission::PlayerParameter);

    // Parameter flags
    {
        Mission m(42, "os#,hi mom");
        TS_ASSERT(m.getParameterFlags(game::TowParameter).contains(Mission::OwnParameter));
        TS_ASSERT_EQUALS(m.getParameterType(game::TowParameter), Mission::ShipParameter);
    }
    {
        Mission m(42, "!s#,hi mom");
        TS_ASSERT(m.getParameterFlags(game::TowParameter).contains(Mission::NotThisParameter));
        TS_ASSERT_EQUALS(m.getParameterType(game::TowParameter), Mission::ShipParameter);
    }
    {
        Mission m(42, "s*!#,hi mom");
        TS_ASSERT(m.getParameterFlags(game::InterceptParameter).empty());
        TS_ASSERT_EQUALS(m.getParameterType(game::InterceptParameter), Mission::ShipParameter);
        TS_ASSERT(m.getParameterFlags(game::TowParameter).contains(Mission::NotThisParameter));
        TS_ASSERT_EQUALS(m.getParameterType(game::TowParameter), Mission::ShipParameter);
    }
}

/** Test default constructor. */
void
TestGameSpecMission::testDefault()
{
    // The default constructor is not normally used.
    game::spec::Mission testee;
    TS_ASSERT_EQUALS(testee.getNumber(), 0);
    TS_ASSERT(testee.getRaceMask().empty());
    TS_ASSERT(testee.getFlags().empty());
    TS_ASSERT_EQUALS(testee.getName(), "");
    TS_ASSERT_EQUALS(testee.getShortName(), "");
}

