/**
  *  \file test/game/v3/trn/stringfiltertest.cpp
  *  \brief Test for game::v3::trn::StringFilter
  */

#include "game/v3/trn/stringfilter.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/test/testrunner.hpp"
#include "game/timestamp.hpp"
#include "game/v3/turnfile.hpp"

/** Simple tests. */
AFL_TEST("game.v3.trn.StringFilter", a)
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
    a.checkEqual("01. getNumCommands", trn.getNumCommands(), 7U);

    // Test each command against "abc"
    a.check("11", !game::v3::trn::StringFilter("abc").accept(trn, 0));   // not a string command
    a.check("12",  game::v3::trn::StringFilter("abc").accept(trn, 1));   // ok
    a.check("13",  game::v3::trn::StringFilter("abc").accept(trn, 2));   // ok
    a.check("14", !game::v3::trn::StringFilter("abc").accept(trn, 3));   // not a string command
    a.check("15",  game::v3::trn::StringFilter("abc").accept(trn, 4));   // ok
    a.check("16",  game::v3::trn::StringFilter("abc").accept(trn, 5));   // encrypted match
    a.check("17", !game::v3::trn::StringFilter("abc").accept(trn, 6));   // unknown
    a.check("18", !game::v3::trn::StringFilter("abc").accept(trn, 7));   // out-of-range

    // Test each command against "ABC" (match is case-insensitive)
    a.check("21", !game::v3::trn::StringFilter("ABC").accept(trn, 0));   // not a string command
    a.check("22",  game::v3::trn::StringFilter("ABC").accept(trn, 1));   // ok
    a.check("23",  game::v3::trn::StringFilter("ABC").accept(trn, 2));   // ok
    a.check("24", !game::v3::trn::StringFilter("ABC").accept(trn, 3));   // not a string command
    a.check("25",  game::v3::trn::StringFilter("ABC").accept(trn, 4));   // ok
    a.check("26",  game::v3::trn::StringFilter("ABC").accept(trn, 5));   // encrypted match
    a.check("27", !game::v3::trn::StringFilter("ABC").accept(trn, 6));   // unknown
    a.check("28", !game::v3::trn::StringFilter("ABC").accept(trn, 7));   // out-of-range

    // Test each command against "xyz"
    a.check("31", !game::v3::trn::StringFilter("xyz").accept(trn, 0));   // not a string command
    a.check("32", !game::v3::trn::StringFilter("xyz").accept(trn, 1));   // ok
    a.check("33", !game::v3::trn::StringFilter("xyz").accept(trn, 2));   // ok
    a.check("34", !game::v3::trn::StringFilter("xyz").accept(trn, 3));   // not a string command
    a.check("35", !game::v3::trn::StringFilter("xyz").accept(trn, 4));   // ok
    a.check("36",  game::v3::trn::StringFilter("xyz").accept(trn, 5));   // encrypted match
    a.check("37", !game::v3::trn::StringFilter("xyz").accept(trn, 6));   // unknown
    a.check("38", !game::v3::trn::StringFilter("xyz").accept(trn, 7));   // out-of-range

    // Test each command against ""
    a.check("41", !game::v3::trn::StringFilter("").accept(trn, 0));   // not a string command
    a.check("42",  game::v3::trn::StringFilter("").accept(trn, 1));   // ok
    a.check("43",  game::v3::trn::StringFilter("").accept(trn, 2));   // ok
    a.check("44", !game::v3::trn::StringFilter("").accept(trn, 3));   // not a string command
    a.check("45",  game::v3::trn::StringFilter("").accept(trn, 4));   // ok
    a.check("46",  game::v3::trn::StringFilter("").accept(trn, 5));   // encrypted match
    a.check("47", !game::v3::trn::StringFilter("").accept(trn, 6));   // unknown
    a.check("48", !game::v3::trn::StringFilter("").accept(trn, 7));   // out-of-range

    // Test variations
    a.check("51", !game::v3::trn::StringFilter("abcd").accept(trn, 1));  // FCode has only 3 chars
    a.check("52",  game::v3::trn::StringFilter("abcd").accept(trn, 2));  // ok
    a.check("53", !game::v3::trn::StringFilter("rst").accept(trn, 1));   // FCode has only 3 chars
    a.check("54",  game::v3::trn::StringFilter("rst").accept(trn, 2));   // ok
    a.check("55",  game::v3::trn::StringFilter("xyz[").accept(trn, 5));  // ok
}
