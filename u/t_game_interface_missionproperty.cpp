/**
  *  \file u/t_game_interface_missionproperty.cpp
  *  \brief Test for game::interface::MissionProperty
  */

#include "game/interface/missionproperty.hpp"

#include "t_game_interface.hpp"
#include "interpreter/test/valueverifier.hpp"

using game::spec::Mission;
using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

/** General test. */
void
TestGameInterfaceMissionProperty::testIt()
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
    verifyNewString ("impName",                getMissionProperty(msn, game::interface::impName),                "The Name");
    verifyNewInteger("impNumber",              getMissionProperty(msn, game::interface::impNumber),              30);
    verifyNewInteger("impRaces",               getMissionProperty(msn, game::interface::impRaces),               1<<5);
    verifyNewString ("impFlags",               getMissionProperty(msn, game::interface::impFlags),               "r");
    verifyNewString ("impShortName",           getMissionProperty(msn, game::interface::impShortName),           "Shorter");
    verifyNewString ("impInterceptType",       getMissionProperty(msn, game::interface::impInterceptType),       "n");
    verifyNewString ("impInterceptFlags",      getMissionProperty(msn, game::interface::impInterceptFlags),      "");
    verifyNewString ("impInterceptName",       getMissionProperty(msn, game::interface::impInterceptName),       "The Number");
    verifyNewString ("impTowType",             getMissionProperty(msn, game::interface::impTowType),             "s");
    verifyNewString ("impTowFlags",            getMissionProperty(msn, game::interface::impTowFlags),            "!");
    verifyNewString ("impTowName",             getMissionProperty(msn, game::interface::impTowName),             "The Ship");
    verifyNewString ("impConditionExpression", getMissionProperty(msn, game::interface::impConditionExpression), "Expr()");
    verifyNewString ("impWarningExpression",   getMissionProperty(msn, game::interface::impWarningExpression),   "Warn()");
    verifyNewString ("impLabelExpression",     getMissionProperty(msn, game::interface::impLabelExpression),     "Label()");
    verifyNewString ("impSetCommand",          getMissionProperty(msn, game::interface::impSetCommand),          "Call It");
    verifyNewString ("impHotkey",              getMissionProperty(msn, game::interface::impHotkey),              "q");
}

/** Test all the flag mappings. */
void
TestGameInterfaceMissionProperty::testIt2()
{
    // Flags
    {
        Mission msn(30, ",");
        msn.setFlags(Mission::FlagSet_t());
        verifyNewString("flag empty", getMissionProperty(msn, game::interface::impFlags), "");
    }
    {
        Mission msn(30, ",");
        msn.setFlags(Mission::FlagSet_t() + Mission::WaypointMission);
        verifyNewString("flag i", getMissionProperty(msn, game::interface::impFlags), "i");
    }
    {
        Mission msn(30, ",");
        msn.setFlags(Mission::FlagSet_t() + Mission::RegisteredMission);
        verifyNewString("flag r", getMissionProperty(msn, game::interface::impFlags), "r");
    }

    // Parameter flags
    {
        Mission msn(30, ",");
        msn.setParameterFlags(game::TowParameter, Mission::ParameterFlagSet_t() + Mission::NotThisParameter);
        verifyNewString("param flag !", getMissionProperty(msn, game::interface::impTowFlags), "!");
    }
    {
        Mission msn(30, ",");
        msn.setParameterFlags(game::TowParameter, Mission::ParameterFlagSet_t() + Mission::OwnParameter);
        verifyNewString("param flag o", getMissionProperty(msn, game::interface::impTowFlags), "o");
    }

    // Parameter types
    {
        Mission msn(30, ",");
        verifyNewNull("param type null", getMissionProperty(msn, game::interface::impTowType));
    }
    {
        Mission msn(30, ",");
        msn.setParameterType(game::TowParameter, Mission::IntegerParameter);
        verifyNewString("param type int", getMissionProperty(msn, game::interface::impTowType), "n");
    }
    {
        Mission msn(30, ",");
        msn.setParameterType(game::TowParameter, Mission::PlanetParameter);
        verifyNewString("param type int", getMissionProperty(msn, game::interface::impTowType), "p");
    }
    {
        Mission msn(30, ",");
        msn.setParameterType(game::TowParameter, Mission::ShipParameter);
        verifyNewString("param type int", getMissionProperty(msn, game::interface::impTowType), "s");
    }
    {
        Mission msn(30, ",");
        msn.setParameterType(game::TowParameter, Mission::HereParameter);
        verifyNewString("param type int", getMissionProperty(msn, game::interface::impTowType), "h");
    }
    {
        Mission msn(30, ",");
        msn.setParameterType(game::TowParameter, Mission::PlayerParameter);
        verifyNewString("param type int", getMissionProperty(msn, game::interface::impTowType), "y");
    }
    {
        Mission msn(30, ",");
        msn.setParameterType(game::TowParameter, Mission::BaseParameter);
        verifyNewString("param type int", getMissionProperty(msn, game::interface::impTowType), "b");
    }

    // Empty values
    {
        Mission msn(30, ",");
        verifyNewNull("null intercept name", getMissionProperty(msn, game::interface::impInterceptName));
        verifyNewNull("null tow name",       getMissionProperty(msn, game::interface::impTowName));
        verifyNewNull("null hotkey",         getMissionProperty(msn, game::interface::impHotkey));
    }
}

