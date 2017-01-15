/**
  *  \file u/t_game_v3_trn_stringfilter.cpp
  *  \brief Test for game::v3::trn::StringFilter
  */

#include "game/v3/trn/stringfilter.hpp"

#include "t_game_v3_trn.hpp"
#include "afl/charset/utf8charset.hpp"
#include "game/timestamp.hpp"
#include "game/v3/turnfile.hpp"

/** Simple tests. */
void
TestGameV3TrnStringFilter::testIt()
{
    // Make a turnfile
    afl::charset::Utf8Charset cs;
    game::v3::TurnFile trn(cs, 1, game::Timestamp());

    // Add commands. Give each command some dummy data.
    // As SendMessage command, the message text is 'efghijklmnopqrst', which decrypts to "XYZ[\]^_`abcdefgh"
    const uint8_t DUMMY[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
                              'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't' };
    trn.addCommand(game::v3::tcm_ShipChangeSpeed,    9, DUMMY);   // 0
    trn.addCommand(game::v3::tcm_ShipChangeFc,       9, DUMMY);   // 1
    trn.addCommand(game::v3::tcm_ShipChangeName,     9, DUMMY);   // 2
    trn.addCommand(game::v3::tcm_PlanetChangeMines,  9, DUMMY);   // 3
    trn.addCommand(game::v3::tcm_PlanetChangeFc,     9, DUMMY);   // 4
    trn.addCommand(game::v3::tcm_SendMessage,       20, DUMMY);   // 5
    trn.addCommand(777,                              9, DUMMY);   // 6
    TS_ASSERT_EQUALS(trn.getNumCommands(), 7U);

    // Test each command against "abc"
    TS_ASSERT(!game::v3::trn::StringFilter("abc").accept(trn, 0));   // not a string command
    TS_ASSERT( game::v3::trn::StringFilter("abc").accept(trn, 1));   // ok
    TS_ASSERT( game::v3::trn::StringFilter("abc").accept(trn, 2));   // ok
    TS_ASSERT(!game::v3::trn::StringFilter("abc").accept(trn, 3));   // not a string command
    TS_ASSERT( game::v3::trn::StringFilter("abc").accept(trn, 4));   // ok
    TS_ASSERT( game::v3::trn::StringFilter("abc").accept(trn, 5));   // encrypted match
    TS_ASSERT(!game::v3::trn::StringFilter("abc").accept(trn, 6));   // unknown
    TS_ASSERT(!game::v3::trn::StringFilter("abc").accept(trn, 7));   // out-of-range

    // Test each command against "ABC" (match is case-insensitive)
    TS_ASSERT(!game::v3::trn::StringFilter("ABC").accept(trn, 0));   // not a string command
    TS_ASSERT( game::v3::trn::StringFilter("ABC").accept(trn, 1));   // ok
    TS_ASSERT( game::v3::trn::StringFilter("ABC").accept(trn, 2));   // ok
    TS_ASSERT(!game::v3::trn::StringFilter("ABC").accept(trn, 3));   // not a string command
    TS_ASSERT( game::v3::trn::StringFilter("ABC").accept(trn, 4));   // ok
    TS_ASSERT( game::v3::trn::StringFilter("ABC").accept(trn, 5));   // encrypted match
    TS_ASSERT(!game::v3::trn::StringFilter("ABC").accept(trn, 6));   // unknown
    TS_ASSERT(!game::v3::trn::StringFilter("ABC").accept(trn, 7));   // out-of-range

    // Test each command against "xyz"
    TS_ASSERT(!game::v3::trn::StringFilter("xyz").accept(trn, 0));   // not a string command
    TS_ASSERT(!game::v3::trn::StringFilter("xyz").accept(trn, 1));   // ok
    TS_ASSERT(!game::v3::trn::StringFilter("xyz").accept(trn, 2));   // ok
    TS_ASSERT(!game::v3::trn::StringFilter("xyz").accept(trn, 3));   // not a string command
    TS_ASSERT(!game::v3::trn::StringFilter("xyz").accept(trn, 4));   // ok
    TS_ASSERT( game::v3::trn::StringFilter("xyz").accept(trn, 5));   // encrypted match
    TS_ASSERT(!game::v3::trn::StringFilter("xyz").accept(trn, 6));   // unknown
    TS_ASSERT(!game::v3::trn::StringFilter("xyz").accept(trn, 7));   // out-of-range

    // Test each command against ""
    TS_ASSERT(!game::v3::trn::StringFilter("").accept(trn, 0));   // not a string command
    TS_ASSERT( game::v3::trn::StringFilter("").accept(trn, 1));   // ok
    TS_ASSERT( game::v3::trn::StringFilter("").accept(trn, 2));   // ok
    TS_ASSERT(!game::v3::trn::StringFilter("").accept(trn, 3));   // not a string command
    TS_ASSERT( game::v3::trn::StringFilter("").accept(trn, 4));   // ok
    TS_ASSERT( game::v3::trn::StringFilter("").accept(trn, 5));   // encrypted match
    TS_ASSERT(!game::v3::trn::StringFilter("").accept(trn, 6));   // unknown
    TS_ASSERT(!game::v3::trn::StringFilter("").accept(trn, 7));   // out-of-range

    // Test variations
    TS_ASSERT(!game::v3::trn::StringFilter("abcd").accept(trn, 1));  // FCode has only 3 chars
    TS_ASSERT( game::v3::trn::StringFilter("abcd").accept(trn, 2));  // ok
    TS_ASSERT(!game::v3::trn::StringFilter("rst").accept(trn, 1));   // FCode has only 3 chars
    TS_ASSERT( game::v3::trn::StringFilter("rst").accept(trn, 2));   // ok
    TS_ASSERT( game::v3::trn::StringFilter("xyz[").accept(trn, 5));  // ok
}
