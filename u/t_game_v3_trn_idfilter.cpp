/**
  *  \file u/t_game_v3_trn_idfilter.cpp
  *  \brief Test for game::v3::trn::IdFilter
  */

#include "game/v3/trn/idfilter.hpp"

#include "t_game_v3_trn.hpp"
#include "afl/charset/utf8charset.hpp"
#include "game/timestamp.hpp"
#include "game/v3/turnfile.hpp"

/** Simple tests. */
void
TestGameV3TrnIdFilter::testIt()
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

    // Test each command against [9,9]
    TS_ASSERT( game::v3::trn::IdFilter(9, 9).accept(trn, 0));     // ok
    TS_ASSERT(!game::v3::trn::IdFilter(9, 9).accept(trn, 1));     // wrong Id
    TS_ASSERT( game::v3::trn::IdFilter(9, 9).accept(trn, 2));     // ok
    TS_ASSERT(!game::v3::trn::IdFilter(9, 9).accept(trn, 3));     // wrong Id
    TS_ASSERT( game::v3::trn::IdFilter(9, 9).accept(trn, 4));     // ok
    TS_ASSERT(!game::v3::trn::IdFilter(9, 9).accept(trn, 5));     // wrong Id
    TS_ASSERT(!game::v3::trn::IdFilter(9, 9).accept(trn, 6));     // not Id field
    TS_ASSERT(!game::v3::trn::IdFilter(9, 9).accept(trn, 7));     // not Id field
    TS_ASSERT(!game::v3::trn::IdFilter(9, 9).accept(trn, 8));     // not Id field

    // Test command #1 against different filters
    TS_ASSERT( game::v3::trn::IdFilter(9, 10).accept(trn, 1));
    TS_ASSERT( game::v3::trn::IdFilter(0, 500).accept(trn, 1));
    TS_ASSERT( game::v3::trn::IdFilter(10, 10).accept(trn, 1));
    TS_ASSERT(!game::v3::trn::IdFilter(11, 11).accept(trn, 1));
}
