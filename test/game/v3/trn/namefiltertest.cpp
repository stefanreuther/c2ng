/**
  *  \file test/game/v3/trn/namefiltertest.cpp
  *  \brief Test for game::v3::trn::NameFilter
  */

#include "game/v3/trn/namefilter.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/test/testrunner.hpp"
#include "game/timestamp.hpp"
#include "game/v3/turnfile.hpp"

/** Simple test. */
AFL_TEST("game.v3.trn.NameFilter", a)
{
    // Make a turnfile
    afl::charset::Utf8Charset cs;
    game::v3::TurnFile trn(cs, 1, game::Timestamp());

    // Add commands. Give each command some dummy data.
    const uint8_t DUMMY[] = { 0, 0, 0, 0, 0, 0, };
    trn.addCommand(game::v3::tcm_ShipChangeSpeed,    9, DUMMY);   // 0
    trn.addCommand(game::v3::tcm_ShipChangeSpeed,   10, DUMMY);   // 1
    trn.addCommand(game::v3::tcm_PlanetChangeMines,  9, DUMMY);   // 2
    trn.addCommand(game::v3::tcm_PlanetChangeMines, 42, DUMMY);   // 3
    trn.addCommand(game::v3::tcm_BaseChangeMission,  9, DUMMY);   // 4
    trn.addCommand(game::v3::tcm_BaseChangeMission, 23, DUMMY);   // 5
    trn.addCommand(game::v3::tcm_SendMessage,        9, DUMMY);   // 6
    trn.addCommand(game::v3::tcm_ChangePassword,     9, DUMMY);   // 7
    trn.addCommand(777,                              9, DUMMY);   // 8
    a.checkEqual("01. getNumCommands", trn.getNumCommands(), 9U);

    // Test wildcard case
    a.check("11",  game::v3::trn::NameFilter("ship", true).accept(trn, 0));
    a.check("12",  game::v3::trn::NameFilter("ship", true).accept(trn, 1));
    a.check("13", !game::v3::trn::NameFilter("ship", true).accept(trn, 2));
    a.check("14", !game::v3::trn::NameFilter("ship", true).accept(trn, 3));
    a.check("15", !game::v3::trn::NameFilter("ship", true).accept(trn, 4));
    a.check("16", !game::v3::trn::NameFilter("ship", true).accept(trn, 5));
    a.check("17", !game::v3::trn::NameFilter("ship", true).accept(trn, 6));
    a.check("18", !game::v3::trn::NameFilter("ship", true).accept(trn, 7));
    a.check("19", !game::v3::trn::NameFilter("ship", true).accept(trn, 8));

    // Test non-wildcard case
    a.check("21", !game::v3::trn::NameFilter("basechangemission", false).accept(trn, 0));
    a.check("22", !game::v3::trn::NameFilter("basechangemission", false).accept(trn, 1));
    a.check("23", !game::v3::trn::NameFilter("basechangemission", false).accept(trn, 2));
    a.check("24", !game::v3::trn::NameFilter("basechangemission", false).accept(trn, 3));
    a.check("25",  game::v3::trn::NameFilter("basechangemission", false).accept(trn, 4));
    a.check("26",  game::v3::trn::NameFilter("basechangemission", false).accept(trn, 5));
    a.check("27", !game::v3::trn::NameFilter("basechangemission", false).accept(trn, 6));
    a.check("28", !game::v3::trn::NameFilter("basechangemission", false).accept(trn, 7));
    a.check("29", !game::v3::trn::NameFilter("basechangemission", false).accept(trn, 8));
}
