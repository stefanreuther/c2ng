/**
  *  \file u/t_game_v3_trn_namefilter.cpp
  *  \brief Test for game::v3::trn::NameFilter
  */

#include "game/v3/trn/namefilter.hpp"

#include "t_game_v3_trn.hpp"
#include "afl/charset/utf8charset.hpp"
#include "game/timestamp.hpp"
#include "game/v3/turnfile.hpp"

/** Simple test. */
void
TestGameV3TrnNameFilter::testIt()
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
    TS_ASSERT_EQUALS(trn.getNumCommands(), 9U);

    // Test wildcard case
    TS_ASSERT( game::v3::trn::NameFilter("ship", true).accept(trn, 0));
    TS_ASSERT( game::v3::trn::NameFilter("ship", true).accept(trn, 1));
    TS_ASSERT(!game::v3::trn::NameFilter("ship", true).accept(trn, 2));
    TS_ASSERT(!game::v3::trn::NameFilter("ship", true).accept(trn, 3));
    TS_ASSERT(!game::v3::trn::NameFilter("ship", true).accept(trn, 4));
    TS_ASSERT(!game::v3::trn::NameFilter("ship", true).accept(trn, 5));
    TS_ASSERT(!game::v3::trn::NameFilter("ship", true).accept(trn, 6));
    TS_ASSERT(!game::v3::trn::NameFilter("ship", true).accept(trn, 7));
    TS_ASSERT(!game::v3::trn::NameFilter("ship", true).accept(trn, 8));

    // Test non-wildcard case
    TS_ASSERT(!game::v3::trn::NameFilter("basechangemission", false).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::NameFilter("basechangemission", false).accept(trn, 1));
    TS_ASSERT(!game::v3::trn::NameFilter("basechangemission", false).accept(trn, 2));
    TS_ASSERT(!game::v3::trn::NameFilter("basechangemission", false).accept(trn, 3));
    TS_ASSERT( game::v3::trn::NameFilter("basechangemission", false).accept(trn, 4));
    TS_ASSERT( game::v3::trn::NameFilter("basechangemission", false).accept(trn, 5));
    TS_ASSERT(!game::v3::trn::NameFilter("basechangemission", false).accept(trn, 6));
    TS_ASSERT(!game::v3::trn::NameFilter("basechangemission", false).accept(trn, 7));
    TS_ASSERT(!game::v3::trn::NameFilter("basechangemission", false).accept(trn, 8));
}
