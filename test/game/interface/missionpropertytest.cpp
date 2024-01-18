/**
  *  \file test/game/interface/missionpropertytest.cpp
  *  \brief Test for game::interface::MissionProperty
  */

#include "game/interface/missionproperty.hpp"

#include "afl/test/testrunner.hpp"
#include "interpreter/test/valueverifier.hpp"

using game::spec::Mission;
using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

/** General test. */
AFL_TEST("game.interface.MissionProperty:full", a)
{
    Mission msn(30, ",The Name");
    msn.setRaceMask(game::PlayerSet_t(5));
    msn.setFlags(Mission::FlagSet_t() + Mission::RegisteredMission);
    msn.setShortName("Shorter");
    msn.setHotkey('q');
    msn.setParameterType(game::InterceptParameter, Mission::IntegerParameter);
    msn.setParameterFlags(game::InterceptParameter, Mission::ParameterFlagSet_t());
    msn.setParameterName(game::InterceptParameter, "The Number");
    msn.setParameterType(game::TowParameter, Mission::ShipParameter);
    msn.setParameterFlags(game::TowParameter, Mission::ParameterFlagSet_t() + Mission::NotThisParameter);
    msn.setParameterName(game::TowParameter, "The Ship");
    msn.setConditionExpression("Expr()");
    msn.setWarningExpression("Warn()");
    msn.setLabelExpression("Label()");
    msn.setSetCommand("Call It");

    // Verify
    verifyNewString (a("impName"),                getMissionProperty(msn, game::interface::impName),                "The Name");
    verifyNewInteger(a("impNumber"),              getMissionProperty(msn, game::interface::impNumber),              30);
    verifyNewInteger(a("impRaces"),               getMissionProperty(msn, game::interface::impRaces),               1<<5);
    verifyNewString (a("impFlags"),               getMissionProperty(msn, game::interface::impFlags),               "r");
    verifyNewString (a("impShortName"),           getMissionProperty(msn, game::interface::impShortName),           "Shorter");
    verifyNewString (a("impInterceptType"),       getMissionProperty(msn, game::interface::impInterceptType),       "n");
    verifyNewString (a("impInterceptFlags"),      getMissionProperty(msn, game::interface::impInterceptFlags),      "");
    verifyNewString (a("impInterceptName"),       getMissionProperty(msn, game::interface::impInterceptName),       "The Number");
    verifyNewString (a("impTowType"),             getMissionProperty(msn, game::interface::impTowType),             "s");
    verifyNewString (a("impTowFlags"),            getMissionProperty(msn, game::interface::impTowFlags),            "!");
    verifyNewString (a("impTowName"),             getMissionProperty(msn, game::interface::impTowName),             "The Ship");
    verifyNewString (a("impConditionExpression"), getMissionProperty(msn, game::interface::impConditionExpression), "Expr()");
    verifyNewString (a("impWarningExpression"),   getMissionProperty(msn, game::interface::impWarningExpression),   "Warn()");
    verifyNewString (a("impLabelExpression"),     getMissionProperty(msn, game::interface::impLabelExpression),     "Label()");
    verifyNewString (a("impSetCommand"),          getMissionProperty(msn, game::interface::impSetCommand),          "Call It");
    verifyNewString (a("impHotkey"),              getMissionProperty(msn, game::interface::impHotkey),              "q");
}

/** Test all the flag mappings. */
// Flags
AFL_TEST("game.interface.MissionProperty:flag:none", a)
{
    Mission msn(30, ",");
    msn.setFlags(Mission::FlagSet_t());
    verifyNewString(a, getMissionProperty(msn, game::interface::impFlags), "");
}

AFL_TEST("game.interface.MissionProperty:flag:WaypointMission", a)
{
    Mission msn(30, ",");
    msn.setFlags(Mission::FlagSet_t() + Mission::WaypointMission);
    verifyNewString(a, getMissionProperty(msn, game::interface::impFlags), "i");
}

AFL_TEST("game.interface.MissionProperty:flag:RegisteredMission", a)
{
    Mission msn(30, ",");
    msn.setFlags(Mission::FlagSet_t() + Mission::RegisteredMission);
    verifyNewString(a, getMissionProperty(msn, game::interface::impFlags), "r");
}

// Parameter flags
AFL_TEST("game.interface.MissionProperty:param:NotThisParameter", a)
{
    Mission msn(30, ",");
    msn.setParameterFlags(game::TowParameter, Mission::ParameterFlagSet_t() + Mission::NotThisParameter);
    verifyNewString(a, getMissionProperty(msn, game::interface::impTowFlags), "!");
}

AFL_TEST("game.interface.MissionProperty:param:OwnParameter", a)
{
    Mission msn(30, ",");
    msn.setParameterFlags(game::TowParameter, Mission::ParameterFlagSet_t() + Mission::OwnParameter);
    verifyNewString(a, getMissionProperty(msn, game::interface::impTowFlags), "o");
}

// Parameter types
AFL_TEST("game.interface.MissionProperty:type:none", a)
{
    Mission msn(30, ",");
    verifyNewNull(a, getMissionProperty(msn, game::interface::impTowType));
}

AFL_TEST("game.interface.MissionProperty:type:IntegerParameter", a)
{
    Mission msn(30, ",");
    msn.setParameterType(game::TowParameter, Mission::IntegerParameter);
    verifyNewString(a, getMissionProperty(msn, game::interface::impTowType), "n");
}

AFL_TEST("game.interface.MissionProperty:type:PlanetParameter", a)
{
    Mission msn(30, ",");
    msn.setParameterType(game::TowParameter, Mission::PlanetParameter);
    verifyNewString(a, getMissionProperty(msn, game::interface::impTowType), "p");
}

AFL_TEST("game.interface.MissionProperty:type:ShipParameter", a)
{
    Mission msn(30, ",");
    msn.setParameterType(game::TowParameter, Mission::ShipParameter);
    verifyNewString(a, getMissionProperty(msn, game::interface::impTowType), "s");
}

AFL_TEST("game.interface.MissionProperty:type:HereParameter", a)
{
    Mission msn(30, ",");
    msn.setParameterType(game::TowParameter, Mission::HereParameter);
    verifyNewString(a, getMissionProperty(msn, game::interface::impTowType), "h");
}

AFL_TEST("game.interface.MissionProperty:type:PlayerParameter", a)
{
    Mission msn(30, ",");
    msn.setParameterType(game::TowParameter, Mission::PlayerParameter);
    verifyNewString(a, getMissionProperty(msn, game::interface::impTowType), "y");
}

AFL_TEST("game.interface.MissionProperty:type:BaseParameter", a)
{
    Mission msn(30, ",");
    msn.setParameterType(game::TowParameter, Mission::BaseParameter);
    verifyNewString(a, getMissionProperty(msn, game::interface::impTowType), "b");
}

// Empty values
AFL_TEST("game.interface.MissionProperty:empty", a)
{
    Mission msn(30, ",");
    verifyNewNull(a("null intercept name"), getMissionProperty(msn, game::interface::impInterceptName));
    verifyNewNull(a("null tow name"),       getMissionProperty(msn, game::interface::impTowName));
    verifyNewNull(a("null hotkey"),         getMissionProperty(msn, game::interface::impHotkey));
}
