/**
  *  \file test/game/spec/missiontest.cpp
  *  \brief Test for game::spec::Mission
  */

#include "game/spec/mission.hpp"
#include "afl/test/testrunner.hpp"

/** Test setters/getters. */
AFL_TEST("game.spec.Mission:basics", a)
{
    using game::spec::Mission;

    // Verify default state
    Mission testee(42, "7,Bistromathic");
    a.checkEqual("01. getNumber",              testee.getNumber(), 42);
    a.check("02. getRaceMask",                 testee.getRaceMask().contains(7));
    a.check("03. getFlags",                    testee.getFlags().empty());
    a.checkEqual("04. getName",                testee.getName(), "Bistromathic");
    a.checkEqual("05. getShortName",           testee.getShortName(), "Bistrom");
    a.checkEqual("06. getHotkey",              testee.getHotkey(), '\0');

    a.checkEqual("11. getParameterType",       testee.getParameterType(game::InterceptParameter), Mission::NoParameter);
    a.checkEqual("12. getParameterType",       testee.getParameterType(game::TowParameter), Mission::NoParameter);
    a.check     ("13. getParameterFlags",      testee.getParameterFlags(game::InterceptParameter).empty());
    a.check     ("14. getParameterFlags",      testee.getParameterFlags(game::TowParameter).empty());
    a.checkEqual("15. getParameterName",       testee.getParameterName(game::InterceptParameter), "Intercept");
    a.checkEqual("16. getParameterName",       testee.getParameterName(game::TowParameter), "Tow");

    a.checkEqual("21. getConditionExpression", testee.getConditionExpression(), "");
    a.checkEqual("22. getWarningExpression",   testee.getWarningExpression(), "");
    a.checkEqual("23. getLabelExpression",     testee.getLabelExpression(), "");
    a.checkEqual("24. getSetCommand",          testee.getSetCommand(), "");

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
    a.checkEqual("31. getNumber",              testee.getNumber(), 42);
    a.check("32. getRaceMask",                !testee.getRaceMask().contains(7));
    a.check("33. getRaceMask",                 testee.getRaceMask().contains(3));
    a.check("34. getFlags",                    testee.getFlags() == Mission::FlagSet_t(Mission::RegisteredMission));
    a.checkEqual("35. getName",                testee.getName(), "Big Whoop");
    a.checkEqual("36. getShortName",           testee.getShortName(), "bg whp");
    a.checkEqual("37. getHotkey",              testee.getHotkey(), 'w');

    a.checkEqual("41. getParameterType",       testee.getParameterType(game::InterceptParameter), Mission::PlanetParameter);
    a.checkEqual("42. getParameterType",       testee.getParameterType(game::TowParameter), Mission::HereParameter);
    a.check("43. getParameterFlags",           testee.getParameterFlags(game::InterceptParameter).contains(Mission::OwnParameter));
    a.check("44. getParameterFlags",           testee.getParameterFlags(game::TowParameter).contains(Mission::NotThisParameter));
    a.checkEqual("45. getParameterName",       testee.getParameterName(game::InterceptParameter), "own planet");
    a.checkEqual("46. getParameterName",       testee.getParameterName(game::TowParameter), "other ship here");

    a.checkEqual("51. getConditionExpression", testee.getConditionExpression(), "cond?");
    a.checkEqual("52. getWarningExpression",   testee.getWarningExpression(), "warn?");
    a.checkEqual("53. getLabelExpression",     testee.getLabelExpression(), "label?");
    a.checkEqual("54. getSetCommand",          testee.getSetCommand(), "set!");
}

