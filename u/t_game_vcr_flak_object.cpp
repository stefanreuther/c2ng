/**
  *  \file u/t_game_vcr_flak_object.cpp
  *  \brief Test for game::vcr::flak::Object
  */

#include "game/vcr/flak/object.hpp"

#include "t_game_vcr_flak.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/codepage.hpp"

void
TestGameVcrFlakObject::testIt()
{
    game::vcr::flak::Object testee;

    // Get/Set roundtrip
    testee.setMaxFightersLaunched(7);
    testee.setRating(8);
    testee.setCompensation(9);
    testee.setEndingStatus(10);

    TS_ASSERT_EQUALS(testee.getMaxFightersLaunched(), 7);
    TS_ASSERT_EQUALS(testee.getRating(), 8);
    TS_ASSERT_EQUALS(testee.getCompensation(), 9);
    TS_ASSERT_EQUALS(testee.getEndingStatus(), 10);
}

void
TestGameVcrFlakObject::testPack()
{
    static const uint8_t data[] = { 'U','S','S',' ','D',0xFC,'l','l',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                                    5,0,        // 5 damage
                                    4,1,        // 260 crew
                                    2,2,        // ID 514
                                    7,0,        // player 7
                                    63,0,       // hull 63
                                    1,0,        // level 1
                                    3,0,        // 3 beams
                                    10,0,       // type 10
                                    12,0,       // 12 launchers
                                    2,3,        // 770 torpedoes
                                    2,0,        // type 2
                                    0,0,        // 0 bays
                                    0,0,        // 0 fighters
                                    200,0,      // 200 kt
                                    100,0,      // 100% shield
                                    3,0,        // max 3 fighters launched
                                    77,0,1,0,   // rating 65613
                                    50,0,       // compensation 50
                                    0,0,        // not a planet
                                    255,255 };  // was destroyed
    game::vcr::flak::structures::Ship sh;
    static_assert(sizeof(sh) == sizeof(data), "sizeof Ship");
    std::memcpy(&sh, data, sizeof(sh));
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);

    // Test loading
    game::vcr::flak::Object testee(sh, cs);
    TS_ASSERT_EQUALS(testee.getName(), "USS D\xC3\xBCll");
    TS_ASSERT_EQUALS(testee.getDamage(), 5);
    TS_ASSERT_EQUALS(testee.getCrew(), 260);
    TS_ASSERT_EQUALS(testee.getId(), 514);
    TS_ASSERT_EQUALS(testee.getOwner(), 7);
    TS_ASSERT_EQUALS(testee.getHull(), 63);
    TS_ASSERT_EQUALS(testee.getExperienceLevel(), 1);
    TS_ASSERT_EQUALS(testee.getNumBeams(), 3);
    TS_ASSERT_EQUALS(testee.getBeamType(), 10);
    TS_ASSERT_EQUALS(testee.getNumLaunchers(), 12);
    TS_ASSERT_EQUALS(testee.getNumTorpedoes(), 770);
    TS_ASSERT_EQUALS(testee.getTorpedoType(), 2);
    TS_ASSERT_EQUALS(testee.getNumBays(), 0);
    TS_ASSERT_EQUALS(testee.getNumFighters(), 0);
    TS_ASSERT_EQUALS(testee.getMass(), 200);
    TS_ASSERT_EQUALS(testee.getShield(), 100);
    TS_ASSERT_EQUALS(testee.getMaxFightersLaunched(), 3);
    TS_ASSERT_EQUALS(testee.getRating(), 65613);
    TS_ASSERT_EQUALS(testee.getCompensation(), 50);
    TS_ASSERT_EQUALS(testee.isPlanet(), false);
    TS_ASSERT_EQUALS(testee.getEndingStatus(), -1);

    // Test saving
    game::vcr::flak::structures::Ship sh2;
    testee.pack(sh2, cs);
    TS_ASSERT_SAME_DATA(&sh, &sh2, sizeof(sh2));
}