/** Test constructor. */
AFL_TEST("game.spec.Mission:construct", a)
{
    using game::spec::Mission;

    // Name and hotkey
    a.checkEqual("01. getName",   Mission(42, ",hi mom").getName(), "hi mom");
    a.checkEqual("02. getHotkey", Mission(42, ",~hi mom").getHotkey(), 'h');
    a.checkEqual("03. getHotkey", Mission(42, ",hi ~Mom").getHotkey(), 'm');
    a.checkEqual("04. getName",   Mission(42, "this is mostly ignored,hi mom").getName(), "hi mom");

    // Races
    {
        Mission m(42, "-7,hi mom");
        a.check("11. getRaceMask", m.getRaceMask().contains(1));
        a.check("12. getRaceMask", !m.getRaceMask().contains(7));
    }
    {
        Mission m(42, "+7,hi mom");
        a.check("13. getRaceMask", !m.getRaceMask().contains(1));
        a.check("14. getRaceMask", m.getRaceMask().contains(7));
    }

    // Flags
    {
        Mission m(42, "r,hi mom");
        a.check("21. getFlags",  m.getFlags().contains(Mission::RegisteredMission));
        a.check("22. hasFlag",   m.hasFlag(Mission::RegisteredMission));
        a.check("23. getFlags", !m.getFlags().contains(Mission::WaypointMission));
        a.check("24. hasFlag",  !m.hasFlag(Mission::WaypointMission));
    }
    {
        Mission m(42, "i,hi mom");
        a.check("25. getFlags", !m.getFlags().contains(Mission::RegisteredMission));
        a.check("26. hasFlag",  !m.hasFlag(Mission::RegisteredMission));
        a.check("27. getFlags",  m.getFlags().contains(Mission::WaypointMission));
        a.check("28. hasFlag",   m.hasFlag(Mission::WaypointMission));
    }
    {
        Mission m(42, "ri,hi mom");
        a.check("29. getFlags", m.getFlags().contains(Mission::RegisteredMission));
        a.check("30. getFlags", m.getFlags().contains(Mission::WaypointMission));
    }

    // Parameter assignment
    {
        Mission m(42, "n#,hi mom");
        a.checkEqual("31. getParameterType", m.getParameterType(game::InterceptParameter), Mission::NoParameter);
        a.checkEqual("32. getParameterType", m.getParameterType(game::TowParameter), Mission::IntegerParameter);
    }
    {
        Mission m(42, "n*,hi mom");
        a.checkEqual("33. getParameterType", m.getParameterType(game::InterceptParameter), Mission::IntegerParameter);
        a.checkEqual("34. getParameterType", m.getParameterType(game::TowParameter), Mission::NoParameter);
    }
    {
        Mission m(42, "n*#,hi mom");
        a.checkEqual("35. getParameterType", m.getParameterType(game::InterceptParameter), Mission::IntegerParameter);
        a.checkEqual("36. getParameterType", m.getParameterType(game::TowParameter), Mission::IntegerParameter);
    }

    // Other parameter types
    a.checkEqual("41. getParameterType", Mission(42,"p#,hi mom").getParameterType(game::TowParameter), Mission::PlanetParameter);
    a.checkEqual("42. getParameterType", Mission(42,"s#,hi mom").getParameterType(game::TowParameter), Mission::ShipParameter);
    a.checkEqual("43. getParameterType", Mission(42,"h#,hi mom").getParameterType(game::TowParameter), Mission::HereParameter);
    a.checkEqual("44. getParameterType", Mission(42,"b#,hi mom").getParameterType(game::TowParameter), Mission::BaseParameter);
    a.checkEqual("45. getParameterType", Mission(42,"y#,hi mom").getParameterType(game::TowParameter), Mission::PlayerParameter);

    // Parameter flags
    {
        Mission m(42, "os#,hi mom");
        a.check     ("51. getParameterFlags", m.getParameterFlags(game::TowParameter).contains(Mission::OwnParameter));
        a.checkEqual("52. getParameterType",  m.getParameterType(game::TowParameter), Mission::ShipParameter);
    }
    {
        Mission m(42, "!s#,hi mom");
        a.check     ("53. getParameterFlags", m.getParameterFlags(game::TowParameter).contains(Mission::NotThisParameter));
        a.checkEqual("54. getParameterType",  m.getParameterType(game::TowParameter), Mission::ShipParameter);
    }
    {
        Mission m(42, "s*!#,hi mom");
        a.check     ("55. getParameterFlags", m.getParameterFlags(game::InterceptParameter).empty());
        a.checkEqual("56. getParameterType",  m.getParameterType(game::InterceptParameter), Mission::ShipParameter);
        a.check     ("57. getParameterFlags", m.getParameterFlags(game::TowParameter).contains(Mission::NotThisParameter));
        a.checkEqual("58. getParameterType",  m.getParameterType(game::TowParameter), Mission::ShipParameter);
    }
}

/** Test default constructor. */
AFL_TEST("game.spec.Mission:default", a)
{
    // The default constructor is not normally used.
    game::spec::Mission testee;
    a.checkEqual("01. getNumber",    testee.getNumber(), 0);
    a.check     ("02. getRaceMask",  testee.getRaceMask().empty());
    a.check     ("03. getFlags",     testee.getFlags().empty());
    a.checkEqual("04. getName",      testee.getName(), "");
    a.checkEqual("05. getShortName", testee.getShortName(), "");
}
